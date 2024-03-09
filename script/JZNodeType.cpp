﻿#include <QVariant>
#include <QMap>
#include "JZNodeType.h"
#include "JZNodeObject.h"
#include "JZNodeIR.h"
#include "JZRegExpHelp.h"

static QMap<QString,int> typeMap;
static QMap<int64_t,ConvertFunc> convertMap;

void JZNodeType::init()
{
     typeMap["any"]    = Type_any;
     typeMap["bool"]   = Type_bool;
     typeMap["int"]    = Type_int;
     typeMap["int64"]  = Type_int64;
     typeMap["double"] = Type_double;
     typeMap["string"] = Type_string;
     typeMap["null"] = Type_nullptr;
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
    else if (id >= Type_object)
        return JZNodeObjectManager::instance()->getClassName(id);
    else
        return typeMap.key(id, QString());
}

int JZNodeType::nameToType(QString name)
{
    if(typeMap.contains(name))
        return typeMap.value(name,Type_none);

    return JZNodeObjectManager::instance()->getId(name);
}

int JZNodeType::typeidToType(QString name)
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

QVariant::Type JZNodeType::typeToQMeta(int type)
{
    if (type == Type_bool)
        return QVariant::Bool;
    else if (type == Type_int)
        return QVariant::Int;
    else if (type == Type_int64)
        return QVariant::LongLong;
    else if (type == Type_double)
        return QVariant::Double;
    else if (type == Type_string)
        return QVariant::String;

    Q_ASSERT(0);
    return QVariant::Invalid;
}

int JZNodeType::variantType(const QVariant &v)
{
    if(v.type() == QVariant::Bool)
        return Type_bool;
    else if(v.type() == QVariant::Int)
        return Type_int;
    else if(v.type() == QVariant::LongLong)
        return Type_int64;
    else if(v.type() == QVariant::Double)
        return Type_double;
    else if(v.type() == QVariant::String)
        return Type_string;
    else if(v.type() == QVariant::UserType)
    {
        if(isJZObject(v))        
            return JZObjectType(v);          
    }
    return Type_none;
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
    return (type >= Type_none && type <= Type_string || isEnum(type));
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
    return (type >= Type_none && type <= Type_string);
}

bool JZNodeType::isObject(int type)
{
    return (type == Type_nullptr || type >= Type_object);
}

int JZNodeType::isInherits(int type1,int type2)
{
    return JZNodeObjectManager::instance()->isInherits(type1,type2);
}

int JZNodeType::calcExprType(int type1,int type2)
{           
    if (isEnum(type1) || isBool(type1))
        type1 = Type_int;
    if (isEnum(type2) || isBool(type2))
        type2 = Type_int;
    if (type1 == type2)
        return type1;

    if(type1 > type2)
        qSwap(type1,type2);    
    if (type1 == Type_any || type2 == Type_any)
        return Type_double;
    if(type1 == Type_int && type2 == Type_int64)
        return Type_int64;
    if((type1 == Type_int || type1 == Type_int64) && type2 == Type_double)    
        return Type_double;
            
    return Type_none;
}

bool JZNodeType::canConvert(int type1,int type2)
{
    if(type1 == type2 || type1 == Type_any || type2 == Type_any)
        return true;
    if(isNumber(type1) && isNumber(type2))
        return true;
    if (type1 == Type_int && isEnum(type2))
        return true;
    if (type1 == Type_nullptr && type2 >= Type_object)
        return true;
    if(type1 >= Type_object && type2 >= Type_object)
    {
        auto inst = JZNodeObjectManager::instance();        
        return inst->isInherits(type1,type2);
    }
    int64_t id = makeConvertId(type1, type2);
    if (convertMap.contains(id))
        return true;

    return false;
}

bool JZNodeType::canConvert(int type1, const QVariant &v)
{
    return canConvert(type1, JZNodeType::variantType(v));
}

QString JZNodeType::toString(JZNodeObject *obj)
{
    QString text = obj->className();
    if (obj->type() == Type_list)
    {        
    }
    else if (obj->type() == Type_map)
    {

    }
    else
    {

    }
    return text;
}

QString JZNodeType::toString(const QVariant &v)
{
    if (isJZObject(v))
    {
        auto obj = toJZObject(v);
        if (obj)
        {
            return toString(obj);            
        }
        else
            return "null";
    }
    else
    {
        return v.toString();
    }    
}

QString JZNodeType::opName(int op)
{
    QMap<int, QString> opNames;
    opNames[OP_add] = "+";
    opNames[OP_sub] = "-";
    opNames[OP_mul] = "*";
    opNames[OP_div] = "/";
    opNames[OP_mod] = "%";
    opNames[OP_eq] = "==";
    opNames[OP_ne] = "!=";
    opNames[OP_le] = "<=";
    opNames[OP_ge] = ">=";
    opNames[OP_lt] = "<";
    opNames[OP_gt] = ">";
    opNames[OP_and] = "and";
    opNames[OP_or] = "or";
    opNames[OP_not] = "not";
    opNames[OP_bitor] = "|";
    opNames[OP_bitand] = "&";
    opNames[OP_bitxor] = "~";
    
    if (opNames.contains(op))
        return opNames[op];
    else
        return QString();    
}

QVariant JZNodeType::value(int type)
{
    return QVariant(typeToQMeta(type));
}

QVariant JZNodeType::convertTo(int type,const QVariant &v)
{
    if (type == Type_any)
        return v;
    int v_type = variantType(v);
    if (v_type == Type_string && type == Type_bool)
    {
        QString text = v.toString();
        if (text.isEmpty() || text == "false" || text == "0")
            return false;
        else
            return true;
    }
    if (v_type == Type_nullptr && type >= Type_object)
    {        
        return QVariant::fromValue(JZObjectNull(type));
    }
    int64_t cvt_id = makeConvertId(v_type, type);
    auto it = convertMap.find(cvt_id);
    if (it != convertMap.end())
        return it.value()(v);

    auto qtype = typeToQMeta(type);
    QVariant ret = v;
    ret.convert(qtype);
    return ret;
}

int JZNodeType::upType(int type1, int type2)
{
    if (type1 > type2)
        std::swap(type1, type2);

    if (type1 >= Type_bool && type1 <= Type_double
        && type2 >= Type_bool && type2 <= Type_double)
        return type2;

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

QVariant JZNodeType::matchValue(int dataType,const QVariant &value)
{
    if (dataType == Type_any)
        return value;

    if (JZNodeType::isBase(dataType))
    {
        QVariant::Type q_type = JZNodeType::typeToQMeta(dataType);
        if (value.isNull())
            return QVariant(q_type);
        else
        {
            return convertTo(dataType, value);
        }
    }
    else if (JZNodeType::isEnum(dataType))
    {
        if (value.isNull())
        {
            auto meta = JZNodeObjectManager::instance()->enumMeta(dataType);
            return meta->value(0);
        }
        else
            return value.toInt();
    }
    else if(JZNodeType::isObject(dataType))
    {        
        auto v_type = variantType(value);
        if (isInherits(v_type, dataType))
            return value;
        else
            return QVariant::fromValue(JZObjectNull(dataType));
    }
    else
    {
        Q_ASSERT(false);
        return QVariant();
    }
}

int JZNodeType::matchType(QList<int> dst_types, QList<int> src_types)
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
            else if(src_types[i] == Type_any)
            {
                if (j == 0)
                    near_type = dst_types[0];
                else
                    near_type = upType(near_type,dst_types[j]);
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

int JZNodeType::matchType(QList<int> dst_types, const QString &text)
{    
    if (text.isEmpty())
        return upType(dst_types);

    JZRegExpHelp help;
    bool isInt = help.isInt(text);
    bool isHex = help.isHex(text);
    bool isFloat = help.isFloat(text);
    if (dst_types.contains(Type_int) && (isInt || isHex))
        return Type_int;    
    if (dst_types.contains(Type_double) && (isInt || isHex || isFloat))    
        return Type_double;    
    if (dst_types.contains(Type_bool) && help.isBool(text))
        return Type_bool;
    if (dst_types.contains(Type_string) && text.front() == '"' && text.back() == '"')
        return Type_string;
    if (dst_types.contains(Type_any))    
        return Type_any;

    for (int i = 0; i < dst_types.size(); i++)
    {
        if (isEnum(dst_types[i]))
        {
            auto meta = JZNodeObjectManager::instance()->enumMeta(dst_types[i]);
            if (meta->hasKey(text))
                return dst_types[i];
        }
        if (isObject(dst_types[i]) && text == "null")
            return dst_types[i];
    }   

    return Type_none;
}

QVariant JZNodeType::initValue(int type, const QString &text)
{
    if (text.isEmpty())
        return JZNodeType::matchValue(type, QVariant());

    bool type_any = (type == Type_any);
    if (type == Type_string || type_any)
    {
        if(text.front() == '"' && text.back() == '"')
            return text.mid(1, text.size() - 2);
    }
    if (type == Type_bool || type_any)
    {
        if (text == "false")
            return false;
        else if (text == "true")
            return true;
    }
    if(type >= Type_object || type_any)
    {
        if(text == "null")
            return QVariant::fromValue(JZObjectNull(type));        
    }
    if (type == Type_int || type == Type_int64 || type == Type_double || type_any)
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

    Q_ASSERT(0);    
    return QVariant();
}

bool JZNodeType::isMatchValue(const QList<int> &dst_types, const QString &v)
{
    return matchType(dst_types, v) != Type_none;
}

QString JZNodeType::dispString(const QString &text)
{
    if (text.size() < 2 || !(text.front() == '"' && text.back() == '"'))
        return QString();

    if (text.size() == 2 || text.indexOf(' ') >= 0)
        return text;
    
    return text.mid(1, text.size() - 2);
}

QString JZNodeType::storgeString(const QString &text)
{
    return '"' + text + '"';
}

QString JZNodeType::addMark(const QString &text)
{
    return '"' + text + '"';
}

QString JZNodeType::removeMark(const QString &text)
{
    if (text.isEmpty())
        return QString();

    Q_ASSERT(text.size() >= 2 && text.front() == '"' && text.back() == '"');
    return text.mid(1,text.size() - 2);
}

QVariant JZNodeType::defaultValue(int type)
{
    return initValue(type, QString());
}

void JZNodeType::registConvert(int from, int to, ConvertFunc func)
{
    int id = (int64_t)from << 32 | (int64_t)to;
    convertMap[id] = func;
}