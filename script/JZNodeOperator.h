#ifndef JZNODE_OPERATOR_H_
#define JZNODE_OPERATOR_H_

#include "JZNode.h"

class JZCORE_EXPORT JZNodeOperator: public JZNode
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
class JZCORE_EXPORT JZNodeAdd: public JZNodeOperator
{
public:
    JZNodeAdd();
};

//JZNodeSub
class JZCORE_EXPORT JZNodeSub: public JZNodeOperator
{
public:
    JZNodeSub();
};
    
//JZNodeMul
class JZCORE_EXPORT JZNodeMul: public JZNodeOperator
{
public:
    JZNodeMul();
};

//JZNodeDiv
class JZCORE_EXPORT JZNodeDiv: public JZNodeOperator
{
public:
    JZNodeDiv();
};

//JZNodeMod
class JZCORE_EXPORT JZNodeMod: public JZNodeOperator
{
public:
    JZNodeMod();
};

//JZNodeBitAnd
class JZCORE_EXPORT JZNodeBitAnd: public JZNodeOperator
{
public:
    JZNodeBitAnd();
};

//JZNodeBitAnd
class JZCORE_EXPORT JZNodeBitOr: public JZNodeOperator
{
public:
    JZNodeBitOr();
};
    
//JZNodeBitXor
class JZCORE_EXPORT JZNodeBitXor: public JZNodeOperator
{
public:
    JZNodeBitXor();
};

//JZNodeBitResver
class JZCORE_EXPORT JZNodeBitResver : public JZNode
{
public:
    JZNodeBitResver();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
};

//JZNodeEQ
class JZCORE_EXPORT JZNodeEQ : public JZNodeOperator
{
public:
    JZNodeEQ();
};

//JZNodeNE
class JZCORE_EXPORT JZNodeNE : public JZNodeOperator
{
public:
    JZNodeNE();
};

//JZNodeLE
class JZCORE_EXPORT JZNodeLE : public JZNodeOperator
{
public:
    JZNodeLE();
};

//JZNodeGE
class JZCORE_EXPORT JZNodeGE : public JZNodeOperator
{
public:
    JZNodeGE();
};

//JZNodeLT
class JZCORE_EXPORT JZNodeLT : public JZNodeOperator
{
public:
    JZNodeLT();
};

//JZNodeGT
class JZCORE_EXPORT JZNodeGT : public JZNodeOperator
{
public:
    JZNodeGT();
};

//JZNodeAnd
class JZCORE_EXPORT JZNodeAnd : public JZNodeOperator
{
public:
    JZNodeAnd();

    bool compiler(JZNodeCompiler *c, QString &error);
};

//JZNodeOr
class JZCORE_EXPORT JZNodeOr : public JZNodeOperator
{
public:
    JZNodeOr();

    bool compiler(JZNodeCompiler *c, QString &error);
};

//JZNodeNot
class JZCORE_EXPORT JZNodeNot : public JZNode
{
public:
    JZNodeNot();

    bool compiler(JZNodeCompiler *c, QString &error);
};

#endif
