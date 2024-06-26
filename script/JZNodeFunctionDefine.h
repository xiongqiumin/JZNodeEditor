#ifndef JZNODE_FUNCTION_DEFINE_H_
#define JZNODE_FUNCTION_DEFINE_H_

#include "JZNodeType.h"
#include <QSharedPointer>

//JZParamDefine
class JZParamDefine
{
public:
    JZParamDefine();
    JZParamDefine(QString name, int dataType, const QString &v = QString());
    JZParamDefine(QString name, QString dataType, const QString &v = QString());    

    int dataType() const;

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

    QString variable;
    QString widget;
    int dir;
};
QDataStream &operator<<(QDataStream &s, const JZNodeParamBind &param);
QDataStream &operator>>(QDataStream &s, JZNodeParamBind &param);

//CFunction
class JZNodeObject;
class CFunction
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

class JZFunctionDefine
{
public:
    JZFunctionDefine();
    
    bool isNull() const;

    QString fullName() const;    
    bool isMemberFunction() const;    
    
    void setDefaultValue(int index, QString text);
    void setDefaultValue(int from, QStringList values);

    void updateParam(CFunction *func);
        
    QString name;    
    QString className;
    bool isFlowFunction;
    bool isVirtualFunction;
    bool isProtected;
    bool isCFunction;
    bool isSlot;
    bool isSingle;
    QList<JZParamDefine> paramIn;
    QList<JZParamDefine> paramOut;       
};
QDataStream &operator<<(QDataStream &s, const JZFunctionDefine &param);
QDataStream &operator>>(QDataStream &s, JZFunctionDefine &param);

//single
class CSingle
{
public:
    CSingle();
    virtual ~CSingle();

    virtual void connect(JZNodeObject *sender,JZNodeObject *recv,QString slot) = 0;
    virtual void disconnect(JZNodeObject *sender,JZNodeObject *recv,QString slot) = 0;

protected:
    Q_DISABLE_COPY(CSingle);       
};

class JZSingleDefine
{
public:
    JZSingleDefine();

    QString fullName() const;
    bool isCSingle() const;

    QString name;
    QString className;
    QList<JZParamDefine> paramOut;

    CSingle *csingle;
};
QDataStream &operator<<(QDataStream &s, const JZSingleDefine &param);
QDataStream &operator>>(QDataStream &s, JZSingleDefine &param);

//JZParam
class JZParam
{
public:
    static JZParam formDefine(const JZParamDefine &def);

    JZParam();
    JZParam(const QString &name, int type);

    JZParamDefine define() const;

    QString name;
    int dataType;    
};
QDataStream &operator<<(QDataStream &s, const JZParam &param);
QDataStream &operator>>(QDataStream &s, JZParam &param);

//JZFunction
class JZFunction
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

    JZFunctionDefine define;  
    
    //script
    QString path;
    int addr;
    int addrEnd;
    QString file;
    QList<JZParamDefine> localVariables;

    //c
    QSharedPointer<CFunction> cfunc;
};
QDataStream &operator<<(QDataStream &s, const JZFunction &param);
QDataStream &operator>>(QDataStream &s, JZFunction &param);

#endif
