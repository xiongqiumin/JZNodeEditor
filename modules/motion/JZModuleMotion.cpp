#include <opencv2/opencv.hpp>
#include <QBuffer>
#include "JZModuleMotion.h"
#include "JZScriptEnvironment.h"
#include "JZNodeBind.h"
#include "JZMotionMotean.h"

using namespace cv;

//JZModuleMotion
JZModuleMotion::JZModuleMotion()
{        
    m_name = "motion";

    auto inst = JZModuleConfigFactory<JZMotionMoteanConfig>::instance();
    inst->regist(Motion_Motean, JZModuleConfigCreator<JZMotionMoteanConfig>);
}

JZModuleMotion::~JZModuleMotion()
{
}

void JZModuleMotion::regist(JZScriptEnvironment *env)
{    
    auto node_inst = env->nodeFactory();
    node_inst->registNode(Node_MotionInit, createJZNode<JZNodeMotionInit>);
    node_inst->registNode(Node_MotionZero, createJZNode<JZNodeMotionZero>);
    node_inst->registNode(Node_MotionMove, createJZNode<JZNodeMotionMove>);    
}

void JZModuleMotion::unregist(JZScriptEnvironment *env)
{
}