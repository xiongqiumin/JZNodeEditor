#include "JZCommManager.h"
#include "JZNodeEngine.h"
#include "JZNodeUtils.h"
#include "../JZModuleConfigFactory.h"

//JZCommConfigEnum
QDataStream& operator<<(QDataStream& s, const JZCommConfigEnum& param)
{
    JZModuleConfigFactory<JZCommConfig>::instance()->saveToStream(s, param);
    return s;
}

QDataStream& operator>>(QDataStream& s, JZCommConfigEnum& param)
{
    JZModuleConfigFactory<JZCommConfig>::instance()->loadFromStream(s, param);
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
JZCommManager::JZCommManager(QObject* parent)
    :QObject(parent)
{
}

JZCommManager::~JZCommManager()
{
    qDeleteAll(m_commList);
    m_commList.clear();
}

QObject* JZCommManager::comm(QString name)
{
    for (int i = 0; i < m_config.commList.size(); i++)
    {
        if (m_config.commList[i]->name == name)
            return m_commList[i];
    }
    return nullptr;
}

JZCommModbusClient* JZCommManager::modbusClient(QString name)
{
    return qobject_cast<JZCommModbusClient*>(comm(name));
}

JZCommModbusServer* JZCommManager::modbusServer(QString name)
{
    return qobject_cast<JZCommModbusServer*>(comm(name));
}

JZTcpClient* JZCommManager::tcpClient(QString name)
{
    return qobject_cast<JZTcpClient*>(comm(name));
}

JZTcpServer* JZCommManager::tcpServer(QString name)
{
    return qobject_cast<JZTcpServer*>(comm(name));
}

JZUdpSocket* JZCommManager::udp(QString name)
{
    return qobject_cast<JZUdpSocket*>(comm(name));
}

JZSerialPort* JZCommManager::serial(QString name)
{
    return qobject_cast<JZSerialPort*>(comm(name));
}

void JZCommManager::init()
{
    for (int i = 0; i < m_commList.size(); i++)
    {
        m_commList[i]->close();
        m_commList[i]->deleteLater();
    }
    m_commList.clear();

    //init
    for (int i = 0; i < m_config.commList.size(); i++)
    {
        auto &cfg = m_config.commList[i];
        int comm_type = cfg->type;

        //modbus
        JZCommObject* comm_obj = nullptr;
        if (comm_type == Comm_ModbusRtuClient || comm_type == Comm_ModbusTcpClient)
        {
            JZCommModbusClient* client = new JZCommModbusClient(this);
            comm_obj = client;
        }
        else if (comm_type == Comm_ModbusRtuServer || comm_type == Comm_ModbusTcpServer)
        {
            JZCommModbusServer* server = new JZCommModbusServer(this);
            comm_obj = server;        
        }
        else if (comm_type == Comm_TcpClient)
        {
            JZTcpClient* client = new JZTcpClient(this);
            comm_obj = client;
        }
        else if (comm_type == Comm_TcpServer)
        {
            JZTcpServer* server = new JZTcpServer(this);
            comm_obj = server;
        }
        else if (comm_type == Comm_Udp)
        {
            JZUdpSocket* udp = new JZUdpSocket(this);
            comm_obj = udp;
        }
        else if (comm_type == Comm_SerialPort)
        {
            JZSerialPort* com = new JZSerialPort(this);
            comm_obj = com;
        }
        else
        {
            Q_ASSERT(0);
        }

        comm_obj->setConfig(cfg);
        m_commList.push_back(comm_obj);
    }
}

void JZCommManager::openAll()
{
    for (auto c : m_commList)
        c->open();
}

void JZCommManager::closeAll()
{
    for (auto c : m_commList)
        c->close();
}

void JZCommManager::setConfig(const JZCommManagerConfig&config)
{
    m_config = config;
    init();
}

JZCommManagerConfig JZCommManager::config()
{
    return m_config;
}

QList<JZCommObject*> JZCommManager::commList()
{
    return m_commList;
}

//func
JZTcpClient* getTcpClient(JZCommManager* mgr, QString name)
{
    JZTcpClient* client = mgr->tcpClient(name);
    if(!client)
        throw std::runtime_error("client is nullptr");

    if (!client->isOpen() && !client->open())
        throw std::runtime_error("client open failed");

    return client;
}

JZSerialPort* getSerial(JZCommManager* mgr, QString name)
{
    auto client = mgr->serial(name);
    if (!client)
        throw std::runtime_error("client is nullptr");

    if (!client->isOpen() && !client->open())
        throw std::runtime_error("client open failed");

    return client;
}

JZUdpSocket* getUdp(JZCommManager* mgr, QString name)
{
    auto udp = mgr->udp(name);
    if (!udp)
        throw std::runtime_error("client is nullptr");

    if (!udp->isOpen() && !udp->open())
        throw std::runtime_error("client open failed");

    return udp;
}

void JZCommInit(JZCommManager* inst, const QByteArray& buffer)
{
    JZCommManagerConfig config = JZNodeUtils::fromBuffer<JZCommManagerConfig>(buffer);
    inst->setConfig(config);
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
    auto client = getTcpClient(mgr, name);
    return client->read();
}

void JZCommTcpWrite(JZCommManager* mgr, const QString& name, const QByteArray& param)
{
    auto client = getTcpClient(mgr, name);
    client->write(param);
}

QString JZCommTcpReadText(JZCommManager* mgr, const QString& name)
{
    auto client = getTcpClient(mgr, name);
    return client->readText();
}

void JZCommTcpWriteText(JZCommManager* mgr, const QString& name, const QString& param)
{
    auto client = getTcpClient(mgr,name);
    client->writeText(param);
}

QByteArray JZCommUdpRead(JZCommManager* mgr, const QString& name)
{
    auto com = getUdp(mgr, name);
    return com->read().data();
}

void JZCommUdpWrite(JZCommManager* mgr, const QString& name, const QByteArray& buffer, QString ip, int port)
{
    auto com = getUdp(mgr, name);
    com->write(buffer, ip, port);
}

//serial
QByteArray JZCommSerialRead(JZCommManager* mgr, const QString& name)
{
    auto com = getSerial(mgr, name);
    return com->read();
}

void JZCommSerialWrite(JZCommManager* mgr, const QString& name, const QByteArray& param) 
{
    auto com = getSerial(mgr, name);
    return com->write(param);
}

QString JZCommSerialReadText(JZCommManager* mgr, const QString& name)
{
    auto com = getSerial(mgr, name);
    return com->readText();
}

void JZCommSerialWriteText(JZCommManager* mgr, const QString& name, const QString& param)
{
    auto com = getSerial(mgr, name);
    return com->writeText(param);
}