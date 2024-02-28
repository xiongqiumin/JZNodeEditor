#ifndef JZNODE_TYPE_H_
#define JZNODE_TYPE_H_

#include <QVariant>
#include <QDataStream>
#include <QSharedPointer>

enum
{
    Type_none = -1,
    Type_bool,
    Type_int,
    Type_int64,
    Type_double,
    Type_string,
    Type_nullptr,    
    Type_any,
    Type_function,

    Type_enum = 1000,    
    Type_keyCode,   //Qt::Key

    Type_internalEnum = 1100,

    Type_object = 5000,    
    Type_list,
    Type_map,
    Type_timer,
    Type_widget,

    Type_internalObject = 8000, // 内部注册起始
    Type_userObject = 10000,    // 用户注册起始
};

typedef QVariant (*ConvertFunc)(const QVariant& v);
typedef QSharedPointer<QVariant>    JZVariantPtr;
typedef QList<JZVariantPtr>         JZVariantList;
typedef QMap<QString, JZVariantPtr> JZVariantMap;
typedef QMap<int, JZVariantPtr>     JZVariantIntMap;

class JZNodeObject;
class JZNodeType
{
public:
    static void init();
    static QString typeToName(int id);
    static int nameToType(QString name);
    static int variantType(const QVariant &v);    
    static QVariant::Type typeToQMeta(int id);
    static int typeidToType(QString name);
    static QVariant convertTo(int type,const QVariant &v);
    static QVariant value(int type);    
        
    static bool isBase(int type);    
    static bool isEnum(int type);
    static bool isBaseOrEnum(int type);
    static bool isBool(int type);
    static bool isNumber(int type);
    static bool isObject(int type);
   
    static bool isNullptr(const QVariant &v);
    static bool isWidget(const QVariant &v);

    static int isInherits(int type1,int type2);
    static int calcExprType(int type1,int type2);
    static bool canConvert(int type1,int type2);    
    static bool canConvert(int type1,const QVariant &v);
    static QString toString(const QVariant &v);
    static QString toString(JZNodeObject *obj);
    
    static int upType(int type1, int type2);  //提升类型
    static int upType(QList<int> types);
    static QVariant matchValue(int type,const QVariant &v);
    static int matchType(QList<int> dst_types, QList<int> src_types);
    static int matchType(QList<int> dst_types, const QString &v);
    static QVariant initValue(int type, const QString &v);
    
    static QString opName(int op);

    static void registConvert(int from, int to, ConvertFunc func);
};

#endif
