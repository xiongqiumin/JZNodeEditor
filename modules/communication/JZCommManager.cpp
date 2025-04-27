#include "JZCommManager.h"
#include "JZNodeEngine.h"
#include "JZNodeUtils.h"

//JZModbusManagerConfig
QDataStream &operator<<(QDataStream &s, const JZModbusManagerConfig &param)
{
    s << param.m_clientConnet;
    return s;
}

QDataStream &operator >> (QDataStream &s, JZModbusManagerConfig &param)
{
    s >> param.m_clientConnet;
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

JZModbusClient* JZCommManager::modbusClient(int idx)
{
    if (idx < 0 || idx >= m_modbusClient.size())
        return nullptr;

	return m_modbusClient[idx];
}

void JZCommManager::init()
{

}

void JZCommManager::setConfig(const JZModbusManagerConfig &config)
{
    m_config = config;
}

JZModbusManagerConfig JZCommManager::config()
{
    return m_config;
}

void JZCommInit(JZCommManager* inst, const QByteArray& buffer)
{
    JZModbusManagerConfig config = JZNodeUtils::fromBuffer<JZModbusManagerConfig>(buffer);
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

JZVariantAny JZCommModbusRead(JZModbusClient *client, const QJsonObject &param)
{
    QDataStream::ByteOrder bit_order = (QDataStream::ByteOrder)client->property("BitOrder").toInt();
    if (!client->isOpen())
        throw std::runtime_error("client is not open");
    
    JZVariantAny any;
    bool ret = false;
    int addr = param["addr"].toInt();
    int function = param["function"].toInt();
    int count = param["count"].toInt();
    QString read_type = param["readType"].toString();
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
        QVector<uint16_t> buffer;
        if (function == Function_InputRegister)
            ret = client->readInputRegisters(addr, count, buffer);
        else
            ret = client->readRegisters(addr, count, buffer);

        if (ret)
        {             
            int dataType = g_engine->environment()->nameToType(read_type);
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

void JZCommModbusWrite(JZModbusClient *client, const QJsonObject &param, JZVariantAny value)
{
    QDataStream::ByteOrder bit_order = (QDataStream::ByteOrder)client->property("BitOrder").toInt();
    if (!client->isOpen())
        throw std::runtime_error("client is not open");    

    JZVariantAny any;
    bool ret = false;
    int addr = param["addr"].toInt();
    int function = param["function"].toInt();
    int count = param["count"].toInt();
    QString read_type = param["readType"].toString();
    if (function == Function_Bit)
    {        
        ret = client->writeBit(addr, value.value<uint8_t>());
    }
    else if (function == Function_Register)
    {
        QVector<uint16_t> buffer;
        
        int dataType = g_engine->environment()->nameToType(read_type);
        if (dataType == Type_int16)
            paramPack<int16_t>(any, bit_order, buffer);
        else if (dataType == Type_int16)
            paramPack<uint16_t>(any, bit_order, buffer);
        else if (dataType == Type_int16)
            paramPack<int32_t>(any, bit_order, buffer);
        else if (dataType == Type_int16)
            paramPack<uint32_t>(any, bit_order, buffer);
        else if (dataType == Type_int16)
            paramPack<float>(any, bit_order, buffer);
        else if (dataType == Type_int16)
            paramPack<double>(any, bit_order, buffer);

        ret = client->writeRegisters(addr, buffer);
    }

    if (!ret)
        throw std::runtime_error("write param failed");
}