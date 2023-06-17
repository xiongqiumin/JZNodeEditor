
#include "JZNodeValue.h"
#include "JZNodeCompiler.h"

//JZNodeCreate
JZNodeCreate::JZNodeCreate()
{
    m_name = "createObject";
    m_type = Node_create;

    addFlowIn();
    addFlowOut();

    addParamIn("Class",Prop_edit | Prop_dispName | Prop_dispValue);
    addParamOut("Return", Prop_dispName);
}

JZNodeCreate::~JZNodeCreate()
{

}

void JZNodeCreate::setClassName(QString name)
{
    setPropValue(paramIn(0),name);
}

void JZNodeCreate::saveToStream(QDataStream &s) const
{
    JZNode::saveToStream(s);
}

void JZNodeCreate::loadFromStream(QDataStream &s)
{
    JZNode::loadFromStream(s);
}

bool JZNodeCreate::compiler(JZNodeCompiler *c,QString &error)
{
    c->addFlowInput(m_id);

    int in_id = c->paramId(m_id,paramIn(0));
    int out_id = c->paramId(m_id,paramOut(0));
    JZNodeIRParam irIn = irId(in_id);
    JZNodeIRParam irOut = irId(out_id);

    c->addCall(irLiteral("createObject"),{irIn},{irOut});
    c->addFlowOutput(m_id);
    c->addJumpNode(flowOut());
    return true;
}

//JZNodeLiteral
JZNodeLiteral::JZNodeLiteral()
{
    m_type = Node_literal;
    addParamOut("out", Prop_dispValue);
    prop(paramOut(0))->setDataType({Type_none});
}

JZNodeLiteral::~JZNodeLiteral()
{
}

int JZNodeLiteral::dataType()
{
    return prop(paramOut(0))->dataType()[0];
}

void JZNodeLiteral::setDataType(int type)
{
    prop(paramOut(0))->setDataType({type});
}

QVariant JZNodeLiteral::literal() const
{
    return propValue(paramOut(0));
}

void JZNodeLiteral::setLiteral(QVariant value)
{
    setPropValue(paramOut(0),value);
}

bool JZNodeLiteral::compiler(JZNodeCompiler *c,QString &error)
{   
    int id = c->paramId(m_id,paramOut(0));
    c->addSetVariable(irId(id),irLiteral(literal()));
    return true;
}

void JZNodeLiteral::saveToStream(QDataStream &s) const
{
    JZNode::saveToStream(s);        
}

void JZNodeLiteral::loadFromStream(QDataStream &s)
{
    JZNode::loadFromStream(s);    
}

//JZNodePrint
JZNodePrint::JZNodePrint()
{
    m_type = Node_print;    
    addParamIn("");
}

JZNodePrint::~JZNodePrint()
{
}

void JZNodePrint::saveToStream(QDataStream &s) const
{
    JZNode::saveToStream(s);
}

void JZNodePrint::loadFromStream(QDataStream &s)
{
    JZNode::loadFromStream(s);
}

bool JZNodePrint::compiler(JZNodeCompiler *compiler,QString &error)
{
    return true;
}

//JZNodeParam
JZNodeParam::JZNodeParam()
{
    m_name = "get";
    m_type = Node_param;
    m_flag = Node_propVariable;
    addParamOut("name",Prop_dispValue | Prop_edit);
}

JZNodeParam::~JZNodeParam()
{
}

QString JZNodeParam::variable() const
{
    return propName(paramIn(0));
}

void JZNodeParam::setVariable(const QString &name)
{
    setName(name);
    setPropName(paramOut(0),name);
}

bool JZNodeParam::compiler(JZNodeCompiler *c,QString &error)
{
    int out_id = c->paramId(m_id,paramOut(0));
    JZNodeIRParam ref = irRef(variable());
    c->addSetVariable(irId(out_id),ref);
    return true;
}

void JZNodeParam::saveToStream(QDataStream &s) const
{
    JZNode::saveToStream(s);
}

void JZNodeParam::loadFromStream(QDataStream &s)
{
    JZNode::loadFromStream(s);
}

//JZNodeThis
JZNodeThis::JZNodeThis()
{
    m_name = "this";
    m_type = Node_this;
    addParamOut("this",Prop_dispName);
}

JZNodeThis::~JZNodeThis()
{

}

void JZNodeThis::saveToStream(QDataStream &s) const
{
    JZNode::saveToStream(s);
}

void JZNodeThis::loadFromStream(QDataStream &s)
{
    JZNode::loadFromStream(s);
}

bool JZNodeThis::compiler(JZNodeCompiler *c,QString &error)
{
    int out_id = c->paramId(m_id,paramOut(0));
    c->addSetVariable(irId(out_id),irThis());
    return true;
}

//JZNodeSetParam
JZNodeSetParam::JZNodeSetParam()
{
    m_type = Node_setParam;
    m_name = "set";
    m_flag = Node_propVariable;

    addFlowIn();    
    addFlowOut();
    addParamIn("",Prop_dispName | Prop_editName | Prop_edit | Prop_dispValue);
    addParamOut("");
}

JZNodeSetParam::~JZNodeSetParam()
{
}

QString JZNodeSetParam::variable() const
{
    return propName(paramIn(0));
}

void JZNodeSetParam::setVariable(const QString &name)
{
    setPropName(paramIn(0),name);
}


void JZNodeSetParam::saveToStream(QDataStream &s) const
{
    JZNode::saveToStream(s);
}

void JZNodeSetParam::loadFromStream(QDataStream &s)
{
    JZNode::loadFromStream(s);
}

bool JZNodeSetParam::compiler(JZNodeCompiler *c,QString &error)
{
    if(!c->addFlowInput(m_id))
        return false;

    int id = c->paramId(m_id,paramIn(0));
    int m_out = c->paramId(m_id,paramOut(0));

    JZNodeIRParam ref = irRef(variable());
    c->addSetVariable(ref,irId(id));
    c->addSetVariable(irId(m_out),irId(id));
    c->addFlowOutput(m_id);
    c->addJumpNode(flowOut());
    return true;
}

//JZNodeSetParamData
JZNodeSetParamData::JZNodeSetParamData()
{
    m_name = "set";
    m_type = Node_setParamData;
    m_flag = Node_propVariable;
    addParamIn("",Prop_dispName | Prop_dispValue | Prop_edit);
}

JZNodeSetParamData::~JZNodeSetParamData()
{
}


QString JZNodeSetParamData::variable() const
{
    return propName(paramIn(0));
}

void JZNodeSetParamData::setVariable(const QString &name)
{
    setPropName(paramIn(0),name);
}

void JZNodeSetParamData::saveToStream(QDataStream &s) const
{
    JZNode::saveToStream(s);
}

void JZNodeSetParamData::loadFromStream(QDataStream &s)
{
    JZNode::loadFromStream(s);
}

bool JZNodeSetParamData::compiler(JZNodeCompiler *c,QString &error)
{
    if(!c->addDataInput(m_id))
        return false;

    QString param_name = variable();
    int id = c->paramId(m_id,paramIn(0));
    c->addSetVariable(irRef(param_name),irId(id));
    return true;
}
