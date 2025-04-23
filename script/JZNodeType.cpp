#include <QVariant>
#include <QMap>
#include <QDebug>
#include "JZNodeType.h"
#include "JZNodeObject.h"
#include "JZNodeIR.h"
#include "JZRegExpHelp.h"

static QMap<QString,int> typeMap;
static QMap<int, QString> opNameMap;

//QVariantPtr
QVariantPtr::QVariantPtr()
{
    type = Type_none;
    ptr = QSharedPointer<QVariant>(new QVariant());
    cparam = nullptr;
}

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
JZNodeVariantAny::JZNodeVariantAny()
{
}

bool JZNodeVariantAny::operator==(const JZNodeVariantAny& other) const
{
    return value == other.value;
}

int JZNodeVariantAny::type()
{
    return JZNodeType::variantType(value);
}

//JZObjectNull
JZNodeObjectNull::JZNodeObjectNull()
{
}

QDataStream &operator<<(QDataStream &s, const JZNodeObjectNull&param)
{
    return s;
}
QDataStream &operator>>(QDataStream &s, JZNodeObjectNull&param)
{
    return s;
}

//JZNodeType
void JZNodeType::init()
{
    typeMap["none"]   = Type_none;
    typeMap["any"]    = Type_any;
    typeMap["bool"]   = Type_bool;
    typeMap["int8"]   = Type_int8;
    typeMap["int16"]  = Type_int16;
    typeMap["int"]    = Type_int;
    typeMap["int64"]  = Type_int64;
    typeMap["uint8"] = Type_uint8;
    typeMap["uint16"] = Type_uint16;
    typeMap["uint"] = Type_uint;
    typeMap["uint64"] = Type_uint64;
    typeMap["float"] = Type_float;
    typeMap["double"] = Type_double;
    typeMap["string"] = Type_string;
    typeMap["null"] = Type_nullptr;
    typeMap["function"] = Type_function;
    typeMap["QObject"] = Type_object;
    typeMap["QWidget"] = Type_widget;
    
    typeMap["arg"] = Type_arg;
    typeMap["argPointer"] = Type_argPointer;
    typeMap["args"] = Type_args; 
    typeMap["auto"] = Type_auto;

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
    opNameMap[OP_neg] = "-";
    opNameMap[OP_bitor] = "|";
    opNameMap[OP_bitand] = "&";
    opNameMap[OP_bitxor] = "^";
    opNameMap[OP_bitreverse] = "~";
}

bool JZNodeType::isBool(int type)
{
    return (type == Type_bool);
}

bool JZNodeType::isNumber(int type)
{
    if(type == Type_int8 || type == Type_int16 || type == Type_int || type == Type_int64 
        || type == Type_uint8 || type == Type_uint16 || type == Type_uint || type == Type_uint64
        || type == Type_float  || type == Type_double)
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
    auto obj = toJZObject(v);
    return (obj == nullptr || (obj->isCObject() && !obj->cobj()));
}

bool JZNodeType::isNullptr(const QVariant &v)
{
    return (v.userType() == qMetaTypeId<JZNodeObjectNull>());
}

bool JZNodeType::isBase(int type)
{
    return (type >= Type_none) && (type <= Type_nullptr);
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

int JZNodeType::baseType(int type)
{
    return type & (~Type_pointerFlag);
}

int JZNodeType::pointerType(int type)
{
    Q_ASSERT(type != Type_none && !isPointer(type));
    return type | Type_pointerFlag;
}

bool JZNodeType::isPointer(int type)
{
    return type & Type_pointerFlag;
}

QString JZNodeType::baseType(const QString& type)
{
    if (type.endsWith("*"))
        return type.left(type.size() - 1);
    else
        return type;
}

QString JZNodeType::pointerType(const QString& type)
{
    Q_ASSERT(!isPointer(type));
    return type + "*";
}

bool JZNodeType::isPointer(const QString& type)
{
    return type.endsWith("*");
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
    else if (v_type == qMetaTypeId<int8_t>())
        return Type_int8;
    else if (v_type == qMetaTypeId<int16_t>())
        return Type_int16;
    else if(v_type == QVariant::Int)
        return Type_int;
    else if (v_type == QVariant::LongLong)
        return Type_int64;
    else if (v_type == qMetaTypeId<uint8_t>())
        return Type_uint8;
    else if (v_type == qMetaTypeId<uint16_t>())
        return Type_uint16;
    else if (v_type == QVariant::Int)
        return Type_uint;
    else if (v_type == QVariant::LongLong)
        return Type_uint64;
    else if (v_type == qMetaTypeId<float>())
        return Type_float;
    else if(v_type == QVariant::Double)
        return Type_double;
    else if(v_type == QVariant::String)
        return Type_string;
    else if(v_type == QVariant::UserType)
    {
        int v_usertype = v.userType(); 
        if (v_usertype == qMetaTypeId<JZEnum>())
            return ((JZEnum*)v.data())->type;
        else if (v_usertype == qMetaTypeId<JZNodeObjectNull>())
            return Type_nullptr;
        else if (v_usertype == qMetaTypeId<JZFunctionPointer>())
            return Type_function;
        else if (v_usertype == qMetaTypeId<JZNodeObjectHolder>())
            return ((JZNodeObjectHolder*)v.data())->object()->type();
        else if (v_usertype == qMetaTypeId<JZNodeVariantAny>())
            return Type_any;
        else if (v_usertype == qMetaTypeId<JZNodeObjectPointer>())
            return ((JZNodeObjectPointer*)v.data())->type;
    }
    
    return Type_none;
}

bool JZNodeType::variantIsPointer(const QVariant& v)
{
    return v.userType() == qMetaTypeId<JZNodeObjectPointer>();
}

bool JZNodeType::isContainerType(QString value)
{
    return value.startsWith("QList<") || value.startsWith("QMap<") || value.startsWith("QSet<");
}

QString JZNodeType::listType(QString value)
{
    return "QList<" + value + ">";
}

QString JZNodeType::listIteratorType(QString list_type)
{
    return list_type + "::iterator";
}

bool JZNodeType::listValueType(QString list_type, QString& value_type)
{
    if (!list_type.startsWith("QList<") || !list_type.endsWith(">"))
        return false;

    value_type = list_type.mid(6, list_type.size() - 7);
    return true;
}

QString JZNodeType::mapType(QString key, QString value)
{
    return "QMap<" + key + "," + value + ">";
}

QString JZNodeType::mapIteratorType(QString map_type)
{
    return map_type + "::iterator";
}

bool JZNodeType::mapValueType(QString map_type, QString& value_type, QString& key_type)
{
    if (!map_type.startsWith("QMap<") || !map_type.endsWith(">"))
        return false;

    map_type = map_type.mid(6, map_type.size() - 7);
    QStringList str = map_type.split(",");
    if (str.size() != 2)
        return false;

    value_type = str[0];
    key_type = str[1];
    return true;
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


QVariant JZNodeType::convertToPointer(const QVariant& srcValue)
{
    if (srcValue.type() == QVariant::UserType && srcValue.userType() == qMetaTypeId<JZNodeObjectHolder>())
    {
        JZNodeObjectHolder* obj_ptr = (JZNodeObjectHolder*)srcValue.data();
        JZNodeObjectPointer pointer = obj_ptr->toPointer();
        return QVariant::fromValue(pointer);
    }

    Q_ASSERT(0);
    return QVariant();
}

QVariant JZNodeType::convertNumber(const QVariant& srcValue, int dstType)
{
    int srcType = variantType(srcValue);
    if (srcType == Type_int8) {
        if (dstType == Type_int16) {
            return QVariant::fromValue(srcValue.value<int16_t>());
        }
        else if (dstType == Type_int) {
            return QVariant::fromValue(srcValue.value<int>());
        }
        else if (dstType == Type_int64) {
            return QVariant::fromValue(srcValue.value<int64_t>());
        }
        else if (dstType == Type_uint8) {
            return QVariant::fromValue(srcValue.value<uint8_t>());
        }
        else if (dstType == Type_uint16) {
            return QVariant::fromValue(srcValue.value<uint16_t>());
        }
        else if (dstType == Type_uint) {
            return QVariant::fromValue(srcValue.value<unsigned int>());
        }
        else if (dstType == Type_uint64) {
            return QVariant::fromValue(srcValue.value<uint64_t>());
        }
        else if (dstType == Type_float) {
            return QVariant::fromValue(srcValue.value<float>());
        }
        else if (dstType == Type_double) {
            return QVariant::fromValue(srcValue.value<double>());
        }
    }
    else if (srcType == Type_int16) {
        if (dstType == Type_int8) {
            return QVariant::fromValue(srcValue.value<int8_t>());
        }
        else if (dstType == Type_int) {
            return QVariant::fromValue(srcValue.value<int>());
        }
        else if (dstType == Type_int64) {
            return QVariant::fromValue(srcValue.value<int64_t>());
        }
        else if (dstType == Type_uint8) {
            return QVariant::fromValue(srcValue.value<uint8_t>());
        }
        else if (dstType == Type_uint16) {
            return QVariant::fromValue(srcValue.value<uint16_t>());
        }
        else if (dstType == Type_uint) {
            return QVariant::fromValue(srcValue.value<unsigned int>());
        }
        else if (dstType == Type_uint64) {
            return QVariant::fromValue(srcValue.value<uint64_t>());
        }
        else if (dstType == Type_float) {
            return QVariant::fromValue(srcValue.value<float>());
        }
        else if (dstType == Type_double) {
            return QVariant::fromValue(srcValue.value<double>());
        }
    }
    else if (srcType == Type_int) {
        if (dstType == Type_int8) {
            return QVariant::fromValue(srcValue.value<int8_t>());
        }
        else if (dstType == Type_int16) {
            return QVariant::fromValue(srcValue.value<int16_t>());
        }
        else if (dstType == Type_int64) {
            return QVariant::fromValue(srcValue.value<int64_t>());
        }
        else if (dstType == Type_uint8) {
            return QVariant::fromValue(srcValue.value<uint8_t>());
        }
        else if (dstType == Type_uint16) {
            return QVariant::fromValue(srcValue.value<uint16_t>());
        }
        else if (dstType == Type_uint) {
            return QVariant::fromValue(srcValue.value<unsigned int>());
        }
        else if (dstType == Type_uint64) {
            return QVariant::fromValue(srcValue.value<uint64_t>());
        }
        else if (dstType == Type_float) {
            return QVariant::fromValue(srcValue.value<float>());
        }
        else if (dstType == Type_double) {
            return QVariant::fromValue(srcValue.value<double>());
        }
    }
    else if (srcType == Type_int64) {
        if (dstType == Type_int8) {
            return QVariant::fromValue(srcValue.value<int8_t>());
        }
        else if (dstType == Type_int16) {
            return QVariant::fromValue(srcValue.value<int16_t>());
        }
        else if (dstType == Type_int) {
            return QVariant::fromValue(srcValue.value<int>());
        }
        else if (dstType == Type_uint8) {
            return QVariant::fromValue(srcValue.value<uint8_t>());
        }
        else if (dstType == Type_uint16) {
            return QVariant::fromValue(srcValue.value<uint16_t>());
        }
        else if (dstType == Type_uint) {
            return QVariant::fromValue(srcValue.value<unsigned int>());
        }
        else if (dstType == Type_uint64) {
            return QVariant::fromValue(srcValue.value<uint64_t>());
        }
        else if (dstType == Type_float) {
            return QVariant::fromValue(srcValue.value<float>());
        }
        else if (dstType == Type_double) {
            return QVariant::fromValue(srcValue.value<double>());
        }
    }
    else if (srcType == Type_uint8) {
        if (dstType == Type_int8) {
            return QVariant::fromValue(srcValue.value<int8_t>());
        }
        else if (dstType == Type_int16) {
            return QVariant::fromValue(srcValue.value<int16_t>());
        }
        else if (dstType == Type_int) {
            return QVariant::fromValue(srcValue.value<int>());
        }
        else if (dstType == Type_int64) {
            return QVariant::fromValue(srcValue.value<int64_t>());
        }
        else if (dstType == Type_uint16) {
            return QVariant::fromValue(srcValue.value<uint16_t>());
        }
        else if (dstType == Type_uint) {
            return QVariant::fromValue(srcValue.value<unsigned int>());
        }
        else if (dstType == Type_uint64) {
            return QVariant::fromValue(srcValue.value<uint64_t>());
        }
        else if (dstType == Type_float) {
            return QVariant::fromValue(srcValue.value<float>());
        }
        else if (dstType == Type_double) {
            return QVariant::fromValue(srcValue.value<double>());
        }
    }
    else if (srcType == Type_uint16) {
        if (dstType == Type_int8) {
            return QVariant::fromValue(srcValue.value<int8_t>());
        }
        else if (dstType == Type_int16) {
            return QVariant::fromValue(srcValue.value<int16_t>());
        }
        else if (dstType == Type_int) {
            return QVariant::fromValue(srcValue.value<int>());
        }
        else if (dstType == Type_int64) {
            return QVariant::fromValue(srcValue.value<int64_t>());
        }
        else if (dstType == Type_uint8) {
            return QVariant::fromValue(srcValue.value<uint8_t>());
        }
        else if (dstType == Type_uint) {
            return QVariant::fromValue(srcValue.value<unsigned int>());
        }
        else if (dstType == Type_uint64) {
            return QVariant::fromValue(srcValue.value<uint64_t>());
        }
        else if (dstType == Type_float) {
            return QVariant::fromValue(srcValue.value<float>());
        }
        else if (dstType == Type_double) {
            return QVariant::fromValue(srcValue.value<double>());
        }
    }
    else if (srcType == Type_uint) {
        if (dstType == Type_int8) {
            return QVariant::fromValue(srcValue.value<int8_t>());
        }
        else if (dstType == Type_int16) {
            return QVariant::fromValue(srcValue.value<int16_t>());
        }
        else if (dstType == Type_int) {
            return QVariant::fromValue(srcValue.value<int>());
        }
        else if (dstType == Type_int64) {
            return QVariant::fromValue(srcValue.value<int64_t>());
        }
        else if (dstType == Type_uint8) {
            return QVariant::fromValue(srcValue.value<uint8_t>());
        }
        else if (dstType == Type_uint16) {
            return QVariant::fromValue(srcValue.value<uint16_t>());
        }
        else if (dstType == Type_uint64) {
            return QVariant::fromValue(srcValue.value<uint64_t>());
        }
        else if (dstType == Type_float) {
            return QVariant::fromValue(srcValue.value<float>());
        }
        else if (dstType == Type_double) {
            return QVariant::fromValue(srcValue.value<double>());
        }
    }
    else if (srcType == Type_uint64) {
        if (dstType == Type_int8) {
            return QVariant::fromValue(srcValue.value<int8_t>());
        }
        else if (dstType == Type_int16) {
            return QVariant::fromValue(srcValue.value<int16_t>());
        }
        else if (dstType == Type_int) {
            return QVariant::fromValue(srcValue.value<int>());
        }
        else if (dstType == Type_int64) {
            return QVariant::fromValue(srcValue.value<int64_t>());
        }
        else if (dstType == Type_uint8) {
            return QVariant::fromValue(srcValue.value<uint8_t>());
        }
        else if (dstType == Type_uint16) {
            return QVariant::fromValue(srcValue.value<uint16_t>());
        }
        else if (dstType == Type_uint) {
            return QVariant::fromValue(srcValue.value<unsigned int>());
        }
        else if (dstType == Type_float) {
            return QVariant::fromValue(srcValue.value<float>());
        }
        else if (dstType == Type_double) {
            return QVariant::fromValue(srcValue.value<double>());
        }
    }
    else if (srcType == Type_float) {
        if (dstType == Type_int8) {
            return QVariant::fromValue(srcValue.value<int8_t>());
        }
        else if (dstType == Type_int16) {
            return QVariant::fromValue(srcValue.value<int16_t>());
        }
        else if (dstType == Type_int) {
            return QVariant::fromValue(srcValue.value<int>());
        }
        else if (dstType == Type_int64) {
            return QVariant::fromValue(srcValue.value<int64_t>());
        }
        else if (dstType == Type_uint8) {
            return QVariant::fromValue(srcValue.value<uint8_t>());
        }
        else if (dstType == Type_uint16) {
            return QVariant::fromValue(srcValue.value<uint16_t>());
        }
        else if (dstType == Type_uint) {
            return QVariant::fromValue(srcValue.value<unsigned int>());
        }
        else if (dstType == Type_uint64) {
            return QVariant::fromValue(srcValue.value<uint64_t>());
        }
        else if (dstType == Type_double) {
            return QVariant::fromValue(srcValue.value<double>());
        }
    }
    else if (srcType == Type_double) {
        if (dstType == Type_int8) {
            return QVariant::fromValue(srcValue.value<int8_t>());
        }
        else if (dstType == Type_int16) {
            return QVariant::fromValue(srcValue.value<int16_t>());
        }
        else if (dstType == Type_int) {
            return QVariant::fromValue(srcValue.value<int>());
        }
        else if (dstType == Type_int64) {
            return QVariant::fromValue(srcValue.value<int64_t>());
        }
        else if (dstType == Type_uint8) {
            return QVariant::fromValue(srcValue.value<uint8_t>());
        }
        else if (dstType == Type_uint16) {
            return QVariant::fromValue(srcValue.value<uint16_t>());
        }
        else if (dstType == Type_uint) {
            return QVariant::fromValue(srcValue.value<unsigned int>());
        }
        else if (dstType == Type_uint64) {
            return QVariant::fromValue(srcValue.value<uint64_t>());
        }
        else if (dstType == Type_float) {
            return QVariant::fromValue(srcValue.value<float>());
        }
    }
    Q_ASSERT(0);
    return QVariant();
}