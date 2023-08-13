#ifndef JZNODE_FUNCTION_DEFINE_H_
#define JZNODE_FUNCTION_DEFINE_H_

#include "JZNodeType.h"
#include <QSharedPointer>

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

    QString fullName() const;
    void updateCFunctionParam();
            
    QString name;
    QString className;
    bool isFlowFunction;    
    QList<JZParamDefine> paramIn;
    QList<JZParamDefine> paramOut;       

    //for c    
    bool isCFunction;
    QSharedPointer<CFunction> cfunc;
};
QDataStream &operator<<(QDataStream &s, const FunctionDefine &param);
QDataStream &operator>>(QDataStream &s, FunctionDefine &param);

//single
class CSingle
{
public:
    CSingle();
    virtual ~CSingle();
    virtual void connect(JZNodeObject *obj,int type) = 0;

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
    QList<JZParamDefine> paramOut;
};
QDataStream &operator<<(QDataStream &s, const EventDefine &param);
QDataStream &operator>>(QDataStream &s, EventDefine &param);

#endif
