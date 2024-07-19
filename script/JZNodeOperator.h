#ifndef JZNODE_OPERATOR_H_
#define JZNODE_OPERATOR_H_

#include "JZNode.h"

class JZNodeOperator: public JZNode
{
public:
    JZNodeOperator(int node_type,int op_type);

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
    virtual JZNodePinWidget* createWidget(int id) override;

    void addInput();
    void removeInput(int index);    
    int op() const;

protected:
    void addInputButton();    

    virtual QStringList pinActionList(int id) override;
    virtual bool pinActionTriggered(int id, int index) override;

    bool checkPinInput(JZNodeCompiler *compiler,QString &error);
    void calcPinOutType(JZNodeCompiler *compiler);

    int m_op;    
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

//JZNodeBitAnd
class JZNodeBitAnd: public JZNodeOperator
{
public:
    JZNodeBitAnd();
};

//JZNodeBitAnd
class JZNodeBitOr: public JZNodeOperator
{
public:
    JZNodeBitOr();
};
    
//JZNodeBitXor
class JZNodeBitXor: public JZNodeOperator
{
public:
    JZNodeBitXor();
};

//JZNodeBitResver
class JZNodeBitResver : public JZNode
{
public:
    JZNodeBitResver();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
};

//JZNodeEQ
class JZNodeEQ : public JZNodeOperator
{
public:
    JZNodeEQ();
};

//JZNodeNE
class JZNodeNE : public JZNodeOperator
{
public:
    JZNodeNE();
};

//JZNodeLE
class JZNodeLE : public JZNodeOperator
{
public:
    JZNodeLE();
};

//JZNodeGE
class JZNodeGE : public JZNodeOperator
{
public:
    JZNodeGE();
};

//JZNodeLT
class JZNodeLT : public JZNodeOperator
{
public:
    JZNodeLT();
};

//JZNodeGT
class JZNodeGT : public JZNodeOperator
{
public:
    JZNodeGT();
};

//JZNodeAnd
class JZNodeAnd : public JZNodeOperator
{
public:
    JZNodeAnd();

    bool compiler(JZNodeCompiler *c, QString &error);
};

//JZNodeOr
class JZNodeOr : public JZNodeOperator
{
public:
    JZNodeOr();

    bool compiler(JZNodeCompiler *c, QString &error);
};

//JZNodeNot
class JZNodeNot : public JZNode
{
public:
    JZNodeNot();

    bool compiler(JZNodeCompiler *c, QString &error);
};

#endif
