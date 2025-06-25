#include "JZNetSimulator.h"

JZNetSimulatorConfig::JZNetSimulatorConfig()
{
}

QDataStream &operator<<(QDataStream &s, const JZNetSimulatorConfig &param)
{
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNetSimulatorConfig &param)
{
    return s;
}

//JZNetSimulator
JZNetSimulator::JZNetSimulator()
{

}

JZNetSimulator::~JZNetSimulator()
{

}

bool JZNetSimulator::isOpen() 
{

}

bool JZNetSimulator::open() 
{

}

void JZNetSimulator::close() 
{

}

void JZNetSimulator::setConfig(const QByteArray &buffer) 
{

}

QByteArray JZNetSimulator::getConfig() 
{

}