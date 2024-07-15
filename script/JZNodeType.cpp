#include <QVariant>
#include <QMap>
#include <QDebug>
#include "JZNodeType.h"
#include "JZNodeObject.h"
#include "JZNodeIR.h"
#include "JZRegExpHelp.h"

static QMap<QString,int> typeMap;
static QMap<int64_t,ConvertFunc> convertMap;
static QMap<int, QString> opNameMap;

//JZEnum
JZEnum::JZEnum()
{
    type = Type_none;
    value = 0;
}

QDataStream &operator<<(QDataStream &s, const JZEnum &param)
{
    s << param.type << param.value;    
    return s;
}

QDataStream &operator>>(QDataStream &s, JZEnum &param)
{
    s >> param.type >> param.value;
    return s;
}

//JZFunctionPointer
bool JZFunctionPointer::operator==(const JZFunctionPointer &other)
{
    return this->functionName == other.functionName;
}

QDataStream &operator<<(QDataStream &s, const JZFunctionPointer &param)
{
    s << param.functionName;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZFunctionPointer &param)
{
    s >> param.functionName;
    return s;
}

//JZNodeVariantAny
int JZNodeVariantAny::type()
{
    return JZNodeType::variantType(value);
}

//JZNodeType
void JZNodeType::init()
{
    typeMap["none"]   = Type_none;
    typeMap["any"]    = Type_any;
    typeMap["bool"]   = Type_bool;
    typeMap["int"]    = Type_int;
    typeMap["int64"]  = Type_int64;
    typeMap["double"] = Type_double;
    typeMap["string"] = Type_string;
    typeMap["null"] = Type_nullptr;
    typeMap["function"] = Type_function;     
    typeMap["args"] = Type_args;

    opNameMap[OP_add] = "+";
    opNameMap[OP_sub] = "-";
    opNameMap[OP_mul] = "*";
    opNameMap[OP_div] = "/";
    opNameMap[OP_mod] = "%";
    opNameMap[OP_eq] = "==";
    opNameMap[OP_ne] = "!=";
    opNameMap[OP_le] = "<=";
    opNameMap[OP_ge] = ">=";
    opNameMap[OP_lt] = "<";
    opNameMap[OP_gt] = ">";
    opNameMap[OP_and] = "and";
    opNameMap[OP_or] = "or";
    opNameMap[OP_not] = "not";
    opNameMap[OP_bitor] = "|";
    opNameMap[OP_bitand] = "&";
    opNameMap[OP_bitxor] = "~";
}

int64_t makeConvertId(int from, int to)
{
    int64_t id = (int64_t)from << 32 | (int64_t)to;
    return id;
}

QString JZNodeType::typeToName(int id)
{            
    if(isEnum(id))
        return JZNodeObjectManager::instance()->getEnumName(id);
    else if (id >= Type_class)
        return JZNodeObjectManager::instance()->getClassName(id);
    else
        return typeMap.key(id, "unknown type " + QString::number(id));
}

int JZNodeType::nameToType(const QString &name)
{
    if(typeMap.contains(name))
        return typeMap[name];

    return JZNodeObjectManager::instance()->getId(name);
}

int JZNodeType::typeidToType(const QString &name)
{
    if(name == typeid(QVariant).name())
        return Type_any;
    else if(name == typeid(bool).name())
        return Type_bool;
    else if(name == typeid(int).name())
        return Type_int;
    else if(name == typeid(int64_t).name())
        return Type_int64;
    else if(name == typeid(double).name())
        return Type_double;
    else if(name == typeid(QString).name())
        return Type_string;
    else    
        return JZNodeObjectManager::instance()->getIdByCType(name);
}

int JZNodeType::variantType(const QVariant &v)
{
    int v_type = v.type(); 
    if(v_type == QVariant::Bool)
        return Type_bool;
    else if(v_type == QVariant::Int)
        return Type_int;
    else if(v_type == QVariant::LongLong)
        return Type_int64;
    else if(v_type == QVariant::Double)
        return Type_double;
    else if(v_type == QVariant::String)
        return Type_string;
    else if(v_type == QVariant::UserType)
    {
        int v_usertype = v.userType(); 
        if(v_usertype == qMetaTypeId<JZEnum>())
            return ((JZEnum*)v.data())->type;
        else if(v_usertype == qMetaTypeId<JZObjectNull>())
            return Type_nullptr;
        else if(v_usertype == qMetaTypeId<JZFunctionPointer>())
            return Type_function;
        else if (v_usertype == qMetaTypeId<JZNodeObjectPtr>())
            return ((JZNodeObjectPtr*)v.data())->object()->type();
        else if (v_usertype == qMetaTypeId<JZNodeVariantAny>())
            return Type_any;
    }
    
    return Type_none;
}

QString JZNodeType::variantTypeName(const QVariant &v)
{
    return typeToName(variantType(v));
}

bool JZNodeType::isBool(int type)
{
    return (type == Type_bool);
}

bool JZNodeType::isNumber(int type)
{
    if(type == Type_int || type == Type_int64 || type == Type_double || type == Type_bool)
        return true;
    
    return false;
}

bool JZNodeType::isEnum(int type)
{
    return (JZNodeObjectManager::instance()->enumMeta(type) != nullptr);
}

bool JZNodeType::isBaseOrEnum(int type)
{
    return isBase(type) || isEnum(type);
}

bool JZNodeType::isNullObject(const QVariant &v)
{
    return toJZObject(v)->isNull();
}

bool JZNodeType::isNullptr(const QVariant &v)
{
    return (v.userType() == qMetaTypeId<JZObjectNull>());
}

bool JZNodeType::isWidget(const QVariant &v)
{
    auto type = variantType(v);
    return isInherits(type, Type_widget);
}

bool JZNodeType::isBase(int type)
{
    return (type >= Type_none) && (type <= Type_string);
}

bool JZNodeType::isObject(int type)
{
    return (type == Type_nullptr || type >= Type_class);
}

int JZNodeType::isInherits(const QString &type1,const QString &type2)
{
    int t1 = JZNodeType::nameToType(type1);
    int t2 = JZNodeType::nameToType(type2);
    return JZNodeObjectManager::instance()->isInherits(t1, t2);
}

bool JZNodeType::isSameType(const QVariant &v1,const QVariant &v2)
{
    int type1 = JZNodeType::variantType(v1);
    int type2 = JZNodeType::variantType(v2);
    return isSameType(type1,type2);
}

bool JZNodeType::isSameType(int type1,int type2)
{
    if(type1 == type2)
        return true;
    else if((isEnum(type1) && type2 == Type_int) || (type1 == Type_int && isEnum(type2))) 
        return true;
    else if(type1 >= Type_class && type2 >= Type_class)
        return isInherits(type1,type2);

    qDebug() << JZNodeType::typeToName(type1) << JZNodeType::typeToName(type2);
    return false;
}

bool JZNodeType::isLiteralType(int type)
{
    if(isBaseOrEnum(type) || type == Type_nullptr || type == Type_function)
        return true;
    
    return false;
}

bool JZNodeType::isPointer(const QVariant &v)
{
    return v.userType() == qMetaTypeId<QVariantPointer>();
}

QVariant *JZNodeType::getPointer(const QVariant &v)
{
    Q_ASSERT(isPointer(v));

    QVariantPointer *ptr = (QVariantPointer *)v.data();
    return ptr->value;
}

int JZNodeType::isInherits(int type1,int type2)
{
    return JZNodeObjectManager::instance()->isInherits(type1,type2);
}

int JZNodeType::calcExprType(int type1,int type2,int op)
{   
    switch (op)
    {
        case OP_add:
        case OP_sub:
        case OP_mul:
        case OP_div:
        case OP_mod:
        {
            return upType(type1,type2);
        }
        case OP_eq:
        case OP_ne:
        case OP_le:
        case OP_ge:
        case OP_lt:
        case OP_gt:
        case OP_and:
        case OP_or:
            return Type_bool;
        case OP_bitand:
        case OP_bitor:
        case OP_bitxor:
            return Type_int;
        default:
            Q_ASSERT(0);
            return Type_none;
    }    
}

bool JZNodeType::canConvert(int type1,int type2)
{
    if(type1 == type2 || type2 == Type_any)
        return true;
    if(isNumber(type1) && isNumber(type2))
        return true;
    if ((type1 == Type_int && isEnum(type2)) || (isEnum(type1) && type2 == Type_int))
        return true;
    if (isEnum(type1) && isEnum(type2))
    {
        auto meta1 = JZNodeObjectManager::instance()->enumMeta(type1);
        auto meta2 = JZNodeObjectManager::instance()->enumMeta(type2);
        if ((meta1->isFlag() && meta1->flagEnum() == type2)
            || (meta2->isFlag() && meta2->flagEnum() == type1))
            return true;
        
        return false;
    }
    if (type1 == Type_nullptr && type2 >= Type_class)
        return true;
    if(type1 >= Type_class && type2 >= Type_class)
    {
        auto inst = JZNodeObjectManager::instance();        
        return inst->isInherits(type1,type2);
    }
    int64_t id = makeConvertId(type1, type2);
    if (convertMap.contains(id))
        return true;

    return false;
}

QString JZNodeType::debugString(const JZNodeObject *obj)
{
    QString text = obj->className();

    return text;
}

QString JZNodeType::debugString(const QVariant &v)
{
    int v_type = variantType(v);
    if (isJZObject(v))
    {
        auto obj = toJZObject(v);
        if (obj)
        {
            return debugString(obj);
        }
        else
            return "null";
    }
    else if (v_type == Type_any)
    {
        JZNodeVariantAny *ptr = (JZNodeVariantAny*)v.data();
        return "Variant{" + debugString(ptr->value) + "}";
    }
    else if (v_type == Type_function)
    {
        JZFunctionPointer *ptr = (JZFunctionPointer*)v.data();
        return ptr->functionName;
    }
    else
    {
        return v.toString();
    }    
}

QString JZNodeType::opName(int op)
{   
    Q_ASSERT(opNameMap.contains(op));
    return opNameMap[op];
}

int JZNodeType::opType(const QString &name)
{
    int ret = opNameMap.key(name,OP_none);
    Q_ASSERT(ret != OP_none);
    return ret;
}

int JZNodeType::opPri(const QString &op)
{
    if( op == "(")
        return 100;

    if( op == "*" || op == "/" || op == "%" )
		return -1;

	if( op == "+" || op == "-" )
		return -2;

	if( op == "<<" ||
		op == ">>" )
		return -3;

	if( op == "&" )
		return -4;

	if( op == "^" )
		return -5;

	if( op == "|" )
		return -6;

	if( op == "<=" ||
		op == "<" ||
		op == ">=" ||
		op == ">" )
		return -7;

	if( op == "==" || op == "!=")
		return -8;

	if( op == "&&" )
		return -9;

	if( op == "||" )
		return -10;

    qDebug() << op;
    Q_ASSERT(0);
    return -100;
}

bool JZNodeType::isDoubleOp(const QString &op)
{
    return false;
}

QVariant JZNodeType::convertTo(int dst_type,const QVariant &v)
{
    int src_type = variantType(v);
    if (src_type == dst_type)
        return v;

    if (dst_type == Type_any)
    {
        JZNodeVariantAny any;
        any.value = v;
        return QVariant::fromValue(any);
    }   
    else if(src_type == Type_any)
    {
        auto *ptr = (const JZNodeVariantAny*)v.data();
        return convertTo(dst_type,ptr->value);
    }
    else if (src_type == Type_nullptr && dst_type >= Type_class)
    {        
        auto null_obj = JZNodeObjectManager::instance()->createNull(dst_type);
        return QVariant::fromValue(JZNodeObjectPtr(null_obj,true));
    }
    else if(src_type >= Type_class && dst_type >= Type_class)
    {
        if(JZNodeObjectManager::instance()->isInherits(src_type,dst_type))
            return v;
    }
    else if(isNumber(src_type) || isNumber(dst_type))
    {
        if(src_type == Type_bool)
        {
            bool b = v.toBool();
            if(dst_type == Type_int)
                return (int)b;
            else if(dst_type == Type_int64)
                return (qint64)b;
            else 
                return (double)b;
        }
        else if(src_type == Type_int)
        {
            int i = v.toInt();
            if(dst_type == Type_bool)
                return (bool)i;
            else if(dst_type == Type_int64)
                return (qint64)i;
            else 
                return (double)i;
        }
        else if(src_type == Type_int64)
        {
            qint64 i = (qint64)v.toLongLong();
            if(dst_type == Type_bool)
                return (int)i;
            else if(dst_type == Type_int)
                return (int)i;
            else 
                return (double)i;
        }
        else
        {
            double d = v.toDouble();
            if(dst_type == Type_bool)
                return (bool)d;
            else if(dst_type == Type_int)
                return (int)d;
            else 
                return (qint64)d;
        }
    }

    int64_t cvt_id = makeConvertId(src_type, dst_type);
    auto it = convertMap.find(cvt_id);
    if (it != convertMap.end())
        return it.value()(v);

    Q_ASSERT_X(0,"Error",qUtf8Printable(typeToName(src_type) + " -> " + typeToName(dst_type)));
    return QVariant();
}

QVariant JZNodeType::clone(const QVariant &v)
{    
    int v_type = JZNodeType::variantType(v);
    if (v_type == Type_any)
    {
        JZNodeVariantAny *ptr = (JZNodeVariantAny*)v.data();
        JZNodeVariantAny ret;
        ret.value = clone(ptr->value);
        return QVariant::fromValue(ret);
    }
    else if (JZNodeType::isBaseOrEnum(v_type))
        return v;
    else if(v_type > Type_class)
    {
        auto obj = toJZObject(v);
        auto new_obj = JZNodeObjectManager::instance()->clone(obj);
        return QVariant::fromValue(JZNodeObjectPtr(new_obj,true));
    }
    else
    {
        Q_ASSERT(0);
        return QVariant();
    }
}

int JZNodeType::upType(int type1, int type2)
{
    if (type1 > type2)
        std::swap(type1, type2);
    
    if (type1 == type2)
        return type1;
        
    if (type1 >= Type_bool && type1 <= Type_double
        && type2 >= Type_bool && type2 <= Type_double)
        return type2;

    if(type1 == Type_int && isEnum(type2)) 
        return Type_int;

    if(type1 == Type_nullptr && type2 >= Type_class)
        return type2;

    if(type1 >= Type_class && type2 >= Type_class)
    {
        if(isInherits(type2,type1))
            return type1;
    }

    return Type_none;
}

int JZNodeType::upType(QList<int> types)
{
    if (types.size() == 0)
        return Type_none;

    int type = types[0];
    for (int i = 1; i < types.size(); i++)
        type = upType(type, types[i]);
    
    return type;
}

int JZNodeType::matchType(QList<int> src_types,QList<int> dst_types)
{   
    QList<int> dst_allow_type;    
    //在dst中选择能被所有src转换到的类型
    for (int i = 0; i < dst_types.size(); i++)
    {
        bool can_convert = true;
        for (int j = 0; j < src_types.size(); j++)
        {
            if (!canConvert(src_types[j], dst_types[i]))
            {
                can_convert = false;
                break;
            }                
        }
        if (can_convert)
            dst_allow_type << dst_types[i];
    }
    if (dst_allow_type.size() == 0)
        return Type_none;
    if (dst_allow_type.size() == 1)
        return dst_allow_type[0];    

    QList<int> dst_near_type;
    //对scr 选择dst_allow_type 里面最近似的类型
    for (int i = 0; i < src_types.size(); i++)
    {
        int near_type = Type_none;
        int near = INT_MAX;
        for (int j = 0; j < dst_allow_type.size(); j++)
        {
            if (src_types[i] == dst_allow_type[j])
            {
                near_type = dst_allow_type[j];
                break;
            }
            else
            {
                int cur_near = 0;
                if (isBaseOrEnum(src_types[i]))
                {
                    cur_near = abs(src_types[i] - dst_allow_type[j]);
                }
                else if (isObject(src_types[i]))
                {

                }

                if (cur_near < near) 
                {
                    near_type = dst_allow_type[j];
                    near = cur_near;
                }
            }
        }
        dst_near_type << near_type;
    }
    return upType(dst_near_type);
}

QVariant JZNodeType::initValue(int type, const QString &text)
{
    if (text.isEmpty())
        return defaultValue(type);

    if (type == Type_any)
    {
        JZNodeVariantAny any;
        any.value = initValue(stringType(text),text);
        return QVariant::fromValue(any);
    }
    else if (type == Type_string)
    {
        if(text.front() == '"' && text.back() == '"')
            return text.mid(1, text.size() - 2);
    }
    else if (type == Type_bool)
    {
        if (text == "false")
            return false;
        else if (text == "true")
            return true;
    }
    else if(type == Type_nullptr)
    {
        return QVariant::fromValue(JZObjectNull());
    }
    else if(type == Type_function)
    {   
        JZFunctionPointer func;
        func.functionName = text;
        return QVariant::fromValue(JZFunctionPointer());
    }
    else if(type >= Type_enum && type < Type_class)
    {
        auto enum_meta = JZNodeObjectManager::instance()->enumMeta(type);
        if(enum_meta->hasKey(text))
            return enum_meta->keyToValue(text);
    }
    else if(type >= Type_class)
    {
        if(text == "null")
        {
            auto obj = JZNodeObjectManager::instance()->createNull(type);
            return QVariant::fromValue(JZNodeObjectPtr(obj,true));        
        }
    }
    else if (type == Type_int || type == Type_int64 || type == Type_double)
    {        
        bool isInt = JZRegExpHelp::isInt(text);
        bool isHex = JZRegExpHelp::isHex(text);
        bool isFloat = JZRegExpHelp::isFloat(text);
        
        if (isHex)
            return text.toInt(nullptr, 16);
        else
        {
            if (type == Type_int || type == Type_int64)
            {
                if (isFloat)
                    return (int)text.toDouble();
                else if (isInt)
                    return text.toInt();
            }
            else if (type == Type_double)
            {
                if (isFloat || isInt)
                    return text.toDouble();
            }
            else
            {
                if (isInt)
                    return text.toInt();
                if (isFloat)
                    return text.toDouble();
            }
        }
    }

    return QVariant();
}

int JZNodeType::stringType(const QString &text)
{
    if (text == "false" || text == "true")
        return Type_bool;
    else if (text == "null")
        return Type_nullptr;
    else if (text.size() >= 2 && text.front() == '"' && text.back() == '"')
        return Type_string;

    bool isInt = JZRegExpHelp::isInt(text);
    bool isHex = JZRegExpHelp::isHex(text);
    bool isFloat = JZRegExpHelp::isFloat(text);
    if (isInt || isHex)
        return Type_int;
    else if(isFloat)
        return Type_double;

    return Type_none;
}

QVariant JZNodeType::defaultValue(int type)
{
    if(type == Type_any)
        return QVariant::fromValue(JZNodeVariantAny());
    else if(type == Type_bool)
        return false;
    else if(type == Type_int)
        return 0;
    else if(type == Type_int64)
        return (qint64)0;
    else if(type == Type_double)
        return (double)0.0;
    else if(type == Type_string)
        return QString();
    else if(type == Type_function)
        return QVariant::fromValue(JZFunctionPointer());
    else if(type == Type_nullptr)
        return QVariant::fromValue(JZObjectNull());
    else if(type >= Type_enum && type < Type_class)
    {
        auto e = JZNodeObjectManager::instance()->createEnum(type);
        return QVariant::fromValue(e);
    }
    else if(type >= Type_class)
    {
        JZNodeObject *obj = JZNodeObjectManager::instance()->createNull(type);
        return QVariant::fromValue(JZNodeObjectPtr(obj,true));
    }
    else
    {        
        return QVariant();
    }
}

void JZNodeType::registConvert(int from, int to, ConvertFunc func)
{
    int id = (int64_t)from << 32 | (int64_t)to;
    convertMap[id] = func;
}

bool JZNodeType::sigSlotTypeMatch(const JZSingleDefine *sig,const JZFunctionDefine *slot)
{
    int slot_param = slot->paramIn.size() - 1; 
    if(sig->paramOut.size() < slot_param)
        return false;

    for(int i = 0; i < slot_param; i++)
    {
        int slot_idx = i + 1;
        if(sig->paramOut[i].type != slot->paramIn[slot_idx].type)
            return false;
    }
    return true;
}

bool JZNodeType::functionTypeMatch(const JZFunctionDefine *func1,const JZFunctionDefine *func2)
{
    if(func1->paramIn.size() != func2->paramIn.size() 
        || func1->paramOut.size() != func2->paramOut.size())
        return false;
    if(func1->isVirtualFunction != func2->isVirtualFunction)
        return false;
    if(func1->isFlowFunction != func2->isFlowFunction)
        return false;

    for(int i = 0; i < func1->paramIn.size(); i++)
    {
        if(func1->paramIn[i].type != func2->paramIn[i].type)
            return false;
    }
    for(int i = 0; i < func1->paramOut.size(); i++)
    {
        if(func1->paramOut[i].type != func2->paramOut[i].type)
            return false;
    }
    return true;
}