#include "JZNodeEnum.h"

int JZNodeEnum::count()
{
    return m_keys.size();
}

QString JZNodeEnum::key(int index)
{
    return m_keys[index];
}

int JZNodeEnum::value(int index)
{
    return m_values[index];
}