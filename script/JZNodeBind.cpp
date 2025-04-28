#include "JZNodeBind.h"
#include "JZNodeEngine.h"

namespace jzbind
{

static JZScriptEnvironment *g_bindEnv = nullptr;
void setBindEnvironment(JZScriptEnvironment *env)
{    
    Q_ASSERT(env == nullptr || g_bindEnv == nullptr);
    g_bindEnv = env;
}

JZScriptEnvironment *bindEnvironment()
{
    Q_ASSERT(g_bindEnv);
    return g_bindEnv;    
}

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

template<>
JZVariantAny fromVariant<JZVariantAny>(const QVariant &v, std::false_type)
{
    Q_ASSERT(v.userType() == qMetaTypeId<JZVariantAny>());
    return v.value<JZVariantAny>();
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
    Q_ASSERT(v.userType() != qMetaTypeId<JZVariantAny>());
    JZVariantAny any;
    any.variant = v;
    return QVariant::fromValue(any);
}

template<>
QVariant toVariant(JZVariantAny value)
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

}