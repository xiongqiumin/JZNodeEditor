#include "JZModelNode.h"
#include "JZNodeCompiler.h"

//JZNodeModelForward
JZNodeModelForward::JZNodeModelForward()
{
    m_name = "modelForward";
    m_type = Node_ModelForward;

    addFlowIn();
    addFlowOut();
}

JZNodeModelForward::~JZNodeModelForward()
{

}

bool JZNodeModelForward::compiler(JZNodeCompiler *c, QString &error)
{
    c->addNodeEnter(m_id);
    return true;
}