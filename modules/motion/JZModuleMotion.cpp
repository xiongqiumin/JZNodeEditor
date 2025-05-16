#include <opencv2/opencv.hpp>
#include <QBuffer>
#include "JZModuleMotion.h"
#include "JZScriptEnvironment.h"
#include "JZNodeBind.h"

using namespace cv;

//JZModuleMotion
JZModuleMotion::JZModuleMotion()
{        
    m_name = "vision";
}

JZModuleMotion::~JZModuleMotion()
{
}

void JZModuleMotion::regist(JZScriptEnvironment *env)
{    
    
}

void JZModuleMotion::unregist(JZScriptEnvironment *env)
{
}