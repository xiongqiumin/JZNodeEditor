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
    Type_stringList,
    Type_varList,    
    Type_intList,
    Type_doubleList,
    Type_varMap,
    Type_intIntMap,
    Type_intStringMap,
    Type_StringIntMap,
    Type_StringStringMap,
    Type_timer,
    Type_widget,

    Type_internalObject = 8000, // 内部注册起始
    Type_userObject = 10000,    // 用户注册起始
};

typedef QVariant (*ConvertFunc)(const QVariant& v);
typedef QSharedPointer<QVariant> QVariantPtr;

class JZEnum
{
public:
    JZEnum();
    
    int type;
    int value;
};
QDataStream &operator<<(QDataStream &s, const JZEnum &param);
QDataStream &operator>>(QDataStream &s, JZEnum &param);
Q_DECLARE_METATYPE(JZEnum)

class JZFunctionPointer
{
public:
    bool operator==(const JZFunctionPointer &other);

    QString functionName;
};
QDataStream &operator<<(QDataStream &s, const JZFunctionPointer &param);
QDataStream &operator>>(QDataStream &s, JZFunctionPointer &param);
Q_DECLARE_METATYPE(JZFunctionPointer)

class JZNodeVariantAny
{
public:
    int type();
    QVariant value;
};
Q_DECLARE_METATYPE(JZNodeVariantAny)

class QVariantPointer
{
public:
    QVariant *value;
};
Q_DECLARE_METATYPE(QVariantPointer)

/*
    任意类型可以隐式转换为 any
    any 必须显示转换为指定类型
*/
class JZNodeObject;
class JZSingleDefine;
class JZFunctionDefine;
class JZNodeType
{
public:
    static void init();

    static QString opName(int op);
    static int opType(const QString &name);
    static int opPri(const QString &op);
    static bool isDoubleOp(const QString &op);

    static QString typeToName(int id);
    static int nameToType(QString name);
    static int variantType(const QVariant &v);
    static int typeidToType(QString name);

    static bool canConvert(int from,int to);    //隐式转换
    static QVariant convertTo(int type,const QVariant &v); 
    static void registConvert(int from, int to, ConvertFunc func);
    static QVariant clone(const QVariant &v);
        
    static bool isBase(int type);    
    static bool isEnum(int type);
    static bool isBaseOrEnum(int type);
    static bool isBool(int type);
    static bool isNumber(int type);
    static bool isObject(int type);
 
    static bool isNullObject(const QVariant &v);   
    static bool isNullptr(const QVariant &v);
    static bool isWidget(const QVariant &v);
    static bool isSameType(const QVariant &src_v,const QVariant &dst_v);
    static bool isSameType(int src_type,int dst_type);
    static bool isLiteralType(int type);

    static bool isPointer(const QVariant &v);
    static QVariant *getPointer(const QVariant &v);

    static int isInherits(QString type1,QString type2);
    static int isInherits(int type1,int type2);
    static int calcExprType(int type1,int type2,int op);
        
    static QString debugString(const QVariant &v);
    static QString debugString(const JZNodeObject *obj);
    
    static int upType(int type1, int type2);  //提升类型
    static int upType(QList<int> types);
    static int matchType(QList<int> src_types,QList<int> dst_types);
    static QVariant defaultValue(int type);
    static QVariant initValue(int type, const QString &v);

    static QString dispString(const QString &text);
    static QString storgeString(const QString &text);
    static int stringType(const QString &text);
    
    static QString addQuote(const QString &text);
    static QString removeQuote(const QString &text);

    static bool sigSlotTypeMatch(const JZSingleDefine *sig,const JZFunctionDefine *slot);
    static bool functionTypeMatch(const JZFunctionDefine *func1,const JZFunctionDefine *func2);
};

#endif
