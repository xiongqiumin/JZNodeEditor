#include "JZNodeEnum.h"
#include "JZNodeType.h"

void JZNodeEnumDefine::init(QString name, QStringList keys, QVector<int> values)
{
    m_type = Type_none;
    m_name = name;
    m_keys = keys;
    m_values = values;
    m_isFlag = false;
    m_default = -1;
}

void JZNodeEnumDefine::setType(int type)
{
    m_type = type;
}

int JZNodeEnumDefine::type() const
{
    return m_type;
}

void JZNodeEnumDefine::setFlag(bool isFlag)
{
    m_isFlag = isFlag;
}

bool JZNodeEnumDefine::flag() const
{
    return m_isFlag;
}

QString JZNodeEnumDefine::name() const
{
    return m_name;
}

int JZNodeEnumDefine::count() const
{
    return m_keys.size();
}

bool JZNodeEnumDefine::hasKey(const QString &key) const
{
    return m_keys.contains(key);
}

QString JZNodeEnumDefine::key(int index) const
{
    return m_keys[index];
}

QString JZNodeEnumDefine::defaultKey() const
{
    return m_keys[defaultValue()];
}

int JZNodeEnumDefine::value(int index) const
{
    return m_values[index];
}

int JZNodeEnumDefine::defaultValue() const
{
    if (m_default != -1)
        return m_values[m_default];
    else
    {
        int index = std::min_element(m_values.begin(), m_values.end()) - m_values.begin();
        return m_values[index];
    }
}

void JZNodeEnumDefine::setDefaultValue(int value)
{
    m_default = m_values.indexOf(value);
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