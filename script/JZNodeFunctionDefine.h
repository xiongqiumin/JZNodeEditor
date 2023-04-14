#ifndef JZNODE_FUNCTION_H_
#define JZNODE_FUNCTION_H_

#include "JZNode.h"

enum
{
    Func_none,    
    Func_ii_i,
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
    //for c
    int functionType;
    void *pointer;
};


#endif
