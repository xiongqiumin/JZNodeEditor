﻿#include "JZNodePin.h"

//JZNodePin
JZNodePin::JZNodePin()
{
    m_id = INVALID_ID;
    m_flag = Prop_none;    
}

JZNodePin::JZNodePin(QString name, int dataType, int flag)
{
    m_name = name;
    m_flag = flag;    
}

JZNodePin::~JZNodePin()
{
}

void JZNodePin::setId(int id)
{
    m_id = id;
}

int JZNodePin::id() const
{
    return m_id;
}

void JZNodePin::setName(const QString &name)
{
    m_name = name;
}

const QString &JZNodePin::name() const
{
    return m_name;
}

void JZNodePin::setFlag(int flag)
{
    m_flag = flag;    
}

int JZNodePin::flag() const
{
    return m_flag;
}

bool JZNodePin::isInput() const
{
    return (m_flag & Prop_in);
}

bool JZNodePin::isOutput() const
{
    return (m_flag & Prop_out);
}

bool JZNodePin::isParam() const
{
    return (m_flag & Prop_param);
}

bool JZNodePin::isFlow() const
{
    return (m_flag & Prop_flow);
}

bool JZNodePin::isSubFlow() const
{
    return (m_flag & Prop_subFlow);
}

bool JZNodePin::isButton() const
{
    return (m_flag & Prop_button);
}

bool JZNodePin::isEditValue() const
{
    return (m_flag & Prop_editValue);
}

bool JZNodePin::isDispName() const
{
    return (m_flag & Prop_dispName);
}

bool JZNodePin::isDispValue() const
{
    return (m_flag & Prop_dispValue);
}

bool JZNodePin::isLiteral() const
{
    return (m_flag & Prop_literal);
}

void JZNodePin::setDataType(QList<int> type)
{
    m_dataType = type;
}

QList<int> JZNodePin::dataType() const
{
    return m_dataType;
}

QString JZNodePin::value() const
{
    return m_value;
}

void JZNodePin::setValue(QString value)
{
    Q_ASSERT(JZNodeType::isNullptr(value) || JZNodeType::isBase(JZNodeType::variantType(value)));
    m_value = value;
}

void operator<<(JZProjectStream &s, const JZNodePin &param)
{
    s << param.m_id;
    s << param.m_name;    
    s << param.m_flag;     
    s << param.m_dataType;
    s << param.m_value;           
}

void operator>>(JZProjectStream &s, JZNodePin &param)
{
    s >> param.m_id;
    s >> param.m_name;    
    s >> param.m_flag;    
    s >> param.m_dataType;
    s >> param.m_value;       
}
