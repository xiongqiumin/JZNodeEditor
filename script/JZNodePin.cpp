#include "JZNodePin.h"

JZNodePin::JZNodePin()
{
    m_id = INVALID_ID;
    m_flag = Prop_none;
    m_dataType = Type_none;
}

JZNodePin::JZNodePin(QString name, int dataType, int flag)
{
    m_name = name;
    m_dataType = dataType;
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

void JZNodePin::setDataType(int type)
{
    m_dataType = type;
}

int JZNodePin::dataType() const
{
    return m_dataType;
}

void JZNodePin::setDefaultValue(QVariant value)
{
    m_default = value;
}

QVariant JZNodePin::defaultValue() const
{
    return m_default;
}

QDataStream &operator<<(QDataStream &s, const JZNodePin &param)
{
    s << param.m_id;
    s << param.m_name;
    s << param.m_flag;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodePin &param)
{
    s >> param.m_id;
    s >> param.m_name;
    s >> param.m_flag;
    return s;
}
