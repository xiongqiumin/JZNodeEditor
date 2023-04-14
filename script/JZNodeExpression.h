#ifndef JZNODE_EXPRESSION_H_
#define JZNODE_EXPRESSION_H_

#include "JZNode.h"
#include <QVector>
#include "JZExpression.h"

class JZNodeOperator: public JZNode
{
public:
    JZNodeOperator();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:
    int m_op;    
    int m_in1,m_in2,m_out;
};

//JZNodeAdd
class JZNodeAdd: public JZNodeOperator
{
public:
    JZNodeAdd();
};

//JZNodeSub
class JZNodeSub: public JZNodeOperator
{
public:
    JZNodeSub();
};
    
//JZNodeMul
class JZNodeMul: public JZNodeOperator
{
public:
    JZNodeMul();
};

//JZNodeDiv
class JZNodeDiv: public JZNodeOperator
{
public:
    JZNodeDiv();
};

//JZNodeMod
class JZNodeMod: public JZNodeOperator
{
public:
    JZNodeMod();
};
    
//JZNodeEQ
class JZNodeEQ: public JZNodeOperator
{
public:
    JZNodeEQ();
};

//JZNodeNE
class JZNodeNE: public JZNodeOperator
{
public:
    JZNodeNE();
};

//JZNodeLE
class JZNodeLE: public JZNodeOperator
{
public:
    JZNodeLE();
};

//JZNodeGE
class JZNodeGE: public JZNodeOperator
{
public:
    JZNodeGE();
};

//JZNodeLT
class JZNodeLT: public JZNodeOperator
{
public:
    JZNodeLT();
};

//JZNodeGT
class JZNodeGT: public JZNodeOperator
{
public:
    JZNodeGT();
};

//JZNodeAnd
class JZNodeAnd: public JZNodeOperator
{
public:
    JZNodeAnd();
};

//JZNodeOr
class JZNodeOr: public JZNodeOperator
{
public:
    JZNodeOr();
};
    
//JZNodeXor
class JZNodeXor: public JZNodeOperator
{
public:
    JZNodeXor();
};

//JZNodeExpression
class JZNodeExpression: public JZNode
{
public:
    JZNodeExpression();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
    QString expression;
};

#endif
