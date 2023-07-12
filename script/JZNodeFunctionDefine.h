﻿#ifndef JZNODE_FUNCTION_DEFINE_H_
#define JZNODE_FUNCTION_DEFINE_H_

#include "JZNode.h"

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

class FunctionParam
{
public:
    QString name;
    int dataType;
};

class FunctionDefine
{
public:
    FunctionDefine();    
        
    bool isCFunction;
    bool isFlowFunction;
    QString name;
    QList<FunctionParam> paramIn;
    QList<FunctionParam> paramOut;
    
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

    QString name;
    int eventType;

    bool isCSingle;
    CSingle *csingle;
};
QDataStream &operator<<(QDataStream &s, const SingleDefine &param);
QDataStream &operator>>(QDataStream &s, SingleDefine &param);

#endif
