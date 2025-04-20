#include "JZModuleComm.h"
#include "JZCommNode.h"
#include "JZNodeFactory.h"

//JZModuleComm
JZModuleComm::JZModuleComm()
{    
}

JZModuleComm::~JZModuleComm()
{
}

void JZModuleComm::regist(JZScriptEnvironment *env)
{    
}

void JZModuleComm::unregist(JZScriptEnvironment *env)
{

}

void JZModuleCommNodeInit()
{
    JZNodeFactory::instance()->registNode(Node_CommInit,createJZNode<JZCommInitNode>);    
}