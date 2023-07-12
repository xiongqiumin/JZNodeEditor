#include "JZNodeBind.h"

namespace jzbind
{

void *createClassAssert()
{
    Q_ASSERT(0);
    return nullptr;
}

void destoryClassAssert(void *)
{
    Q_ASSERT(0);
}

void copyClassAssert(void *,void *)
{
    Q_ASSERT(0);
}

template<>
QVariant getValue<QVariant>(QVariant v,std::false_type)
{
    return v;
}

template<>
QVariant getReturn(QVariant value,bool)
{
    return value;
}

template<>
QVariant getReturn(QString value,bool)
{
    return value;
}
}


