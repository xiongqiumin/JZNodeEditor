#include "JZCommManager.h"
#include "JZNodeEngine.h"
#include "JZNodeUtils.h"

//JZCommModbusInfo
JZCommModbusInfo::JZCommModbusInfo()
{
    bitOrder = QDataStream::LittleEndian;
}

QDataStream& operator<<(QDataStream& s, const JZCommModbusInfo& param)
{
    s << param.conn << param.bitOrder;
    return s;
}

QDataStream& operator>>(QDataStream& s, JZCommModbusInfo& param)
{
    s >> param.conn >> param.bitOrder;
    return s;
}

//JZCommTcpClientInfo
JZCommTcpClientInfo::JZCommTcpClientInfo()
{
    ip = "127.0.0.1";
    port = 0;
}

QDataStream& operator<<(QDataStream& s, const JZCommTcpClientInfo& param) 
{
    s << param.ip << param.port;
    return s;
}
QDataStream& operator>>(QDataStream& s, JZCommTcpClientInfo& param) 
{
    s >> param.ip >> param.port;
    return s;
}

//JZCommTcpServerInfo
JZCommTcpServerInfo::JZCommTcpServerInfo()
{
    ip = "127.0.0.1";
    port = 0;
}

QDataStream& operator<<(QDataStream& s, const JZCommTcpServerInfo& param) 
{
    s << param.ip << param.port;
    return s;
}
QDataStream& operator>>(QDataStream& s, JZCommTcpServerInfo& param) 
{
    s >> param.ip >> param.port;
    return s;
}

//JZCommUdpInfo
JZCommUdpInfo::JZCommUdpInfo()
{
    port = 0;
}

QDataStream& operator<<(QDataStream& s, const JZCommUdpInfo& param) 
{
    s << param.port;
    return s;
}
QDataStream& operator>>(QDataStream& s, JZCommUdpInfo& param) 
{
    s >> param.port;
    return s;
}

//JZCommSerialPortInfo
JZCommSerialPortInfo::JZCommSerialPortInfo()
{
    portName = "COM1";
    baud = 9600;
    dataBit = QSerialPort::Data8;
    parityBit = QSerialPort::NoParity;
    stopBit = QSerialPort::OneStop;
}

QDataStream& operator<<(QDataStream& s, const JZCommSerialPortInfo& param) 
{
    s << param.portName << param.baud << param.dataBit << param.parityBit << param.stopBit;
    return s;
}
QDataStream& operator>>(QDataStream& s, JZCommSerialPortInfo& param) 
{
    s >> param.portName >> param.baud >> param.dataBit >> param.parityBit >> param.stopBit;
    return s;
}

//JZCommConfig
JZCommConfig::JZCommConfig()
{
    commType = Comm_None;
}

QDataStream &operator<<(QDataStream &s, const JZCommConfig &param)
{
    s << param.name;
    s << param.commType;
    s << param.modbus;
    return s;
}

QDataStream &operator >> (QDataStream &s, JZCommConfig &param)
{
    s >> param.name;
    s >> param.commType;
    s >> param.modbus;
    return s;
}

//JZModbusManagerConfig
QDataStream &operator<<(QDataStream &s, const JZCommManagerConfig &param)
{
    s << param.commList;
    return s;
}

QDataStream &operator >> (QDataStream &s, JZCommManagerConfig &param)
{
    s >> param.commList;
    return s;
}

//JZCommManager
JZCommManager::JZCommManager()
{
}

JZCommManager::~JZCommManager()
{
    qDeleteAll(m_modbusClient);
    qDeleteAll(m_modbusServer);
    qDeleteAll(m_tcpClient);
    qDeleteAll(m_udpClient);
    qDeleteAll(m_serialPort);

    m_modbusClient.clear();
    m_modbusServer.clear();
    m_tcpClient.clear();
    m_udpClient.clear();
    m_serialPort.clear();
}

JZModbusClient* JZCommManager::modbusClient(QString name)
{
    return m_modbusClient.value(name, nullptr);
}

JZTcpClient* JZCommManager::tcpClient(QString name)
{
    return m_tcpClient.value(name, nullptr);
}

JZUdpSocket* JZCommManager::udpClient(QString name)
{
    return m_udpClient.value(name, nullptr);
}

JZSerialPort* JZCommManager::serial(QString name)
{
    return m_serialPort.value(name, nullptr);
}

void JZCommManager::init()
{
    
    for (int i = 0; i < m_config.commList.size(); i++)
    {
        auto &cfg = m_config.commList[i];
        //modbus
        if (cfg.commType == Comm_ModbusRtuClient)
        {
            JZModbusClient* client = new JZModbusClient(this);
            client->initConn(cfg.modbus.conn);
            client->setProperty("bitOrder", cfg.modbus.bitOrder);
            m_modbusClient[cfg.name] = client;
        }
        else if (cfg.commType == Comm_ModbusRtuServer)
        {

        }
        else if (cfg.commType == Comm_ModbusTcpClient)
        {
            JZTcpClient* client = new JZTcpClient(this);
            client->connectToHost(cfg.tcpClient.ip, cfg.tcpClient.port);
            m_tcpClient[cfg.name] = client;
        }
        else if (cfg.commType == Comm_ModbusTcpServer)
        {
        }
        else if (cfg.commType == Comm_ModbusUdp)
        {
            JZUdpSocket* client = new JZUdpSocket(this);
            m_udpClient[cfg.name] = client;
        }
        else if (cfg.commType == Comm_ModbusSerialPort)
        {
            auto& conn = cfg.serial;

            JZSerialPort* client = new JZSerialPort(this);
            client->open(conn.portName, conn.baud, conn.dataBit, conn.stopBit, conn.parityBit);
            m_serialPort[cfg.name] = client;
        }
    }

}

void JZCommManager::setConfig(const JZCommManagerConfig&config)
{
    m_config = config;
}

JZCommManagerConfig JZCommManager::config()
{
    return m_config;
}

void JZCommInit(JZCommManager* inst, const QByteArray& buffer)
{
    JZCommManagerConfig config = JZNodeUtils::fromBuffer<JZCommManagerConfig>(buffer);
    inst->setConfig(config);
    inst->init();
}

//modbus
template<class T>
static void paramPack(const JZVariantAny &any, QDataStream::ByteOrder remoteByteOrder, QVector<uint16_t>& out)
{
    QVector<T> in;
    in << any.variant.value<T>();
    out = JZModbusTrans::convertByteOrder<T, uint16_t>(in, QDataStream::LittleEndian, remoteByteOrder);
}

template<class T>
static void paramUnpack(JZVariantAny &any, QDataStream::ByteOrder remoteByteOrder, const QVector<uint16_t>& buffer)
{
    QVector<T> out = JZModbusTrans::convertByteOrder<uint16_t, T>(buffer, remoteByteOrder, QDataStream::LittleEndian);
    any.variant = QVariant::fromValue<T>(out[0]);
}

JZVariantAny JZCommModbusRead(JZCommManager* mgr, const QString& name, int function, const QString& read_type, int addr)
{
    auto client = mgr->modbusClient(name);
    if (!client)
        throw std::runtime_error("client is nullptr");

    QDataStream::ByteOrder bit_order = (QDataStream::ByteOrder)client->property("BitOrder").toInt();
    if (!client->isOpen() && !client->open())
        throw std::runtime_error("client open failed");
    
    JZVariantAny any;
    bool ret = false;
    if (function == Function_Bit || function == Function_InputBit)
    {
        QVector<uint8_t> dest;
        if (function == Function_Bit)
        {
            ret = client->readBits(addr, 1, dest);
        }
        else
        {
            ret = client->readInputBits(addr, 1, dest);
        }
        if (ret)
            any.variant = QVariant::fromValue<uint8_t>(dest[0]);
    }
    else if (function == Function_InputRegister || function == Function_Register)
    {
        int count = JZNodeType::byteSize(read_type) / 2;

        QVector<uint16_t> buffer;
        if (function == Function_InputRegister)
            ret = client->readInputRegisters(addr, count, buffer);
        else
            ret = client->readRegisters(addr, count, buffer);

        if (ret)
        {             
            int dataType = JZNodeType::nameToType(read_type);
            if (dataType == Type_int16)
                paramUnpack<int16_t>(any,bit_order, buffer);
            else if (dataType == Type_uint16)
                paramUnpack<uint16_t>(any, bit_order, buffer);
            else if (dataType == Type_int)
                paramUnpack<int32_t>(any, bit_order, buffer);
            else if (dataType == Type_uint)
                paramUnpack<uint32_t>(any, bit_order, buffer);
            else if (dataType == Type_float)
                paramUnpack<float>(any, bit_order, buffer);
            else if (dataType == Type_double)
                paramUnpack<double>(any, bit_order, buffer);
        }
    }

    if (!ret)
        throw std::runtime_error("read param failed");

    return any;
}

void JZCommModbusWrite(JZCommManager* mgr, const QString& name, int function, const QString& read_type, int addr, JZVariantAny value)
{
    auto client = mgr->modbusClient(name);
    if (!client)
        throw std::runtime_error("client is nullptr");

    QDataStream::ByteOrder bit_order = (QDataStream::ByteOrder)client->property("BitOrder").toInt();
    if (!client->isOpen() && !client->open())
        throw std::runtime_error("client open failed");

    bool ret = false;
    if (function == Function_Bit)
    {        
        ret = client->writeBit(addr, value.variant.value<uint8_t>());
    }
    else if (function == Function_Register)
    {
        QVector<uint16_t> buffer;        
        int dataType = JZNodeType::nameToType(read_type);
        if (dataType == Type_int16)
            paramPack<int16_t>(value, bit_order, buffer);
        else if (dataType == Type_uint16)
            paramPack<uint16_t>(value, bit_order, buffer);
        else if (dataType == Type_int)
            paramPack<int32_t>(value, bit_order, buffer);
        else if (dataType == Type_uint)
            paramPack<uint32_t>(value, bit_order, buffer);
        else if (dataType == Type_float)
            paramPack<float>(value, bit_order, buffer);
        else if (dataType == Type_double)
            paramPack<double>(value, bit_order, buffer);
        else {
            Q_ASSERT(0);
        }

        ret = client->writeRegisters(addr, buffer);
    }
    else {
        Q_ASSERT(0);
    }

    if (!ret)
        throw std::runtime_error("write param failed");
}

//net
QByteArray JZCommTcpRead(JZCommManager* mgr, const QString& name)
{
    auto client = mgr->tcpClient(name);
    if (!client)
        throw std::runtime_error("client is nullptr");

    return client->read();
}

void JZCommTcpWrite(JZCommManager* mgr, const QString& name, const QByteArray& param)
{
    auto client = mgr->tcpClient(name);
    if (!client)
        throw std::runtime_error("client is nullptr");

    client->write(param);
}

QString JZCommTcpReadText(JZCommManager* mgr, const QString& name)
{
    auto client = mgr->tcpClient(name);
    if (!client)
        throw std::runtime_error("client is nullptr");

    return client->readText();
}

void JZCommTcpWriteText(JZCommManager* mgr, const QString& name, const QString& param)
{
    auto client = mgr->tcpClient(name);
    if (!client)
        throw std::runtime_error("client is nullptr");

    client->writeText(param);
}

QByteArray JZCommUdpRead(JZCommManager* mgr, const QString& name)
{
    return QByteArray();
}

void JZCommUdpWrite(JZCommManager* mgr, const QString& name, const QByteArray& param)
{
}

//serial
QByteArray JZCommSerialRead(JZCommManager* mgr, const QString& name)
{
    auto com = mgr->serial(name);
    if (!com)
        throw std::runtime_error("client is nullptr");

    return com->read();
}

void JZCommSerialWrite(JZCommManager* mgr, const QString& name, const QByteArray& param) 
{
    auto com = mgr->serial(name);
    if (!com)
        throw std::runtime_error("client is nullptr");

    return com->write(param);
}

QString JZCommSerialReadText(JZCommManager* mgr, const QString& name)
{
    auto com = mgr->serial(name);
    if (!com)
        throw std::runtime_error("client is nullptr");

    return com->readText();
}

void JZCommSerialWriteText(JZCommManager* mgr, const QString& name, const QString& param)
{
    auto com = mgr->serial(name);
    if (!com)
        throw std::runtime_error("client is nullptr");

    return com->writeText(param);
}