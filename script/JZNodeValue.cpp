
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
    m_out = addParamOut("out", Prop_dispValue);
    prop(m_out)->setDataType({Type_none});
}

JZNodeLiteral::~JZNodeLiteral()
{
}

int JZNodeLiteral::dataType()
{
    return prop(m_out)->dataType()[0];
}

void JZNodeLiteral::setDataType(int type)
{
    prop(m_out)->setDataType({type});
}

QVariant JZNodeLiteral::literal() const
{
    return propValue(m_out);
}

void JZNodeLiteral::setLiteral(QVariant value)
{
    setPropValue(m_out,value);
}

bool JZNodeLiteral::compiler(JZNodeCompiler *c,QString &error)
{   
    int id = c->paramId(m_id,m_out);
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
    m_out = addParamOut("",Prop_dispName | Prop_editName);
}

JZNodeParam::~JZNodeParam()
{
}

QString JZNodeParam::paramId() const
{
    return m_param;
}

void JZNodeParam::setParamId(QString paramId,bool global)
{
    setName(paramId);
    setPropName(paramOut(0),paramId);
    m_param = paramId;
    m_local = !global;
}

bool JZNodeParam::compiler(JZNodeCompiler *c,QString &error)
{
    int out_id = c->paramId(m_id,paramOut(0));
    JZNodeIRParam ref = irRef(m_param);
    if(m_local)
        ref = c->localVariable(ref);
    c->addSetVariable(irId(out_id),ref);
    return true;
}

void JZNodeParam::saveToStream(QDataStream &s) const
{
    JZNode::saveToStream(s);
    s << m_out << m_param;
}

void JZNodeParam::loadFromStream(QDataStream &s)
{
    JZNode::loadFromStream(s);
    s >> m_out >> m_param;
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

    addFlowIn();    
    addFlowOut();
    addParamIn("",Prop_dispName | Prop_dispValue | Prop_edit);
    addParamOut("");
}

JZNodeSetParam::~JZNodeSetParam()
{
}

QString JZNodeSetParam::paramId() const
{
    return m_param;
}

void JZNodeSetParam::setParamId(QString paramId,bool global)
{
    setPropName(paramIn(0),paramId);
    m_param = paramId;
    m_local = !global;
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
    JZNodeIRParam ref = irRef(m_param);
    if(m_local)
        ref = c->localVariable(ref);

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
    addParamIn("",Prop_dispName | Prop_dispValue | Prop_edit);
}

JZNodeSetParamData::~JZNodeSetParamData()
{
}

QString JZNodeSetParamData::paramId() const
{
    return m_param;
}

void JZNodeSetParamData::setParamId(QString paramId)
{
    m_param = paramId;
    setPropName(paramIn(0),paramId);
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

    int id = c->paramId(m_id,paramIn(0));
    c->addSetVariable(irRef(m_param),irId(id));
    return true;
}
