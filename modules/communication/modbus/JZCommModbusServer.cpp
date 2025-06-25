#include "JZCommModbusServer.h"

//JZCommModbusRtuServerConfig
JZCommModbusRtuServerConfig::JZCommModbusRtuServerConfig()
{
    bitOrder = QDataStream::LittleEndian;
    conn.modbusType = Modbus_rtuClient;
}

void JZCommModbusRtuServerConfig::saveToStream(QDataStream& s) const
{
    JZCommConfig::saveToStream(s);
    s << conn << bitOrder;
}

void JZCommModbusRtuServerConfig::loadFromStream(QDataStream& s)
{
    JZCommConfig::loadFromStream(s);
    s >> conn >> bitOrder;
}

//JZCommModbusTcpServerConfig
JZCommModbusTcpServerConfig::JZCommModbusTcpServerConfig()
{
    bitOrder = QDataStream::LittleEndian;
    conn.modbusType = Modbus_tcpClient;
}

void JZCommModbusTcpServerConfig::saveToStream(QDataStream& s) const
{
    JZCommConfig::saveToStream(s);
    s << conn << bitOrder;
}

void JZCommModbusTcpServerConfig::loadFromStream(QDataStream& s)
{
    JZCommConfig::loadFromStream(s);
    s >> conn >> bitOrder;
}

//JZCommModbusServer
JZCommModbusServer::JZCommModbusServer(QObject* parent)
    :JZCommObject(parent)
{
    m_server = new JZModbusServer(this);
}

JZCommModbusServer::~JZCommModbusServer()
{
    m_server->stop();
}

bool JZCommModbusServer::isOpen()
{
    return m_server->isStart();
}

bool JZCommModbusServer::open()
{
    return m_server->start();
}

void JZCommModbusServer::close()
{
    m_server->stop();
}

JZModbusServer *JZCommModbusServer::server()
{
    return m_server;
}