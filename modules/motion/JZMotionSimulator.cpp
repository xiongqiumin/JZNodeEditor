#include "JZMotionSimulator.h"

JZMotionSimulatorConfig::JZMotionSimulatorConfig()
{
    type = Motion_Simulator;
}

void JZMotionSimulatorConfig::saveToStream(QDataStream& s) const
{
    JZMotionConfig::saveToStream(s);
}

void JZMotionSimulatorConfig::loadFromStream(QDataStream& s)
{
    JZMotionConfig::loadFromStream(s);
}

//JZMotionSimulator
JZMotionSimulator::JZMotionSimulator(QObject *parent)
    :JZMotion(parent)
{
}

JZMotionSimulator::~JZMotionSimulator()
{
}

bool JZMotionSimulator::isInit() 
{ 
    return false; 
}

bool JZMotionSimulator::init() 
{ 
    return false; 
}

void JZMotionSimulator::deinit() 
{
}

bool JZMotionSimulator::isMoving() const
{
    return false;
}

void gotoZero()
{
}

void moveTo(double x, double y, double z)
{
}