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
QString* fromVariant<QString*>(const QVariant &v, std::true_type)
{
    if (v.type() != QVariant::String)
        return nullptr;
    return (QString*)v.data();
}

template<>
QString fromVariant<QString>(const QVariant &v, std::false_type)
{
    Q_ASSERT(v.type() == QVariant::String);
    return v.toString();
}

template<>
QVariant toVariant(JZNodeVariantAny value)
{
    return QVariant::fromValue(value);
}

template<>
QVariant toVariant(QString value)
{
    return value;
}
}


