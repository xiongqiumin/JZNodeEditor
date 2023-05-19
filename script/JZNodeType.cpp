#include "JZNodeType.h"
#include <QVariant>

bool JZNodeType::isNumber(int type)
{
    if(type == Type_int || type == Type_int64 || type == Type_double)
        return true;
    
    return false;
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
    if(type1 == type2 || type2 == Type_any)
        return true;
    if(isNumber(type1) && isNumber(type2))
        return true;

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