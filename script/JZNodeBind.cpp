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
int8_t fromVariant<int8_t>(const QVariant &v, std::false_type)
{
	return v.value<uint8_t>();
}

template<>
uint8_t fromVariant<uint8_t>(const QVariant &v, std::false_type)
{
	return v.value<uint8_t>();
}

template<>
int16_t fromVariant<int16_t>(const QVariant &v, std::false_type)
{
    return v.value<int16_t>();
}

template<>
uint16_t fromVariant<uint16_t>(const QVariant &v, std::false_type)
{
	return v.value<uint16_t>();
}

template<>
int fromVariant<int>(const QVariant &v, std::false_type)
{
    return v.toInt();
}

template<>
uint fromVariant<uint>(const QVariant &v, std::false_type)
{
    return v.toUInt();
}

template<>
int64_t fromVariant<int64_t>(const QVariant &v, std::false_type)
{
    return v.value<int64_t>();
}

template<>
uint64_t fromVariant<uint64_t>(const QVariant &v, std::false_type)
{
    return v.value<uint64_t>();
}

template<>
float fromVariant<float>(const QVariant &v, std::false_type)
{
    return v.value<float>();
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
const QString& fromVariant<const QString&>(const QVariant& v, std::false_type)
{
    Q_ASSERT(v.type() == QVariant::String);
    QString *pstr = (QString*)v.data();
    return *pstr;
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

}