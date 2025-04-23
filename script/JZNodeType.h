#ifndef JZNODE_TYPE_H_
#define JZNODE_TYPE_H_

#include <QVariant>
#include <QDataStream>
#include <QSharedPointer>

constexpr int INVALID_ID = -1; 

enum
{
    Type_pointerFlag = 1 << 30,
    
    Type_none = -1,
    Type_bool,
    Type_int8,
    Type_int16,
    Type_int,
    Type_int64,
    Type_uint8,
    Type_uint16,
    Type_uint,
    Type_uint64,
    Type_float,
    Type_double,
    Type_string,
    Type_nullptr,
    Type_any,
    Type_function,
    Type_auto,        //auto
    Type_arg,         //泛型,任意参数
    Type_argPointer = (Type_arg | Type_pointerFlag),  //泛型,任意指针
    Type_args = Type_arg + 1,        //变长参数

    Type_enum = 2000,
    Type_keyCode,   //Qt::Key

    Type_internalEnum = 2100,

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
    Type_userObject = 50000,    // 用户注册起始
};

class JZCParamDefine;
class QVariantPtr
{
public:
    QVariantPtr();

    int type;
    QSharedPointer<QVariant> ptr;
    const JZCParamDefine* cparam;
};

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

/*
    任意类型可以隐式转换为 any
    any 必须显示转换为指定类型
*/
class JZNodeVariantAny
{
public:
    JZNodeVariantAny();

    bool operator==(const JZNodeVariantAny &other) const;

    int type();
    QVariant value;
};
Q_DECLARE_METATYPE(JZNodeVariantAny)

//这是一个值 nullptr, 不是类型
class JZNodeObjectNull
{
public:
    JZNodeObjectNull();
};
QDataStream &operator<<(QDataStream &s, const JZNodeObjectNull&param);
QDataStream &operator>>(QDataStream &s, JZNodeObjectNull&param);
Q_DECLARE_METATYPE(JZNodeObjectNull)

class JZNodeObject;
class JZSignalDefine;
class JZFunctionDefine;
class JZNodeType
{
public:
    static void init();

    static QString typeName(int type);
    static int nameToType(const QString &name);

    static QString opName(int op);
    static int opType(const QString &name);
    static int opPri(const QString &op);
    static bool isDoubleOp(const QString &op);    
        
    static int variantType(const QVariant &v);
    static bool variantIsPointer(const QVariant& v);

    static bool isBase(int type);    
    static bool isEnum(int type);
    static bool isBaseOrEnum(int type);
    static bool isBool(int type);
    static bool isNumber(int type);
    static bool isObject(int type);
    static bool isNullObject(const QVariant &v);
    static bool isNullptr(const QVariant &v);    
    static bool isLiteralType(int type);

    static int baseType(int type);
    static int pointerType(int type);
    static bool isPointer(int type);

    static QString baseType(const QString& type);
    static QString pointerType(const QString& type);
    static bool isPointer(const QString& type);
    
    static bool isContainerType(QString value);

    static QString listType(QString value);
    static QString listIteratorType(QString list_type);
    static bool listValueType(QString list_type,QString &value_type);
    static QString mapType(QString key, QString value);
    static QString mapIteratorType(QString map_type);
    static bool mapValueType(QString map_type, QString& value_type,QString& key_type);

    static int calcExprType(int type1,int type2,int op);
        
    static QString debugString(const QVariant &v);
    static QString debugString(const JZNodeObject *obj);
        
    static bool sigSlotTypeMatch(const JZSignalDefine *sig,const JZFunctionDefine *slot);
    
    static QVariant convertNumber(const QVariant& srcValue, int dstType);
    static QVariant convertToPointer(const QVariant& srcValue);
};

#endif
