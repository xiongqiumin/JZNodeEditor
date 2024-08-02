#include <QThread>
#include <QElapsedTimer>
#include <QDebug>
#include <QApplication>
#include <QDateTime>
#include "JZModbusClient.h"
#include "JZModbusContext.h"
#include "JZModbusRtu.h"
#include "JZModbusTcp.h"

void JZModbusClient::ReqInfo::clear()
{
    function = -1;
    addr = 0;
    nb = 0;    
    reqTime = 0;
    buffer.clear();
}

//JZModbusClient
JZModbusClient::JZModbusClient(QObject *parent)
    :QObject(parent)
{
    m_ctx = new JZModbusContext();
    m_waitRecv = false;
    m_io = nullptr;
    m_com = nullptr;
    m_socket = nullptr;
    m_timeId = 0;
    m_req.clear();
}

JZModbusClient::~JZModbusClient()
{
    close();
    delete m_ctx;
}

bool JZModbusClient::setSlave(int slave)
{
    return m_ctx->setSlave(slave);    
}

bool JZModbusClient::isOpen()
{
    return m_io && m_io->isOpen();
}

bool JZModbusClient::openRtu(QString com, int baud,QSerialPort::DataBits data_bit, QSerialPort::StopBits stop_bit, QSerialPort::Parity parity_bit)
{
    if (m_io)
        return false;

    m_com = new QSerialPort(this);    
    connect(m_com, &QSerialPort::readyRead, this, &JZModbusClient::onReadyRead);
    initRtuContext(m_ctx);

    m_com->setPortName(com);
    if (!m_com->open(QIODevice::ReadWrite))
    {
        delete m_com;
        m_com = nullptr;
        return false;
    }

    m_io = m_com;
    m_com->setBaudRate(baud);//波特率
    m_com->setDataBits(data_bit);  //数据位
    m_com->setStopBits(stop_bit);  //停止位      
    m_com->setParity(parity_bit);  //校验位
    if(m_timeId != 0)
        m_timeId = startTimer(500);
    return true;
}

bool JZModbusClient::openTcp(QString ip, int port)
{
    if (m_io)
        return false;

    m_socket = new QTcpSocket(this);    
    connect(m_socket, &QTcpSocket::readyRead, this, &JZModbusClient::onReadyRead);
    initTcpContext(m_ctx);

    if (m_timeId != 0)
        m_timeId = startTimer(500);
    
    m_socket->connectToHost(ip, port);    
    bool ret = m_socket->waitForConnected();
    if (!ret)
    {
        delete m_socket;
        m_socket = nullptr;
    }

    m_io = m_socket;
    return true;
}

void JZModbusClient::close()
{
    if (m_timeId != 0)
    {
        killTimer(m_timeId);
        m_timeId = 0;
    }

    if (m_com)
    {
        m_com->close();
        delete m_com;
        m_com = nullptr;
    }

    if (m_socket)
    {
        m_socket->disconnectFromHost();
        delete m_socket;
        m_socket = nullptr;
    }
    m_io = nullptr;
    m_req.clear();
    m_buffer.clear();
}

QString JZModbusClient::error()
{
    return m_error;
}

bool JZModbusClient::isBusy()
{
    return (m_req.function != -1);
}

bool JZModbusClient::waitFinish()
{
    if (!m_io || !isBusy())
        return true;

    while(isBusy())
    {
        m_io->waitForReadyRead(10);
    }    
    return true;
}

void JZModbusClient::sendPacket(uint8_t *req, int req_length)
{
    req_length = m_ctx->sendMsgPre(req, req_length);
    m_req.buffer = QByteArray((char*)req, req_length);
    m_req.reqTime = QDateTime::currentMSecsSinceEpoch();
    m_io->write(m_req.buffer);    
}

JZModebusReply JZModbusClient::waitPacket()
{
    QElapsedTimer t;
    t.start();
    
    JZModebusReply reply;

    m_waitRecv = true;
    while (t.elapsed() < 30 * 1000)
    {                
        if (m_io->waitForReadyRead(10))
            m_buffer.append(m_io->readAll());

        int rc = dealBuffer(reply);
        if (rc != MODBUS_WAIT_RECV)
        {            
            m_waitRecv = false;
            return reply;
        }
    }
    m_waitRecv = false;
    
    m_req.clear();
    reply.code = -1;
    return reply;
}

void JZModbusClient::timerEvent(QTimerEvent *e)
{
    if (!isBusy())
        return;

    qint64 cur_ms = QDateTime::currentMSecsSinceEpoch();
    if (cur_ms > m_req.reqTime + 30 * 1000)
    {
        JZModebusReply reply;
        reply.function = m_req.function;
        reply.code = -1;
        m_req.clear();
        emit sigModbusReplay(reply);
    }
}

bool JZModbusClient::checkInitBusy()
{
    if (m_io == NULL) {
        errno = EINVAL;
        return false;
    }

    if (m_req.function != -1)
    {
        errno = EMBXSBUSY;
        return false;
    }

    return true;
}

bool JZModbusClient::readBits(int addr, int nb, QVector<uint8_t> &dest)
{          
    if (!readBitsAsync(addr, nb))
        return false;

    JZModebusReply reply = waitPacket();
    if (reply.code == 0)
        dest = reply.bit;
    return reply.code == 0;
}

bool JZModbusClient::readInputBits(int addr, int nb, QVector<uint8_t> &dest)
{
    if (!readInputBitsAsync(addr, nb))
        return false;

    JZModebusReply reply = waitPacket();
    if (reply.code == 0)
        dest = reply.bit;
    return reply.code == 0;
}

bool JZModbusClient::readRegisters(int addr, int nb, QVector<uint16_t> &dest)
{
    if (!readRegistersAsync(addr, nb))
        return false;

    JZModebusReply reply = waitPacket();
    if (reply.code == 0)
        dest = reply.reg;
    return reply.code == 0;
}

bool JZModbusClient::readInputRegisters(int addr, int nb, QVector<uint16_t> &dest)
{    
    if (!readInputRegistersAsync(addr, nb))
        return false;

    JZModebusReply reply = waitPacket();
    if (reply.code == 0)
        dest = reply.reg;
    return reply.code == 0;
}

bool JZModbusClient::readBitsAsync(int addr, int nb)
{
    if (!checkInitBusy()) 
        return false;    

    if (nb > MODBUS_MAX_READ_BITS) {
        errno = EMBMDATA;
        return false;
    }

    uint8_t req[_MIN_REQ_LENGTH];
    int req_length = m_ctx->buildRequestBasis(MODBUS_FC_READ_COILS, addr, nb, req);    
    sendPacket(req, req_length);
    m_req.function = MODBUS_FC_READ_COILS;
    m_req.addr = addr;
    m_req.nb = nb;
    return true;
}

bool JZModbusClient::readInputBitsAsync(int addr, int nb)
{
    if (!checkInitBusy())
        return false;

    if (nb > MODBUS_MAX_READ_BITS) {        
        errno = EMBMDATA;
        return false;
    }

    uint8_t req[_MIN_REQ_LENGTH];
    int req_length = m_ctx->buildRequestBasis(MODBUS_FC_READ_DISCRETE_INPUTS, addr, nb, req);
    sendPacket(req, req_length);
    m_req.function = MODBUS_FC_READ_DISCRETE_INPUTS;
    m_req.addr = addr;
    m_req.nb = nb;
    return true;
}

bool JZModbusClient::readRegistersAsync(int addr, int nb)
{
    if (!checkInitBusy())
        return false;

    if (nb > MODBUS_MAX_READ_REGISTERS) {
        errno = EMBMDATA;
        return false;
    }

    uint8_t req[_MIN_REQ_LENGTH];
    int req_length = m_ctx->buildRequestBasis(MODBUS_FC_READ_HOLDING_REGISTERS, addr, nb, req);
    sendPacket(req, req_length);
    m_req.function = MODBUS_FC_READ_HOLDING_REGISTERS;
    m_req.addr = addr;
    m_req.nb = nb;
    return true;
}

bool JZModbusClient::readInputRegistersAsync(int addr, int nb)
{
    if (!checkInitBusy())
        return false;

    if (nb > MODBUS_MAX_READ_REGISTERS) {        
        errno = EMBMDATA;
        return false;
    }

    uint8_t req[_MIN_REQ_LENGTH];
    int req_length = m_ctx->buildRequestBasis(MODBUS_FC_READ_INPUT_REGISTERS, addr, nb, req);
    sendPacket(req,req_length);
    m_req.function = MODBUS_FC_READ_INPUT_REGISTERS;
    m_req.addr = addr;
    m_req.nb = nb;
    return true;
}

bool JZModbusClient::writeBit(int coil_addr, int status)
{
    if (!writeBitAsync(coil_addr, status))
        return false;

    JZModebusReply reply = waitPacket();
    return reply.code == 0;
}

bool JZModbusClient::writeBits(int addr, const QVector<uint8_t> &dest)
{
    if (!writeBitsAsync(addr, dest))
        return false;

    JZModebusReply reply = waitPacket();
    return reply.code == 0;
}

bool JZModbusClient::writeRegister(int reg_addr, int value)
{
    if (!writeRegisterAsync(reg_addr, value))
        return false;

    JZModebusReply reply = waitPacket();
    return reply.code == 0;
}

bool JZModbusClient::writeRegisters(int addr, const QVector<uint16_t> &dest)
{
    if (!writeRegistersAsync(addr, dest))
        return false;

    JZModebusReply reply = waitPacket();
    return reply.code == 0;
}

bool JZModbusClient::writeBitAsync(int coil_addr, int status)
{
    if (!checkInitBusy())
        return false;

    uint8_t req[_MIN_REQ_LENGTH];
    int req_length = m_ctx->buildRequestBasis(MODBUS_FC_WRITE_SINGLE_COIL, coil_addr, status ? 0xFF00 : 0, req);
    m_req.function = MODBUS_FC_WRITE_SINGLE_COIL;
    m_req.addr = coil_addr;
    m_req.nb = 1;
    sendPacket(req, req_length);
    return true;
}

bool JZModbusClient::writeBitsAsync(int addr, const QVector<uint8_t> &dest)
{
    if (!checkInitBusy())
        return false;

    if (dest.size() > MODBUS_MAX_WRITE_BITS) {        
        errno = EMBMDATA;
        return false;
    }

    uint8_t req[MAX_MESSAGE_LENGTH];
    int i;
    int byte_count;
    int bit_check = 0;
    int pos = 0;
    int nb = dest.size();
    int req_length = m_ctx->buildRequestBasis(MODBUS_FC_WRITE_MULTIPLE_COILS, addr, nb, req);

    byte_count = (nb / 8) + ((nb % 8) ? 1 : 0);
    req[req_length++] = byte_count;

    const uint8_t *src = dest.data();
    for (i = 0; i < byte_count; i++) {
        int bit;

        bit = 0x01;
        req[req_length] = 0;

        while ((bit & 0xFF) && (bit_check++ < nb)) {
            if (src[pos++])
                req[req_length] |= bit;
            else
                req[req_length] &= ~bit;

            bit = bit << 1;
        }
        req_length++;
    }
    
    m_req.function = MODBUS_FC_WRITE_MULTIPLE_COILS;
    m_req.addr = addr;
    m_req.nb = dest.size();
    sendPacket(req, req_length);
    return true;
}

bool JZModbusClient::writeRegisterAsync(int addr,int value)
{
    if (!checkInitBusy())
        return false;

    uint8_t req[_MIN_REQ_LENGTH];    
    int req_length = m_ctx->buildRequestBasis(MODBUS_FC_WRITE_SINGLE_REGISTER, addr, value, req);
    m_req.nb = 1;
    m_req.function = MODBUS_FC_WRITE_SINGLE_REGISTER;
    m_req.addr = addr;
    sendPacket(req, req_length);
    return true;
}

bool JZModbusClient::writeRegistersAsync(int addr, const QVector<uint16_t> &dest)
{    
    if (!checkInitBusy())
        return false;

    if (dest.size() > MODBUS_MAX_WRITE_REGISTERS) {        
        errno = EMBMDATA;
        return false;
    }

    uint8_t req[MAX_MESSAGE_LENGTH];
    int i;
    int byte_count;
    int nb = dest.size();
    int req_length = m_ctx->buildRequestBasis(MODBUS_FC_WRITE_MULTIPLE_REGISTERS, addr, nb, req);

    byte_count = nb * 2;
    req[req_length++] = byte_count;

    const uint16_t *src = dest.data();
    for (i = 0; i < nb; i++) {
        req[req_length++] = src[i] >> 8;
        req[req_length++] = src[i] & 0x00FF;
    }
    sendPacket(req, req_length);
    m_req.nb = dest.size();
    m_req.function = MODBUS_FC_WRITE_MULTIPLE_REGISTERS;
    m_req.addr = addr;
    return true;
}

int JZModbusClient::dealBuffer(JZModebusReply &info)
{
    int rc = m_ctx->receiveMessage(MSG_CONFIRMATION, m_buffer);
    if (rc == MODBUS_WAIT_RECV)
        return rc;
    else
    {        
        info.function = m_req.function;
        info.addr = m_req.addr;
        if (rc > 0)
        {
            QByteArray res = m_buffer.left(rc);
            m_buffer = m_buffer.mid(rc);

            const uint8_t *req = (const uint8_t *)m_req.buffer.constData();
            const uint8_t *res_ptr = (const uint8_t *)res.constData();
            int rc = m_ctx->checkConfirmation(req, res_ptr, res.size());
            if (rc != -1)
            {
                info.code = 0;
                if (m_req.function == MODBUS_FC_READ_COILS ||
                    m_req.function == MODBUS_FC_READ_DISCRETE_INPUTS)
                    m_ctx->readIoStatus(m_req.nb, res, rc, info.bit);
                else if (m_req.function == MODBUS_FC_READ_HOLDING_REGISTERS ||
                    m_req.function == MODBUS_FC_READ_INPUT_REGISTERS)
                    m_ctx->readRegisters(res, rc, info.reg);
            }
            else
            {
                info.code = rc;
            }            
        }
        else
        {
            m_buffer.clear();
            info.code = rc;            
        }

        m_req.clear();
        return rc;
    }
}

void JZModbusClient::onReadyRead()
{    
    if (m_waitRecv)
        return;

    m_buffer.append(m_io->readAll());   

    JZModebusReply info;
    int rc = dealBuffer(info);
    if (rc == MODBUS_WAIT_RECV)
        return;

    emit sigModbusReplay(info);       
}