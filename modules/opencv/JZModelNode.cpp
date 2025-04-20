#include "JZModelNode.h"
#include "JZNodeCompiler.h"

//JZModelInitNode
JZModelInitNode::JZModelInitNode()
{
    m_name = "ModelInit";
    m_type = Node_ModelInit;

    addFlowIn();
    addFlowOut();
}

JZModelInitNode::~JZModelInitNode()
{

}

bool JZModelInitNode::compiler(JZNodeCompiler *c, QString &error)
{
    c->addNodeEnter(m_id);
    return true;
}

//JZModelSettingNode
JZModelSettingNode::JZModelSettingNode()
{
    m_name = "ModelSetting";
    m_type = Node_ModelSetting;

    addFlowIn();
    addFlowOut();
}

JZModelSettingNode::~JZModelSettingNode()
{
}

bool JZModelSettingNode::compiler(JZNodeCompiler *c, QString &error)
{
    c->addNodeEnter(m_id);
    return true;
}