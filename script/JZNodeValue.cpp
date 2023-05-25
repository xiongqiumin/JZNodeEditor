
#include "JZNodeValue.h"
#include "JZNodeCompiler.h"

//JZNodeParam
JZNodeParam::JZNodeParam()
{
    m_type = Node_param;
    m_out = addParamOut("out");
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

//JZNodeLiteral
JZNodeLiteral::JZNodeLiteral()
{
    m_type = Node_literal;
    m_out = addParamOut("out");
}

JZNodeLiteral::~JZNodeLiteral()
{
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

//JZNodeSetParam
JZNodeSetParam::JZNodeSetParam()
{
    m_type = Node_setParam;

    addFlowIn();    
    addFlowOut();
    addParamIn("value",Prop_edit);
    addParamOut("value");
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
    c->addFlowInput(m_id);
    int id = c->paramId(m_id,paramIn(0));
    JZNodeIRParam ref = irRef(m_param);
    if(m_local)
        ref = c->localVariable(ref);

    c->addSetVariable(ref,irId(id));    
    c->addJumpNode(flowOut());
    return true;
}

//JZNodeSetParamData
JZNodeSetParamData::JZNodeSetParamData()
{
    m_type = Node_setParamData;
    addParamIn("value");
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
    c->addDataInput(m_id);

    int id = c->paramId(m_id,paramIn(0));
    c->addSetVariable(irRef(m_param),irId(id));
    return true;
}
