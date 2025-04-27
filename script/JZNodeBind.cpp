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

JZScriptEnvironment *runtimeEnvironment()
{
    return g_engine->environment();    
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
// 函数调用
template<>
QString* fromVariant<QString*>(const QVariant &v, std::true_type)
{
    if (v.type() != QVariant::String)
        return nullptr;
    return (QString*)v.data();
}

template<>
bool fromVariant<bool>(const QVariant &v, std::false_type)
{
    return v.toBool();
}

template<>
int fromVariant<int>(const QVariant &v, std::false_type)
{
    return v.toInt();
}

template<>
qint64 fromVariant<qint64>(const QVariant &v, std::false_type)
{
    return v.value<qint64>();
}

template<>
double fromVariant<double>(const QVariant &v, std::false_type)
{
    return v.toDouble();
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
    Q_ASSERT(v.userType() == qMetaTypeId<JZVariantAny>());
    auto ptr = (JZVariantAny*)v.data();
    return ptr->variant;    
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

template<>
void getReturn(const QVariantList &)
{

}

}