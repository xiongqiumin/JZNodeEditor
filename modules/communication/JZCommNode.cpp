#include "JZCommNode.h"
#include "JZNodeCompiler.h"

//JZCommInitNode
JZCommInitNode::JZCommInitNode()
{
    m_name = "CommInit";
    m_type = Node_CommInit;

    addFlowIn();
    addFlowOut();
}

JZCommInitNode::~JZCommInitNode()
{

}

bool JZCommInitNode::compiler(JZNodeCompiler *c, QString &error)
{
    c->addNodeEnter(m_id);
    return true;
}