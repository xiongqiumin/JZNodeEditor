#include <QElapsedTimer>
#include <QScopeGuard>
#include "JZCommModbusClient.h"
#include "JZNodeRuntime.h"
#include "jzCo/JZCo.h"

//JZCommModbusRtuClientConfig
JZCommModbusRtuClientConfig::JZCommModbusRtuClientConfig()
{
    type = Comm_ModbusRtuClient;
    name = "modbusTcpClient";
    bitOrder = QDataStream::LittleEndian;
}

void JZCommModbusRtuClientConfig::saveToStream(QDataStream& s) const
{
    JZCommConfig::saveToStream(s);
    s << conn << bitOrder;
}

void JZCommModbusRtuClientConfig::loadFromStream(QDataStream& s)
{
    JZCommConfig::loadFromStream(s);
    s >> conn >> bitOrder;
}

//JZCommModbusTcpClientConfig
JZCommModbusTcpClientConfig::JZCommModbusTcpClientConfig()
{
    type = Comm_ModbusTcpClient;
    name = "modbusRtuClient";
    conn.modbusType = Modbus_rtuClient;
    bitOrder = QDataStream::LittleEndian;
}

void JZCommModbusTcpClientConfig::saveToStream(QDataStream& s) const
{
    JZCommConfig::saveToStream(s);
    s << conn << bitOrder;
}

void JZCommModbusTcpClientConfig::loadFromStream(QDataStream& s)
{
    JZCommConfig::loadFromStream(s);
    s >> conn >> bitOrder;
}

//JZCommModbusClient
JZCommModbusClient::JZCommModbusClient(QObject* parent)
    :JZCommObject(parent)
{
    m_client = new JZModbusClient(this);
    m_waitReplay = false;
    connect(m_client, &JZModbusClient::sigModbusOpen, this, &JZCommModbusClient::onModbusResult);
    connect(m_client, &JZModbusClient::sigModbusReplay, this, &JZCommModbusClient::onModbusReplay);
    m_waitConnect = None;
}

JZCommModbusClient::~JZCommModbusClient()
{
    m_client->close();
}

bool JZCommModbusClient::isOpen()
{
    return m_client->isOpen();
}

bool JZCommModbusClient::open()
{
    if (m_waitConnect != None)
        return false;

    auto *cfg = dynamic_cast<JZCommModbusTcpClientConfig*>(m_config.data());
    m_client->initConn(cfg->conn);
    m_client->openAsync();

    auto cleanup = qScopeGuard([this] {
        m_waitConnect = None;
    });

    m_waitConnect = Connecting;
    QElapsedTimer t;
    t.start();
    while (m_waitConnect == Connecting && t.elapsed() < 10 * 1000)
        jzco_sleep(10);

    bool ret = (m_waitConnect == ConnectSuccessed);
    m_waitConnect = None;    
    return ret;
}

void JZCommModbusClient::close()
{
    m_client->close();
}

JZModbusClient *JZCommModbusClient::client()
{
    return m_client;
}

void JZCommModbusClient::onModbusReplay(const JZModebusReply& reply)
{
    if (m_waitReplay)
    {
        m_reply = reply;
        m_waitReplay = false;
    }
}

void JZCommModbusClient::onModbusResult(bool flag)
{
    if(m_waitConnect == Connecting)
        m_waitConnect = flag? ConnectSuccessed : ConnectFailed;
}

bool JZCommModbusClient::waitReplay()
{
    m_waitReplay = true;
    if (g_scheduler->isInCoroutine())
    {
        QElapsedTimer t;
        t.start();
        while (t.elapsed() < 30 * 1000)
        {
            if (!m_waitReplay)
                return true;

            jzco_sleep(20);
        }
        m_waitReplay = false;
        return false;
    }
    else
    {
        if (m_client->waitFinish())
        {
            if (!m_waitReplay)
                return true;
        }
    }
    m_waitReplay = false;
    return false;
}

bool JZCommModbusClient::readBits(int addr, int nb, QVector<uint8_t>& dest)
{
    if (!m_client->readBitsAsync(addr, nb) || !waitReplay())
        return false;

    if (m_reply.code == 0)
        dest = m_reply.bit;
    return m_reply.code == 0;
}

bool JZCommModbusClient::readInputBits(int addr, int nb, QVector<uint8_t>& dest)
{
    if (!m_client->readInputBitsAsync(addr, nb) || !waitReplay())
        return false;

    if (m_reply.code == 0)
        dest = m_reply.bit;
    return m_reply.code == 0;
}

bool JZCommModbusClient::readRegisters(int addr, int nb, QVector<uint16_t>& dest)
{
    if (!m_client->readRegistersAsync(addr, nb) || !waitReplay())
        return false;

    if (m_reply.code == 0)
        dest = m_reply.reg;
    return m_reply.code == 0;
}

bool JZCommModbusClient::readInputRegisters(int addr, int nb, QVector<uint16_t>& dest)
{
    if (!m_client->readInputRegistersAsync(addr, nb) || !waitReplay())
        return false;

    if (m_reply.code == 0)
        dest = m_reply.reg;
    return m_reply.code == 0;
}

bool JZCommModbusClient::writeBit(int coil_addr, int status)
{
    if (!m_client->writeBitAsync(coil_addr, status) || !waitReplay())
        return false;

    return true;
}

bool JZCommModbusClient::writeRegisters(int addr, const QVector<uint16_t>& dest)
{
    if (!m_client->writeRegistersAsync(addr, dest) || !waitReplay())
        return false;

    return true;
}