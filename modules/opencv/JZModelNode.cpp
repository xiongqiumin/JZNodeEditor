#include "JZModelNode.h"
#include "JZNodeCompiler.h"
#include "JZNodeUtils.h"

//JZNodeModelInit
JZNodeModelInit::JZNodeModelInit()
{
    m_name = "ModelInit";
    m_type = Node_ModelInit;

    addFlowIn();
    addFlowOut();
}

JZNodeModelInit::~JZNodeModelInit()
{

}

void JZNodeModelInit::setConfig(JZModelManagerConfig config)
{
    m_config = config;
}

JZModelManagerConfig JZNodeModelInit::config()
{
    return m_config;
}

bool JZNodeModelInit::compiler(JZNodeCompiler* c, QString& error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    auto env = c->env();
    if (!c->checkVariableType("this.modelManager", env->nameToType("JZModelManager"), error))
        return false;

    int id = c->allocStack(Type_byteArray);
    c->addSetBuffer(irId(id), JZNodeUtils::toBuffer(m_config));

    QList<JZNodeIRParam> in, out;
    in << irRef("this.modelManager") << irId(id);
    c->addCallConvert("JZModelInit", in, out);
    return true;
}

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