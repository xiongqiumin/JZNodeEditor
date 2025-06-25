#include "JZSerialPortSimulator.h"

JZSerialPortConfig::JZSerialPortConfig()
{
}

QDataStream &operator<<(QDataStream &s, const JZSerialPortConfig &param)
{
    return s;
}

QDataStream &operator>>(QDataStream &s, JZSerialPortConfig &param)
{
    return s;
}


//JZSerialPortSimulator
JZSerialPortSimulator::JZSerialPortSimulator()
{

}

JZSerialPortSimulator::~JZSerialPortSimulator()
{

}

bool JZSerialPortSimulator::isOpen() 
{

}

bool JZSerialPortSimulator::open() 
{

}

void JZSerialPortSimulator::close() 
{

}

void JZSerialPortSimulator::setConfig(const QByteArray &buffer) 
{

}

QByteArray JZSerialPortSimulator::getConfig() 
{

}