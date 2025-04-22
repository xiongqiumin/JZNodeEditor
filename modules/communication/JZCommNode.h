#ifndef JZ_COMM_NODE_H_
#define JZ_COMM_NODE_H_

#include "JZNode.h"

enum CommNode
{
    Node_CommId = 1700,
    Node_CommInit,
};

class JZCommInitNode : public JZNode
{
public:
    JZCommInitNode();
    ~JZCommInitNode();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error);
};

#endif