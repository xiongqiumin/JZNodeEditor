#ifndef JZNODE_FUNCTION_DEFINE_H_
#define JZNODE_FUNCTION_DEFINE_H_

#include "JZNode.h"

class JZNodeScript;

class FunctionParam
{
public:
    QString name;

    QStringList input;
    QString output;
};

class CFunction
{
public:
    CFunction();
    virtual ~CFunction();
    virtual void call(const QVariantList &in,QVariantList &out) = 0;

    QStringList args;
    QString result;
};

class FunctionDefine
{
public:
    FunctionDefine();    
    
    bool isCFunction;
    QString name;
    QList<JZNodePin> paramIn;
    QList<JZNodePin> paramOut;    
    
    //for node
    int addr;
    QString script;

    //for c    
    CFunction *cfunc;    
};
QDataStream &operator<<(QDataStream &s, const FunctionDefine &param);
QDataStream &operator>>(QDataStream &s, FunctionDefine &param);

#endif
