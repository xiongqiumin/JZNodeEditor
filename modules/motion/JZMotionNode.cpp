#include "JZMotionNode.h"
#include "JZNodeCompiler.h"
#include "JZNodeUtils.h"

//JZNodeMotionInit
JZNodeMotionInit::JZNodeMotionInit()
{
    m_type = Node_MotionInit;
    m_name = "运控初始化";
}

JZNodeMotionInit::~JZNodeMotionInit()
{
}

bool JZNodeMotionInit::compiler(JZNodeCompiler *c, QString &error)
{
    return false;
}

//JZNodeMotionZero    
JZNodeMotionZero::JZNodeMotionZero()
{
    m_type = Node_MotionZero;
    m_name = "运控归零";
}

JZNodeMotionZero::~JZNodeMotionZero()
{
}

bool JZNodeMotionZero::compiler(JZNodeCompiler *c, QString &error)
{
    return false;
}


//JZNodeMotionMove
JZNodeMotionMove::JZNodeMotionMove()
{
    m_type = Node_MotionMove;
    m_name = "运控移动";
}

JZNodeMotionMove::~JZNodeMotionMove()
{
}

bool JZNodeMotionMove::compiler(JZNodeCompiler *c, QString &error)
{
    return false;
}