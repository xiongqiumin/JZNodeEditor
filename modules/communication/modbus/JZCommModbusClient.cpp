#include "JZCommModbusClient.h"

//JZCommModbusInfo
JZCommModbusClientConfig::JZCommModbusClientConfig()
{
    type = Comm_ModbusRtuClient;
    name = "modbusClient";
    conn.modbusType = Modbus_rtuClient;
    bitOrder = QDataStream::LittleEndian;
}

void JZCommModbusClientConfig::saveToStream(QDataStream& s) const
{
    JZCommConfig::saveToStream(s);
    s << conn << bitOrder;
}

void JZCommModbusClientConfig::loadFromStream(QDataStream& s)
{
    JZCommConfig::loadFromStream(s);
    s >> conn >> bitOrder;
}

//JZCommModbusClient
JZCommModbusClient::JZCommModbusClient(QObject* parent)
    :JZCommObject(parent)
{
    m_client = new JZModbusClient(this);
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
    return m_client->open();
}

void JZCommModbusClient::close()
{
    m_client->close();
}

JZModbusClient *JZCommModbusClient::client()
{
    return m_client;
}