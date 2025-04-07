#include <QSerialPort>
#include <QPushButton>
#include "JZModuleCamera.h"

//JZModuleCamera
JZModuleCamera::JZModuleCamera()
{    
    m_name = "modbus";
}

JZModuleCamera::~JZModuleCamera()
{
}

void JZModuleCamera::regist(JZScriptEnvironment *env)
{
}

void JZModuleCamera::unregist(JZScriptEnvironment *env)
{

}