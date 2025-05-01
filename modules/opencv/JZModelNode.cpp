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
    auto env = c->env();
    if (!c->checkVariableType("this.modelManager", env->nameToType("JZModelManager"), error))
        return false;

    if (!c->addFlowInput(m_id, error))
        return false;    

    int id = c->allocStack(Type_byteArray);
    c->addSetBuffer(irId(id), JZNodeUtils::toBuffer(m_config));

    QList<JZNodeIRParam> in, out;
    in << irRef("this.modelManager") << irId(id);
    c->addCallConvert("JZModelInit", in, out);
    return true;
}

void JZNodeModelInit::saveToStream(QDataStream& s) const
{
    JZNode::saveToStream(s);
    s << m_config;
}

void JZNodeModelInit::loadFromStream(QDataStream& s)
{
    JZNode::loadFromStream(s);
    s >> m_config;
}

//JZNodeModelForward
JZNodeModelForward::JZNodeModelForward()
{
    m_name = "modelForward";
    m_type = Node_ModelForward;

    addFlowIn();
    addFlowOut();

    int in = addParamIn("frame");
    setPinType(in, { "Mat" });

    int out = addParamOut("result");
    setPinType(out, { "QList<JZYoloResult>" });
}

JZNodeModelForward::~JZNodeModelForward()
{

}

void JZNodeModelForward::setModel(QString name)
{
    m_model = name;
}

QString JZNodeModelForward::model()
{
    return m_model;
}

void JZNodeModelForward::saveToStream(QDataStream& s) const
{
    JZNode::saveToStream(s);
    s << m_model;
}

void JZNodeModelForward::loadFromStream(QDataStream& s)
{
    JZNode::loadFromStream(s);
    s >> m_model;
}

bool JZNodeModelForward::compiler(JZNodeCompiler *c, QString &error)
{
    auto env = c->env();
    if (!c->checkVariableType("this.modelManager", env->nameToType("JZModelManager"), error))
        return false;

    if (!c->addFlowInput(m_id, error))
        return false;

    int model_id = c->allocStack("JZModel*");
    c->addCall("JZModelGet", { irRef("this.modelManager"), irLiteral(m_model) }, { irId(model_id) });

    int in_id = c->paramId(m_id, paramIn(0));
    int out_id = c->paramId(m_id, paramOut(0));
    c->addCallConvert("JZYolo::forward", { irId(model_id) ,irId(in_id) }, {  irId(out_id) });

    c->addFlowOutput(m_id);
    return true;
}