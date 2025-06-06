#include "JZMotionMotean.h"

JZMotionMoteanConfig::JZMotionMoteanConfig()
{
    type = Motion_Motean;
}

void JZMotionMoteanConfig::saveToStream(QDataStream& s) const
{
    JZMotionConfig::saveToStream(s);
    s << portName << baud << dataBit << parityBit << stopBit;
}

void JZMotionMoteanConfig::loadFromStream(QDataStream& s)
{
    JZMotionConfig::loadFromStream(s);
    s >> portName >> baud >> dataBit >> parityBit >> stopBit;
}

//JZMotionMotean
JZMotionMotean::JZMotionMotean(QObject *parent)
    :JZMotion(parent)
{
}

JZMotionMotean::~JZMotionMotean()
{
}

bool JZMotionMotean::isInit() 
{ 
    return false; 
}

bool JZMotionMotean::init() 
{ 
    return false; 
}

void JZMotionMotean::deinit() 
{
}

bool JZMotionMotean::isMoving() const
{
    return true;
}

void JZMotionMotean::gotoZero()
{
}

void JZMotionMotean::moveTo(double x, double y, double z)
{
}