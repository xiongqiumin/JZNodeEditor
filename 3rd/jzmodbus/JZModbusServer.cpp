#include "JZModbusServer.h"
#include "JZModbusRtu.h"
#include "JZModbusTcp.h"

JZModbusServer::JZModbusServer(QObject *parent)
    :QObject(parent)
{
    m_com = nullptr;
    m_tcpServer = nullptr;
    m_ctx = new JZModbusContext();    
    m_mapping = new JZModbusMapping();

    m_baud = 9600;
    m_dataBit = QSerialPort::Data8;
    m_stopBit = QSerialPort::OneStop;
    m_parityBit = QSerialPort::NoParity;
    m_port = 0;
}

JZModbusServer::~JZModbusServer()
{
    clear();
    delete m_ctx;
    delete m_mapping;
}

void JZModbusServer::initMapping(int nb_bits, int nb_input_bits,
    int nb_registers, int nb_input_registers)
{
    m_mapping->init(nb_bits, nb_input_bits, nb_registers, nb_input_registers);
}

JZModbusMapping *JZModbusServer::mapping()
{    
    return m_mapping;
}

void JZModbusServer::setSlave(int slave)
{
    m_ctx->setSlave(slave);
}

void JZModbusServer::setPlcMode(bool slave)
{
    return m_ctx->setPlcMode(slave);
}

void JZModbusServer::clear()
{
    stop();

    if (m_com)
    {
        delete m_com;
        m_com = nullptr;
    }

    if (m_tcpServer)
    {
        delete m_tcpServer;
        m_tcpServer = nullptr;
    }
}

void JZModbusServer::initRtu(QString com, int baud, QSerialPort::DataBits data_bit, QSerialPort::StopBits stop_bit, QSerialPort::Parity parity_bit)
{   
    clear();

    m_com = new QSerialPort(this);
    connect(m_com, &QSerialPort::readyRead, this, &JZModbusServer::onComRead);
    initRtuContext(m_ctx);

    m_comName = com;
    m_baud = baud;
    m_dataBit = data_bit;
    m_stopBit = stop_bit;
    m_parityBit = parity_bit;
}

void JZModbusServer::initTcp(int port)
{
    clear();
        
    initTcpContext(m_ctx);
    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(onNewConnect()));

    m_port = port;
}

bool JZModbusServer::isStart() const
{
    if (m_com && m_com->isOpen())
        return true;
    if (m_tcpServer && m_tcpServer->isListening())
        return true;
    
    return false;
}

bool JZModbusServer::start()
{
    if (m_com)
    {
        m_com->setPortName(m_comName);
        if (!m_com->open(QIODevice::ReadWrite))        
            return false;        

        m_com->setBaudRate(m_baud);
        m_com->setDataBits(m_dataBit);
        m_com->setStopBits(m_stopBit);
        m_com->setParity(m_parityBit);
        return true;
    }
    else if(m_tcpServer)
    {
        if (!m_tcpServer->listen(QHostAddress::AnyIPv4, m_port))
            return false;

        return true;
    }

    return false;
}

void JZModbusServer::stop()
{
    if (m_com)
    {
        m_com->close();        
        m_comBuffer.clear();
    }

    if (m_tcpServer)
    {
        m_tcpServer->close();
        auto it = m_tcpClients.begin();
        while (it != m_tcpClients.end())
        {
            auto socket = it.key();
            socket->close();
            socket->deleteLater();
            it++;
        }
        m_tcpClients.clear();        
    }
}

void JZModbusServer::onNewConnect()
{
    //��ȡ������
    QTcpSocket *socket = m_tcpServer->nextPendingConnection();

    //�����ź�
    connect(socket, SIGNAL(readyRead()), this, SLOT(onTcpRead()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(onTcpDisconnected()));

    m_tcpClients[socket] = QByteArray();
}

void JZModbusServer::onTcpRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    m_tcpClients[socket].append(socket->readAll());

    auto &buffer = m_tcpClients[socket];

    uint8_t rsp[MAX_MESSAGE_LENGTH];
    int rc = dealBuffer(rsp, buffer);
    if (rc == MODBUS_WAIT_RECV)
        return;

    if (rc > 0)
    {
        rc = m_ctx->sendMsgPre(rsp, rc);
        socket->write((char*)rsp, rc);
    }
}

void JZModbusServer::onTcpDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    m_tcpClients.remove(socket);
    socket->deleteLater();
}

void JZModbusServer::onComRead()
{
    m_comBuffer.append(m_com->readAll());

    QByteArray &buffer = m_comBuffer;

    uint8_t rsp[MAX_MESSAGE_LENGTH];
    int rc = dealBuffer(rsp, buffer);
    if (rc == MODBUS_WAIT_RECV)
        return;
    
    if (rc > 0)
    {
        rc = m_ctx->sendMsgPre(rsp, rc);
        m_com->write((char*)rsp, rc);
    }
}

int JZModbusServer::dealBuffer(uint8_t *rsp, QByteArray &buffer)
{
    int rc = m_ctx->receiveMessage(MSG_INDICATION, buffer);
    if (rc == MODBUS_WAIT_RECV)
        return rc;
    else if (rc > 0)
    {
        QByteArray res = buffer.left(rc);
        buffer = buffer.mid(rc);

        const uint8_t *req = (const uint8_t *)res.constData();
        int slave = req[m_ctx->backend->header_length - 1];
        
        MappingChanged info;
        int rsp_length = m_ctx->reply(rsp, req, res.size(), m_mapping, info);
        if (info.addr != -1)
            emit sigMappingChanged(info.addr, info.nb);
        return rsp_length;
    }
    else
    {
        buffer.clear();
        return 0;
    }
}