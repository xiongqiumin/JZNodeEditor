#include "JZModuleComm.h"
#include "JZCommNode.h"
#include "JZScriptEnvironment.h"

//JZModuleComm
JZModuleComm::JZModuleComm()
{    
}

JZModuleComm::~JZModuleComm()
{
}

void JZModuleComm::regist(JZScriptEnvironment *env)
{
    env->factoryManager()->registNode(Node_CommInit,createJZNode<JZCommInitNode>);    
}

void JZModuleComm::unregist(JZScriptEnvironment *env)
{

}