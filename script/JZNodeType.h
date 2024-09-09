#ifndef JZNODE_TYPE_H_
#define JZNODE_TYPE_H_

#if defined(_MSC_VER) && _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include <QVariant>
#include <QDataStream>
#include <QSharedPointer>

constexpr int INVALID_ID = -1; 

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
    Type_auto,      //auto
    Type_arg,       //泛型,任意参数
    Type_args,      //变长参数

    Type_internalUse,
    Type_paramName,
    Type_hookEnable,
    Type_ignore,
    Type_internalUseEnd,

    Type_enum = 1000,    
    Type_keyCode,   //Qt::Key

    Type_internalEnum = 1100,

    Type_class = 5000,
    Type_stringList,
    Type_varList,    
    Type_intList,
    Type_doubleList,
    Type_varMap,
    Type_intIntMap,
    Type_intStringMap,
    Type_stringIntMap,
    Type_stringStringMap,

    Type_byteArray,
    Type_dataStream,
    Type_point,
    Type_pointF,
    Type_rect,
    Type_rectF,
    Type_color,

    Type_image,
    Type_painter,
    Type_pen,
    Type_brush,

    Type_event,
    Type_resizeEvent,
    Type_showEvent,
    Type_paintEvent,
    Type_closeEvent,
    Type_keyEvent,
    Type_mouseEvent,

    Type_layout,
    Type_boxLayout,
    Type_hBoxLayout,
    Type_vBoxLayout,
    Type_gridLayout,

    Type_object,  //qobject
    Type_timer,
    
    Type_widget,
    Type_frame,
    Type_label,
    Type_lineEdit,
    Type_textEdit,
    Type_pushButton,
    Type_toolButton,
    Type_radioButton,
    Type_checkBox,
    Type_comboBox,
    Type_spin,
    Type_doubleSpin,
    Type_listWidget,
    Type_listWidgetItem,
    Type_tableWidget,
    Type_tableWidgetItem,
    Type_treeWidget,
    Type_treeWidgetItem,

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
Q_DECLARE_METATYPE(JZFunctionPointer)
QDataStream &operator<<(QDataStream &s, const JZFunctionPointer &param);
QDataStream &operator>>(QDataStream &s, JZFunctionPointer &param);

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
class JZSignalDefine;
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
    static int nameToType(const QString &name);
    static int typeidToType(const QString &name);
    static int variantType(const QVariant &v);
    static QString variantTypeName(const QVariant &v);
    static int stringType(const QString &text);

    static bool canConvert(int from,int to);    //隐式转换
    static bool canConvertExplicitly(int from,int to);    //被 convertTo 支持的
    static QVariant convertTo(int type,const QVariant &v); 
    static void registConvert(int from, int to, ConvertFunc func);
    static QVariant clone(const QVariant &v);
        
    static bool isBase(int type);    
    static bool isEnum(int type);
    static bool isBaseOrEnum(int type);
    static bool isBool(int type);
    static bool isNumber(int type);
    static bool isObject(int type);
    static bool isVaildType(QString type);
 
    static bool isNullObject(const QVariant &v);   
    static bool isNullptr(const QVariant &v);
    static bool isWidget(const QVariant &v);
    static bool isSameType(const QVariant &src_v,const QVariant &dst_v);
    static bool isSameType(int src_type,int dst_type);
    static bool isLiteralType(int type);

    static bool isPointer(const QVariant &v);
    static QVariant *getPointer(const QVariant &v);

    static int isInherits(const QString &type1,const QString &type2);
    static int isInherits(int type1,int type2);
    static int calcExprType(int type1,int type2,int op);
        
    static QString debugString(const QVariant &v);
    static QString debugString(const JZNodeObject *obj);
    
    static int upType(int type1, int type2);  //提升类型
    static int upType(QList<int> types);
    static int matchType(QList<int> src_types,QList<int> dst_types);
    static bool canInitValue(int type,const QString &v);
    static QVariant defaultValue(int type);
    static QVariant initValue(int type, const QString &v);

    static bool sigSlotTypeMatch(const JZSignalDefine *sig,const JZFunctionDefine *slot);
    static bool functionTypeMatch(const JZFunctionDefine *func1,const JZFunctionDefine *func2);
};

#endif
