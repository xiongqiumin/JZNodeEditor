#include "JZModelNode.h"
#include "JZNodeCompiler.h"
#include "JZNodeUtils.h"
#include "../JZModuleCompiler.h"

static bool checkHasModule(JZScriptItem *script, const QString &model, QString &error)
{
    auto env = script->project()->environment();
    if (!JZNodeCompiler::checkVariableType(script, "this.modelManager", env->nameToType("JZModelManager*"), error))
        return false;

    auto init_node = getInitNode(script, Node_ModelInit);
    if (!init_node)
    {
        error = "没有init节点";
        return false;
    }

    JZNodeModelInit *node = dynamic_cast<JZNodeModelInit*>(init_node);
    if (node->config().indexOfModel(model))
    {
        error = "没有模型名称为" + model;
        return false;
    }

    return true;

}
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
    m_name = "模型推理";
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
    auto env = m_file->project()->environment();
    if (!checkHasModule(m_file,model(),error))
        return false;

    return true;
}

bool JZNodeModelForward::compiler(JZNodeCompiler *c, QString &error)
{
    auto env = c->env();
    if (!c->addFlowInput(m_id, error))
        return false;

    int in_id = c->paramId(m_id, paramIn(1));
    int out_id = c->paramId(m_id, paramOut(0));
    c->addCallConvert("JZYoloForward", { irRef("this.modelManager"), irLiteral(model()) ,irId(in_id) }, {  irId(out_id) });

    c->addFlowOutput(m_id);
    return true;
}