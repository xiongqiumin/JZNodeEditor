#include "JZCommModbusServer.h"

//JZCommModbusInfo
JZCommModbusServerConfig::JZCommModbusServerConfig()
{
    bitOrder = QDataStream::LittleEndian;
}

void JZCommModbusServerConfig::saveToStream(QDataStream& s) const
{
    JZCommConfig::saveToStream(s);
    s << conn << bitOrder;
}

void JZCommModbusServerConfig::loadFromStream(QDataStream& s)
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