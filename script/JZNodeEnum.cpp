#include "JZNodeEnum.h"
#include "JZNodeType.h"

void JZNodeEnumDefine::init(QString name, QStringList keys, QVector<int> values)
{
    m_type = Type_none;
    m_name = name;
    m_keys = keys;
    m_values = values;
    m_isFlag = false;
    m_flagEnum = -1;
    m_default = -1;
}

void JZNodeEnumDefine::setId(int type)
{
    m_type = type;
}

int JZNodeEnumDefine::id() const
{
    return m_type;
}

void JZNodeEnumDefine::setFlag(bool isFlag, int enumId)
{
    m_isFlag = isFlag;
    m_flagEnum = enumId;
}

bool JZNodeEnumDefine::isFlag() const
{
    return m_isFlag;
}

int JZNodeEnumDefine::flagEnum() const
{
    return m_flagEnum;
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
    if (m_isFlag)
    {
        auto list = key.split("|");
        for (int i = 0; i < list.size(); i++)
        {
            if (!m_keys.contains(list[i]))
                return false;
        }
        return true;
    }
    else
        return m_keys.contains(key);
}

QString JZNodeEnumDefine::key(int index) const
{
    return m_keys[index];
}

QString JZNodeEnumDefine::defaultKey() const
{
    return valueToKey(defaultValue());
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
        if (m_values.indexOf(0) >= 0)
            return 0;

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
    if (m_isFlag)
    {
        auto list = key.split("|");
        int value = 0;
        for (int i = 0; i < list.size(); i++)
        {
            int key_idx = m_keys.indexOf(list[i]);
            if (key_idx == -1)
                return -1;
            
            value |= m_values[i];
        }
        return value;
    }
    else
    {
        int index = m_keys.indexOf(key);
        if (index >= 0)
            return m_values[index];
        else
            return -1;
    }
}

QString JZNodeEnumDefine::valueToKey(int value) const
{
    if (m_isFlag)
    {        
        QVector<int> key_indexs;
        for (int i = 0; i < m_values.size(); i++)
        {
            if (value & m_values[i])
                key_indexs << i;
        }

        QVector<int> remove;
        for (int i = 0; i < key_indexs.size(); i++)
        {
            for (int j = 0; j < key_indexs.size(); j++)
            {
                if (i != j)
                {
                    int v_i = m_values[key_indexs[i]];
                    int v_j = m_values[key_indexs[j]];

                    bool con = (v_i > v_j) && ((v_i & v_j) == v_j);
                    if (con && !remove.contains(v_j))
                        remove << v_j;                    
                }
            }
        }

        QStringList list;
        for (int i = 0; i < key_indexs.size(); i++)
        {
            if (!remove.contains(i))
                list << m_keys[key_indexs[i]];
        }
        return list.join("|");
    }
    else
    {
        int index = m_values.indexOf(value);
        if (index >= 0)
            return m_keys[index];
        else
            return QString();
    }
}
