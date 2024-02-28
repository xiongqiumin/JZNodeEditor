#ifndef JZNODE_FUNCTION_DEFINE_H_
#define JZNODE_FUNCTION_DEFINE_H_

#include "JZNodeType.h"
#include <QSharedPointer>

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

class FunctionDefine
{
public:
    FunctionDefine();    

    bool isNull() const;

    QString fullName() const;    
    bool isMemberFunction() const;
            
    QString name;    
    QString className;
    bool isFlowFunction;
    bool isCFunction;
    QList<JZParamDefine> paramIn;
    QList<JZParamDefine> paramOut;       
};
QDataStream &operator<<(QDataStream &s, const FunctionDefine &param);
QDataStream &operator>>(QDataStream &s, FunctionDefine &param);

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

    int eventType;
    QString name;
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

    FunctionDefine define() const;
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
    QString file;
    QList<JZParamDefine> localVariables;

    //c
    QSharedPointer<CFunction> cfunc;
};
QDataStream &operator<<(QDataStream &s, const JZFunction &param);
QDataStream &operator>>(QDataStream &s, JZFunction &param);

#endif
