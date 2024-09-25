#include <QVariant>
#include <QMap>
#include <QDebug>
#include "JZNodeType.h"
#include "JZNodeObject.h"
#include "JZNodeIR.h"
#include "JZRegExpHelp.h"

static QMap<QString,int> typeMap;
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
    typeMap["QObject"] = Type_object;
    typeMap["QWidget"] = Type_widget;
    
    typeMap["auto"] = Type_arg;
    typeMap["arg"] = Type_arg;
    typeMap["args"] = Type_args;
    typeMap["paramName"] = Type_paramName;

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
    opNameMap[OP_and] = "&&";
    opNameMap[OP_or] = "||";
    opNameMap[OP_not] = "!";
    opNameMap[OP_bitor] = "|";
    opNameMap[OP_bitand] = "&";
    opNameMap[OP_bitxor] = "~";
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
    return (type > Type_enum && type < Type_class);
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

bool JZNodeType::isBase(int type)
{
    return (type >= Type_none) && (type <= Type_string);
}

bool JZNodeType::isObject(int type)
{
    return (type == Type_nullptr || type >= Type_class);
}

bool JZNodeType::isLiteralType(int type)
{
    if (isBaseOrEnum(type) || type == Type_nullptr || type == Type_function)
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
            if (type1 > type2)
                std::swap(type1, type2);
            return type2;
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

QString JZNodeType::debugString(const JZNodeObject *obj)
{
    if(obj->type() == Type_point)
    {
        QPoint *pt = (QPoint *)obj->cobj();
        return "{" + QString::number(pt->x()) + "," + QString::number(pt->y()) + "}";
    }
    else
    {
        return QString::asprintf("%p",obj);
    }
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

QString JZNodeType::typeName(int type)
{
    return typeMap.key(type,"none");
}

int JZNodeType::nameToType(const QString &name)
{
    return typeMap.value(name, Type_none);
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

bool JZNodeType::sigSlotTypeMatch(const JZSignalDefine *sig,const JZFunctionDefine *slot)
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