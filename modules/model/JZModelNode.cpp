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
    if (!c->checkVariableType("this.modelManager", env->nameToType("JZModelManager*"), error))
        return false;

    if (!c->addFlowInput(m_id, error))
        return false;    

    QList<JZNodeIRParam> in, out;
    in << irRef("this.modelManager") << irLiteral(JZNodeUtils::toBuffer(m_config));
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

    int in = addParamIn("name", Pin_constValue | Pin_noCompiler);
    setPinTypeString(in);

    int in_frame = addParamIn("frame");
    setPinType(in_frame, { "Mat" });

    int out = addParamOut("result");
    setPinType(out, { "QList<JZYoloResult>" });
}

JZNodeModelForward::~JZNodeModelForward()
{

}

void JZNodeModelForward::setModel(QString name)
{
    setParamInValue(0,name);
}

QString JZNodeModelForward::model()
{
    return paramInValue(0);
}

void JZNodeModelForward::saveToStream(QDataStream& s) const
{
    JZNode::saveToStream(s);    
}

void JZNodeModelForward::loadFromStream(QDataStream& s)
{
    JZNode::loadFromStream(s);    
}

bool JZNodeModelForward::updateNode(QString &error)
{
    auto init_script = m_file->getClassItem()->memberFunction("init");
    auto init_list = init_script->findNodeByType(Node_ModelInit);
    if(init_list.size() != 1)
    {
        error = "no JZNodeModelInit in init";
        return false;
    }

    auto env = m_file->project()->environment();
    if (!JZNodeCompiler::checkVariableType(m_file,"this.modelManager", env->nameToType("JZModelManager"), error))
        return false;



    return true;
}

bool JZNodeModelForward::compiler(JZNodeCompiler *c, QString &error)
{
    auto env = c->env();
    if (!c->addFlowInput(m_id, error))
        return false;

    int model_id = c->allocStack("JZModel*");
    c->addCall("JZModelGet", { irRef("this.modelManager"), irLiteral(model()) }, { irId(model_id) });

    int in_id = c->paramId(m_id, paramIn(1));
    int out_id = c->paramId(m_id, paramOut(0));
    c->addCallConvert("JZYolo::forward", { irId(model_id) ,irId(in_id) }, {  irId(out_id) });

    c->addFlowOutput(m_id);
    return true;
}