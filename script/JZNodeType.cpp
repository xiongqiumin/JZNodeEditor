#include "JZNodeType.h"
#include <QVariant>
#include "JZNodeObject.h"
#include <QMap>

static QMap<QString,int> typeMap;

void JZNodeType::init()
{
     typeMap["any"]    = Type_any;
     typeMap["bool"]   = Type_bool;
     typeMap["int"]    = Type_int;
     typeMap["int64"]  = Type_int64;
     typeMap["double"] = Type_double;
     typeMap["string"] = Type_string;
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
    if(type == Type_bool)
        return QVariant::Bool;
    else if(type == Type_int)
        return QVariant::Int;
    else if(type == Type_int64)
        return QVariant::LongLong;
    else if(type == Type_double)
        return QVariant::Double;
    else if(type == Type_string)
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
        {
            auto obj = toJZObject(v);
            return obj->define->id;
        }
    }
    return Type_none;
}


bool JZNodeType::isNumber(int type)
{
    if(type == Type_int || type == Type_int64 || type == Type_double)
        return true;
    
    return false;
}

bool JZNodeType::isObject(int type)
{
    return type >= Type_object;
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
    if(isNumber(type1) && type2 == Type_bool)
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
