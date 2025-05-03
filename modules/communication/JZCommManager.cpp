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
    m_modbusClient.clear();
    m_modbusServer.clear();
}

JZModbusClient* JZCommManager::modbusClient(QString name)
{
    return m_modbusClient.value(name, nullptr);
}

void JZCommManager::init()
{
    for (int i = 0; i < m_config.commList.size(); i++)
    {
        auto &cfg = m_config.commList[i];
        if (cfg.commType == Comm_ModbusRtuClient)
        {
            JZModbusClient* client = new JZModbusClient(this);
            client->initConn(cfg.modbus.conn);
            client->setProperty("bitOrder", cfg.modbus.bitOrder);
            m_modbusClient[cfg.name] = client;
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
    in << any.value<T>();
    out = JZModbusTrans::convertByteOrder<T, uint16_t>(in, QDataStream::LittleEndian, remoteByteOrder);
}

template<class T>
static void paramUnpack(JZVariantAny &any, QDataStream::ByteOrder remoteByteOrder, const QVector<uint16_t>& buffer)
{
    QVector<T> out = JZModbusTrans::convertByteOrder<uint16_t, T>(buffer, remoteByteOrder, QDataStream::LittleEndian);
    any = JZVariantAny::fromValue<T>(out[0]);
}

JZVariantAny JZCommModbusRead(JZCommManager* mgr, const QString& name, const QJsonObject &param)
{
    auto client = mgr->modbusClient(name);
    if (!client)
        throw std::runtime_error("client is nullptr");

    QDataStream::ByteOrder bit_order = (QDataStream::ByteOrder)client->property("BitOrder").toInt();
    if (!client->isOpen() && !client->open())
        throw std::runtime_error("client open failed");
    
    JZVariantAny any;
    bool ret = false;
    int addr = param["addr"].toInt();
    int function = param["function"].toInt();
    QString read_type = param["dataType"].toString();
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

void JZCommModbusWrite(JZCommManager* mgr, const QString& name, const QJsonObject &param, JZVariantAny value)
{
    auto client = mgr->modbusClient(name);
    if (!client)
        throw std::runtime_error("client is nullptr");

    QDataStream::ByteOrder bit_order = (QDataStream::ByteOrder)client->property("BitOrder").toInt();
    if (!client->isOpen() && !client->open())
        throw std::runtime_error("client open failed");

    bool ret = false;
    int addr = param["addr"].toInt();
    int function = param["function"].toInt();
    QString read_type = param["dataType"].toString();
    if (function == Function_Bit)
    {        
        ret = client->writeBit(addr, value.value<uint8_t>());
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
QString JZCommTcpRead(JZCommManager* mgr, const QString& name, const QJsonObject& param)
{
    return QString();
}

void JZCommTcpWrite(JZCommManager* mgr, const QString& name, const QJsonObject& param, QString any)
{
}

QByteArray JZCommTcpReadBin(JZCommManager* mgr, const QString& name, const QJsonObject& param)
{
    return QByteArray();
}

void JZCommTcpWriteBin(JZCommManager* mgr, const QString& name, const QJsonObject& param, const QByteArray& any)
{
}

QString JZCommUdpRead(JZCommManager* mgr, const QString& name, const QJsonObject& param)
{
    return QString();
}

void JZCommUdpWrite(JZCommManager* mgr, const QString& name, const QJsonObject& param, QString any)
{
}

QByteArray JZCommUdpReadBin(JZCommManager* mgr, const QString& name, const QJsonObject& param)
{
    return QByteArray();
}

void JZCommUdpWriteBin(JZCommManager* mgr, const QString& name, const QJsonObject& param, const QByteArray& any)
{
}

//serial
QString JZCommSerialRead(JZCommManager* mgr, const QString& name, const QJsonObject& param)
{
    return QString();
}

void JZCommSerialWrite(JZCommManager* mgr, const QString& name, const QJsonObject& param, QString any)
{
}

QByteArray JZCommSerialReadBin(JZCommManager* mgr, const QString& name, const QJsonObject& param)
{
    return QByteArray();
}

void JZCommSerialWriteBin(JZCommManager* mgr, const QString& name, const QJsonObject& param, const QByteArray& any)
{
}