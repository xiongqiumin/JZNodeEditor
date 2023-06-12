#ifndef JZNODE_FUNCTION_DEFINE_H_
#define JZNODE_FUNCTION_DEFINE_H_

#include "JZNode.h"

class JZNodeScript;
class CFunction
{
public:
    CFunction();
    virtual ~CFunction();
    virtual void call(const QVariantList &in,QVariantList &out) = 0;

    QStringList args;
    QString result;
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

#endif
