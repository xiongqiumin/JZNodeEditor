#include "JZNodeEnum.h"
#include "JZNodeType.h"

void JZNodeEnumDefine::init(QString name, QStringList keys, QVector<int> values)
{
    m_type = Type_none;
    m_name = name;
    m_keys = keys;
    m_values = values;
}

void JZNodeEnumDefine::setType(int type)
{
    m_type = type;
}

int JZNodeEnumDefine::type() const
{
    return m_type;
}

QString JZNodeEnumDefine::name() const
{
    return m_name;
}

int JZNodeEnumDefine::count() const
{
    return m_keys.size();
}

QString JZNodeEnumDefine::key(int index) const
{
    return m_keys[index];
}

QString JZNodeEnumDefine::defaultKey() const
{
    return m_keys[0];
}

int JZNodeEnumDefine::value(int index) const
{
    return m_values[index];
}

int JZNodeEnumDefine::defaultValue() const
{
    return keyToValue(defaultKey());
}

int JZNodeEnumDefine::keyToValue(QString key) const
{
    int index = m_keys.indexOf(key);
    if (index >= 0)
        return m_values[index];
    else
        return -1;
}

QString JZNodeEnumDefine::valueToKey(int value) const
{
    int index = m_values.indexOf(value);
    if (index >= 0)
        return m_keys[index];
    else
        return -1;
}