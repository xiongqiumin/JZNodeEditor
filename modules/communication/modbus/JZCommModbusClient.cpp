#include "JZCommModbusClient.h"

//JZCommModbusInfo
JZCommModbusClientConfig::JZCommModbusClientConfig()
{
    bitOrder = QDataStream::LittleEndian;
}

void JZCommModbusClientConfig::saveToStream(QDataStream& s) const
{
    s << conn << bitOrder;
}

void JZCommModbusClientConfig::loadFromStream(QDataStream& s)
{
    s >> conn >> bitOrder;
}