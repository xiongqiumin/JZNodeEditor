#ifndef JZ_NODE_COMPILER_INFO_H_
#define JZ_NODE_COMPILER_INFO_H_

#include "JZNodeIR.h"

struct ClassInitInfo
{
    QString function;
    QList<JZNodeIRParam> irList;
};


#endif