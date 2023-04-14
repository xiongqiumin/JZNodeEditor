
#include "JZNodeValue.h"

JZNodeValue::JZNodeValue()
{
    m_type = Node_value;

    JZNodePin out;
    out.setName("out");
    out.setFlag(Prop_out | Prop_edit | Prop_disp | Prop_param);
    
    addProp(out);
}

JZNodeValue::~JZNodeValue()
{
}

bool JZNodeValue::compiler(JZNodeCompiler *compiler,QString &error)
{   
    JZNodeIR ir(OP_set);
    ir.params << prop(0)->defaultValue().toString();
    return true;
}

void JZNodeValue::saveToStream(QDataStream &s) const
{
    JZNode::saveToStream(s);    
}

void JZNodeValue::loadFromStream(QDataStream &s)
{
    JZNode::loadFromStream(s);    
}

//JZNodePrint
JZNodePrint::JZNodePrint()
{
    m_type = Node_print;

    JZNodePin in;
    in.setName("in");
    in.setFlag(Prop_in | Prop_disp | Prop_param);

    addProp(in);
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


//JZNodeSet
JZNodeSet::JZNodeSet()
{
    m_type = Node_set;

    JZNodePin name;
    name.setName("name");
    name.setFlag(Prop_in | Prop_disp | Prop_param);

    JZNodePin value;
    value.setName("value");
    value.setFlag(Prop_in | Prop_disp | Prop_param);

    addProp(name);
    addProp(value);
}

JZNodeSet::~JZNodeSet()
{
}

void JZNodeSet::saveToStream(QDataStream &s) const
{
    JZNode::saveToStream(s);
}

void JZNodeSet::loadFromStream(QDataStream &s)
{
    JZNode::loadFromStream(s);
}


bool JZNodeSet::compiler(JZNodeCompiler *compiler,QString &error)
{
    return true;
}

//JZNodeGet
JZNodeGet::JZNodeGet()
{
    m_type = Node_get;

    JZNodePin name;
    name.setName("name");
    name.setFlag(Prop_in | Prop_disp);

    JZNodePin value;
    value.setName("value");
    value.setFlag(Prop_out | Prop_disp);

    addProp(name);
    addProp(value);
}

JZNodeGet::~JZNodeGet()
{
}

void JZNodeGet::saveToStream(QDataStream &s) const
{
    JZNode::saveToStream(s);
}

void JZNodeGet::loadFromStream(QDataStream &s)
{
    JZNode::loadFromStream(s);
}


bool JZNodeGet::compiler(JZNodeCompiler *compiler,QString &error)
{
    return true;
}
