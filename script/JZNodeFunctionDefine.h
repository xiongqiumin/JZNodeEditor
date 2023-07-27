#ifndef JZNODE_FUNCTION_DEFINE_H_
#define JZNODE_FUNCTION_DEFINE_H_

#include "JZNodeType.h"

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
        
    bool isCFunction;
    bool isFlowFunction;
    QString name;
    QList<JZParamDefine> paramIn;
    QList<JZParamDefine> paramOut;
    
    //for node
    int addr;
    QString script;

    //for c    
    CFunction *cfunc;    
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

#endif
