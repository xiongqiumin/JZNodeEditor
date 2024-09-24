#ifndef JZNODE_FUNCTION_DEFINE_H_
#define JZNODE_FUNCTION_DEFINE_H_

#include "JZNodeType.h"
#include <QSharedPointer>

//JZParamDefine
class JZCORE_EXPORT JZParamDefine
{
public:
    JZParamDefine();
    JZParamDefine(QString name, QString dataType, const QString &v = QString());

    QString name;
    QString type;
    QString value;
};
QDataStream &operator<<(QDataStream &s, const JZParamDefine &param);
QDataStream &operator>>(QDataStream &s, JZParamDefine &param);

//JZNodeParamBind
class JZNodeParamBind {
public:
    enum {
        UiToData = 0x1,
        DataToUi = 0x2,
        Duplex = UiToData | DataToUi,
    };

    JZNodeParamBind();

    QString widget;
    int widgetProp;
    QString variable;
    int dir;
};
QDataStream &operator<<(QDataStream &s, const JZNodeParamBind &param);
QDataStream &operator>>(QDataStream &s, JZNodeParamBind &param);

//CFunction
class JZNodeObject;
class JZCORE_EXPORT CFunction
{
public:
    CFunction();
    virtual ~CFunction();
    virtual void call(const QVariantList &in,QVariantList &out) = 0;

    QStringList args;
    QString result;

protected:
    Q_DISABLE_COPY(CFunction);
};
typedef QSharedPointer<CFunction> CFunctionPtr;

class JZNodeEngine;
class JZCORE_EXPORT BuiltInFunction
{
public:
    BuiltInFunction();
    virtual ~BuiltInFunction();
    virtual void call(JZNodeEngine *engine) = 0;

protected:
    Q_DISABLE_COPY(BuiltInFunction);
};
typedef QSharedPointer<BuiltInFunction> BuiltInFunctionPtr;

//JZCParamDefine
class JZCORE_EXPORT JZCParamDefine
{
public:
    CFunctionPtr read;
    CFunctionPtr write;
};

class JZCORE_EXPORT JZFunctionDefine
{
public:
    JZFunctionDefine();
    
    bool isNull() const;

    void setFullName(const QString &name);
    QString fullName() const;    
    bool isMemberFunction() const;    
    bool isVariadicFunction() const;

    QString delcare() const;
    
    void setDefaultValue(int index, QString text);
    void setDefaultValue(int from, QStringList values);    
        
    QString name;    
    QString className;
    bool isCFunction;
    bool isFlowFunction;
    bool isVirtualFunction;
    bool isProtected;
    bool isSlot;
    bool isSingle;
    QList<JZParamDefine> paramIn;
    QList<JZParamDefine> paramOut;       
};
QDataStream &operator<<(QDataStream &s, const JZFunctionDefine &param);
QDataStream &operator>>(QDataStream &s, JZFunctionDefine &param);

//signal
class JZCORE_EXPORT CSignal
{
public:
    CSignal();
    virtual ~CSignal();

    virtual void connect(JZNodeObject *sender,JZNodeObject *recv,QString slot) = 0;
    virtual void disconnect(JZNodeObject *sender,JZNodeObject *recv,QString slot) = 0;

protected:
    Q_DISABLE_COPY(CSignal);       
};

class JZCORE_EXPORT JZSignalDefine
{
public:
    JZSignalDefine();

    QString fullName() const;
    bool isCSignal() const;

    QString name;
    QString className;
    QList<JZParamDefine> paramOut;
    CSignal *csignal;
};
QDataStream &operator<<(QDataStream &s, const JZSignalDefine &param);
QDataStream &operator>>(QDataStream &s, JZSignalDefine &param);

//JZParam
class JZCORE_EXPORT JZParam
{
public:
    JZParam();
    JZParam(const QString &name, int type);

    QString name;
    int dataType;    
};
QDataStream &operator<<(QDataStream &s, const JZParam &param);
QDataStream &operator>>(QDataStream &s, JZParam &param);

//JZFunction
class JZCORE_EXPORT JZFunction
{
public:
    JZFunction();
    ~JZFunction();

    QString name() const;
    QString className() const;
    QString fullName() const;

    bool isCFunction() const;
    bool isMemberFunction() const;
    bool isFlowFunction() const;
    bool isVirtualFunction() const;

    JZFunctionDefine define;  
    
    //script
    QString path;
    int addr;
    int addrEnd;

    //c
    QSharedPointer<CFunction> cfunc;
    QSharedPointer<BuiltInFunction> builtIn;
};
QDataStream &operator<<(QDataStream &s, const JZFunction &param);
QDataStream &operator>>(QDataStream &s, JZFunction &param);

#endif
