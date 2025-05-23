#include "JZCommModbusServer.h"

//JZCommModbusInfo
JZCommModbusServerConfig::JZCommModbusServerConfig()
{
    bitOrder = QDataStream::LittleEndian;
}

void JZCommModbusServerConfig::saveToStream(QDataStream& s) const
{
    s << conn << bitOrder;
}

void JZCommModbusServerConfig::loadFromStream(QDataStream& s)
{
    s >> conn >> bitOrder;
}