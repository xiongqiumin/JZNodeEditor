#ifndef JZNODE_TYPE_H_
#define JZNODE_TYPE_H_

#include <QVariant>
#include <QDataStream>

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

    Type_enum = 1000,
    Type_flag = 2000,

    Type_object = 5000,    
    Type_list,
    Type_map,
    Type_timer,

    Type_internalObject = 8000, // 内部注册起始
    Type_userObject = 10000,    // 用户注册起始
};

class JZNodeType
{
public:
    static void init();
    static QString typeToName(int id);
    static int nameToType(QString name);
    static int variantType(const QVariant &v);
    static QVariant::Type typeToQMeta(int id);
    static int typeidToType(QString name);
    
    static bool isBaseType(int id);    
    static bool isObject(int type);
    static bool isNumber(int type);
    static int isInherits(int type1,int type2);
    static int calcExprType(int type1,int type2);
    static bool canConvert(int type1,int type2);
    static bool canConvert(QList<int> type1,QList<int> type2);
    static QString toString(const QVariant &v);
    static QString opName(int op);
    
    static QVariant matchValue(const QVariant &v, QList<int> type);
};

class JZParamDefine
{
public:
    JZParamDefine();
    JZParamDefine(QString name, int dataType, const QVariant &v = QVariant());
    QVariant initialValue() const;

    QString name;
    int dataType;
    QVariant value;
    bool cref;         //从c++侧引用
};
QDataStream &operator<<(QDataStream &s, const JZParamDefine &param);
QDataStream &operator >> (QDataStream &s, JZParamDefine &param);

#endif
