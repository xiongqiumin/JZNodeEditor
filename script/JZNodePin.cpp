#include "JZNodePin.h"

JZParamDefine::JZParamDefine()
{
    dataType = Type_none;
    cref = false;
}

JZParamDefine::JZParamDefine(QString name,int dataType,const QVariant &v)
{
    this->name = name;
    this->dataType = dataType;
    this->value = v;    
    this->cref = false;
}

QDataStream &operator<<(QDataStream &s, const JZParamDefine &param)
{
    s << param.name;
    s << param.dataType;
    s << param.value;
    s << param.cref;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZParamDefine &param)
{
    s >> param.name;
    s >> param.dataType;
    s >> param.value;
    s >> param.cref;
    return s;
}

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

void JZNodePin::setName(QString name)
{
    m_name = name;
}

QString JZNodePin::name() const
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

int JZNodePin::pri() const
{
    if(m_flag & Prop_subFlow)
        return 0;
    else if(m_flag & Prop_flow)
        return 1;
    else if(m_flag & Prop_button)
        return 3;

    return 2;
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

bool JZNodePin::isEditable() const
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

QVariant JZNodePin::value() const
{
    return m_value;
}

void JZNodePin::setValue(QVariant value)
{
    m_value = value;
}

void JZNodePin::addAction(QString action)
{
    m_actionList.push_back(action);
}

QStringList JZNodePin::actionList()
{
    return m_actionList;
}

QDataStream &operator<<(QDataStream &s, const JZNodePin &param)
{
    s << param.m_id;
    s << param.m_name;
    s << param.m_flag;    
    s << param.m_dataType;
    s << param.m_value;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodePin &param)
{
    s >> param.m_id;
    s >> param.m_name;
    s >> param.m_flag;    
    s >> param.m_dataType;
    s >> param.m_value;
    return s;
}
