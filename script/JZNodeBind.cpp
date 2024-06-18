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
QString* getValue<QString*>(const QVariant &v, std::true_type)
{
    if (v.type() != QVariant::String)
        return nullptr;

    return (QString*)v.data();
}

template<>
QString getValue<QString>(const QVariant &v, std::false_type)
{
    return v.toString();
}

template<>
QVariant getReturn(JZNodeVariantAny value,bool)
{
    return QVariant::fromValue(value);
}

template<>
QVariant getReturn(QString value,bool)
{
    return value;
}
}


