#include <QVariant>
#include <QMap>
#include "JZNodeType.h"
#include "JZNodeObject.h"
#include "JZNodeIR.h"
#include "JZRegExpHelp.h"

static QMap<QString,int> typeMap;

void JZNodeType::init()
{
     typeMap["any"]    = Type_any;
     typeMap["bool"]   = Type_bool;
     typeMap["int"]    = Type_int;
     typeMap["int64"]  = Type_int64;
     typeMap["double"] = Type_double;
     typeMap["string"] = Type_string;
     typeMap["nullptr"] = Type_nullptr;
}

QString JZNodeType::typeToName(int id)
{
    if(id < Type_object)
        return typeMap.key(id,QString());
    else
        return JZNodeObjectManager::instance()->getClassName(id);
}

int JZNodeType::nameToType(QString name)
{
    if(typeMap.contains(name))
        return typeMap.value(name,Type_none);

    return JZNodeObjectManager::instance()->getClassId(name);
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
        return JZNodeObjectManager::instance()->getClassIdByTypeid(name);
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
        if(v.userType() == qMetaTypeId<JZObjectNull>())
            return Type_nullptr;
        else if(isJZObject(v))
        {
            auto obj = toJZObject(v);
            return obj->define->id;
        }
    }
    return Type_none;
}


bool JZNodeType::isNumber(int type)
{
    if(type == Type_int || type == Type_int64 || type == Type_double || type == Type_bool)
        return true;
    
    return false;
}

bool JZNodeType::isBaseType(int type)
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
    if(type1 == type2)
        return type1;
    
    if(type1 > type2)
        qSwap(type1,type2);    
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
    if (type1 == Type_nullptr && type2 >= Type_object)
        return true;
    if(type1 >= Type_object && type2 >= Type_object)
    {
        auto inst = JZNodeObjectManager::instance();        
        return inst->isInherits(type1,type2);
    }

    return false;
}

bool JZNodeType::canConvert(QList<int> type1,QList<int> type2)
{
    for(int i = 0; i < type1.size(); i++)
    {
        for(int j = 0; j < type2.size(); j++)
        {
            if(canConvert(type1[i],type2[j]))
                return true;
        }
    }
    return false;
}

QString JZNodeType::toString(const QVariant &v)
{
    return v.toString();
}

QString JZNodeType::opName(int op)
{
    QStringList opNames = QStringList{ "+","-","*","/","%","==","!=","<=",">=","<",">","and","or","not","|","&","^" };
    Q_ASSERT(op >= OP_add && op <= OP_bitxor);
    return opNames[op - OP_add];
}

QVariant JZNodeType::value(int type)
{
    return QVariant(typeToQMeta(type));
}

QVariant JZNodeType::convertTo(const QVariant &v, int type)
{
    if (type == Type_any)
        return v;
    int v_type = variantType(v);
    if (v_type == Type_nullptr && type > Type_object)
        return v;

    auto qtype = typeToQMeta(type);
    QVariant ret = v;
    ret.convert(qtype);
    return ret;
}

QVariant JZNodeType::matchValue(const QVariant &v, QList<int> type)
{
    int v_type = variantType(v);
    if (type.contains(v_type))
        return v;

    for(int i = 0; i < type.size(); i++)
    {
        if (canConvert(v_type, type[i]))
            return convertTo(v, type[i]);
    }

    if (v.type() == QMetaType::QString)
    {
        QString text = v.toString();

        JZRegExpHelp help;        
        bool isInt = help.isInt(text);
        bool isHex = help.isHex(text);
        bool isFloat = help.isFloat(text);
        if (type.contains(Type_int) && (isInt || isHex))
        {
            if (isInt)
                return text.toInt();
            else
                return text.toInt(nullptr, 16);
        }
        if (type.contains(Type_double) && (isInt || isHex || isFloat))
        {
            if(isHex)
                return text.toInt(nullptr, 16);
            else
                return text.toDouble();
        }
    }

    return value(type[0]);
}

//JZParamDefine
JZParamDefine::JZParamDefine()
{
    dataType = Type_none;
    cref = false;
}

JZParamDefine::JZParamDefine(QString name, int dataType, const QVariant &v)
{
    this->name = name;
    this->dataType = dataType;
    this->value = v;
    this->cref = false;
}

QVariant JZParamDefine::initialValue() const
{
    if (dataType < Type_object)
    {        
        QVariant::Type q_type = JZNodeType::typeToQMeta(dataType);
        if (value.isNull()) 
            return QVariant(q_type);
        else
        {
            QVariant v = value;
            if(v.convert(q_type))
                return v;
            else
                return QVariant(q_type);
        }        
    }
    else
    {
        return QVariant::fromValue(JZObjectNull());
    }
}

QDataStream &operator<<(QDataStream &s, const JZParamDefine &param)
{
    s << param.name;
    s << param.dataType;
    s << param.value;
    s << param.cref;
    return s;
}

QDataStream &operator >> (QDataStream &s, JZParamDefine &param)
{
    s >> param.name;
    s >> param.dataType;
    s >> param.value;
    s >> param.cref;
    return s;
}