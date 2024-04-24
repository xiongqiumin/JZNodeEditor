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
    QString initValue() const;

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
    
    void updateParam(CFunction *func);
    void setDefaultValue(int index, QStringList values);
            
    QString name;    
    QString className;
    bool isFlowFunction;
    bool isCFunction;
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

    virtual void connect(JZNodeObject *obj) = 0;
    virtual void disconnect(JZNodeObject *obj) = 0;

protected:
    Q_DISABLE_COPY(CSingle);       
};

class SingleDefine
{
public:
    SingleDefine();

    QString fullName() const;

    int eventType;
    QString name;
    QString className;
    QList<JZParamDefine> paramOut;

    bool isCSingle;
    CSingle *csingle;
};
QDataStream &operator<<(QDataStream &s, const SingleDefine &param);
QDataStream &operator>>(QDataStream &s, SingleDefine &param);

//event
class EventDefine
{
public:
    EventDefine();

    int eventType;
    QString name;        
};
QDataStream &operator<<(QDataStream &s, const EventDefine &param);
QDataStream &operator>>(QDataStream &s, EventDefine &param);

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

    JZFunctionDefine define() const;
    QString fullName() const;

    bool isCFunction() const;    
    bool isMemberFunction() const;
    bool isFlowFunction() const;

    QString name;
    QString className;
    QList<JZParam> paramIn;
    QList<JZParam> paramOut;    
    
    bool flow;

    //script
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
