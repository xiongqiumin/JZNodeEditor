﻿#include "JZNodeBind.h"
#include "JZNodeEngine.h"

namespace jzbind
{

static JZScriptEnvironment *g_bindEnv = nullptr;
JZCORE_EXPORT void setBindEnvironment(JZScriptEnvironment *env)
{    
    Q_ASSERT(env == nullptr || g_bindEnv == nullptr);
    g_bindEnv = env;
}

JZCORE_EXPORT JZScriptEnvironment *bindEnvironment()
{
    Q_ASSERT(g_bindEnv);
    return g_bindEnv;    
}

JZCORE_EXPORT JZScriptEnvironment *runtimeEnvironment()
{
    return g_engine->environment();    
}

JZCORE_EXPORT void *createClassAssert()
{
    Q_ASSERT(0);
    return nullptr;
}

JZCORE_EXPORT void destoryClassAssert(void *)
{
    Q_ASSERT(0);
}

JZCORE_EXPORT void copyClassAssert(void *,void *)
{
    Q_ASSERT(0);
}

JZCORE_EXPORT bool equalClassAssert(void *,void *)
{
    Q_ASSERT(0);
    return false;
}
// 函数调用
template<>
JZCORE_EXPORT QString* fromVariant<QString*>(const QVariant &v, std::true_type)
{
    if (v.type() != QVariant::String)
        return nullptr;
    return (QString*)v.data();
}

template<>
JZCORE_EXPORT bool fromVariant<bool>(const QVariant &v, std::false_type)
{
    return v.toBool();
}

template<>
JZCORE_EXPORT int fromVariant<int>(const QVariant &v, std::false_type)
{
    return v.toInt();
}

template<>
JZCORE_EXPORT qint64 fromVariant<qint64>(const QVariant &v, std::false_type)
{
    return v.value<qint64>();
}

template<>
JZCORE_EXPORT double fromVariant<double>(const QVariant &v, std::false_type)
{
    return v.toDouble();
}

template<>
JZCORE_EXPORT QString fromVariant<QString>(const QVariant &v, std::false_type)
{
    Q_ASSERT(v.type() == QVariant::String);
    return v.toString();
}

template<>
JZCORE_EXPORT QVariant fromVariant<QVariant>(const QVariant &v, std::false_type)
{
    Q_ASSERT(v.userType() == qMetaTypeId<JZNodeVariantAny>());
    auto ptr = (JZNodeVariantAny*)v.data();
    return ptr->value;    
}

template<>
JZCORE_EXPORT JZNodeVariantAny fromVariant<JZNodeVariantAny>(const QVariant &v, std::false_type)
{
    Q_ASSERT(v.userType() == qMetaTypeId<JZNodeVariantAny>());
    return v.value<JZNodeVariantAny>();
}

template<>
JZCORE_EXPORT JZFunctionPointer fromVariant<JZFunctionPointer>(const QVariant &v, std::false_type)
{
    Q_ASSERT(v.userType() == qMetaTypeId<JZFunctionPointer>());
    return v.value<JZFunctionPointer>();
}

// 函数返回
template<>
JZCORE_EXPORT QVariant toVariant(QVariant v)
{
    Q_ASSERT(v.userType() != qMetaTypeId<JZNodeVariantAny>());
    JZNodeVariantAny any;
    any.value = v;
    return QVariant::fromValue(any);
}

template<>
JZCORE_EXPORT QVariant toVariant(JZNodeVariantAny value)
{
    return QVariant::fromValue(value);
}

template<>
JZCORE_EXPORT QVariant toVariant(JZFunctionPointer ptr)
{
    return QVariant::fromValue(ptr);
}

template<>
JZCORE_EXPORT QVariant toVariant(QString value)
{
    return value;
}

template<>
JZCORE_EXPORT void getReturn(const QVariantList &)
{

}

}