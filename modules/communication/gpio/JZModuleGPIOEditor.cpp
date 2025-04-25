#include <QSerialPort>
#include <QPushButton>
#include "JZModuleGPIO.h"

//JZModuleGPIO
JZModuleGPIO::JZModuleGPIO()
{    
    m_name = "modbus";
}

JZModuleGPIO::~JZModuleGPIO()
{
}

void JZModuleGPIO::regist(JZScriptEnvironment *env)
{
   
}

void JZModuleGPIO::unregist(JZScriptEnvironment *env)
{

}