#ifndef JZNODE_TYPE_H_
#define JZNODE_TYPE_H_

#include <QVariant>

enum
{
    Type_none = -1,
    Type_any,    
    Type_bool,
    Type_int,
    Type_int64,
    Type_double,
    Type_string,

    Type_object = 1000,    
    Type_list,
    Type_map,

    Type_objectRegistId = 2000,
    Type_userObject = 10000,
};

class JZNodeType
{
public:
    static void init();
    static QString idToName(int id);
    static int nameToId(QString name);
    static int typeidToId(QString name);
    static int variantId(const QVariant &v);
    static QVariant::Type toVariantType(int id);        
    
    static bool isObject(int type);
    static bool isNumber(int type);
    static int isInherits(int type1,int type2);
    static int calcExprType(int type1,int type2);
    static bool canConvert(int type1,int type2);
    static bool canConvert(QList<int> type1,QList<int> type2);
};

#endif
