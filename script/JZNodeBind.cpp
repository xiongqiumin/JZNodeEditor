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

bool equalClassAssert(void *,void *)
{
    Q_ASSERT(0);
    return false;
}
// 函数调用
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
QVariant fromVariant<QVariant>(const QVariant &v, std::false_type)
{
    Q_ASSERT(v.userType() == qMetaTypeId<JZNodeVariantAny>());
    auto ptr = (JZNodeVariantAny*)v.data();
    return ptr->value;    
}

template<>
JZNodeVariantAny fromVariant<JZNodeVariantAny>(const QVariant &v, std::false_type)
{
    Q_ASSERT(v.userType() == qMetaTypeId<JZNodeVariantAny>());
    return v.value<JZNodeVariantAny>();
}

template<>
JZFunctionPointer fromVariant<JZFunctionPointer>(const QVariant &v, std::false_type)
{
    Q_ASSERT(v.userType() == qMetaTypeId<JZFunctionPointer>());
    return v.value<JZFunctionPointer>();
}

// 函数返回
template<>
QVariant toVariant(QVariant v)
{
    Q_ASSERT(v.userType() != qMetaTypeId<JZNodeVariantAny>());
    JZNodeVariantAny any;
    any.value = v;
    return QVariant::fromValue(any);
}

template<>
QVariant toVariant(JZNodeVariantAny value)
{
    return QVariant::fromValue(value);
}

template<>
QVariant toVariant(JZFunctionPointer ptr)
{
    return QVariant::fromValue(ptr);
}

template<>
QVariant toVariant(QString value)
{
    return value;
}

template<>
void getReturn(const QVariantList &)
{

}

}