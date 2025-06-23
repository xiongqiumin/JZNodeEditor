#ifndef JZNODE_OPERATOR_H_
#define JZNODE_OPERATOR_H_

#include "JZNode.h"

class JZNodeOperator : public JZNode
{
public:
    JZNodeOperator();

    bool isFlow();
    void setFlow(bool flag);

protected:
    virtual void saveToStream(QDataStream &s) const override;
    virtual void loadFromStream(QDataStream &s) override;
    void updateFlow();

    bool m_isFlow;
};

class JZNodeDoubleOperator: public JZNodeOperator
{
public:
    JZNodeDoubleOperator(int node_type,JZNodeIRType op_type);

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

    void addInput();
    void removeInput(int index);    
    int op() const;

protected:
    void updatePinName();
    bool checkPinInput(JZNodeCompiler *compiler,QString &error);
    void calcPinOutType(JZNodeCompiler *compiler);

    JZNodeIRType m_op;
};

//JZNodeAdd
class JZNodeAdd: public JZNodeDoubleOperator
{
public:
    JZNodeAdd();
};

//JZNodeSub
class JZNodeSub: public JZNodeDoubleOperator
{
public:
    JZNodeSub();
};
    
//JZNodeMul
class JZNodeMul: public JZNodeDoubleOperator
{
public:
    JZNodeMul();
};

//JZNodeDiv
class JZNodeDiv: public JZNodeDoubleOperator
{
public:
    JZNodeDiv();
};

//JZNodeMod
class JZNodeMod: public JZNodeDoubleOperator
{
public:
    JZNodeMod();
};

//JZNodeBitAnd
class JZNodeBitAnd: public JZNodeDoubleOperator
{
public:
    JZNodeBitAnd();
};

//JZNodeBitAnd
class JZNodeBitOr: public JZNodeDoubleOperator
{
public:
    JZNodeBitOr();
};
    
//JZNodeBitXor
class JZNodeBitXor: public JZNodeDoubleOperator
{
public:
    JZNodeBitXor();
};

//JZNodeEQ
class JZNodeEQ : public JZNodeDoubleOperator
{
public:
    JZNodeEQ();
};

//JZNodeNE
class JZNodeNE : public JZNodeDoubleOperator
{
public:
    JZNodeNE();
};

//JZNodeLE
class JZNodeLE : public JZNodeDoubleOperator
{
public:
    JZNodeLE();
};

//JZNodeGE
class JZNodeGE : public JZNodeDoubleOperator
{
public:
    JZNodeGE();
};

//JZNodeLT
class JZNodeLT : public JZNodeDoubleOperator
{
public:
    JZNodeLT();
};

//JZNodeGT
class JZNodeGT : public JZNodeDoubleOperator
{
public:
    JZNodeGT();
};

//JZNodeAnd
class JZNodeAnd : public JZNodeDoubleOperator
{
public:
    JZNodeAnd();

    bool compiler(JZNodeCompiler *c, QString &error);
};

//JZNodeOr
class JZNodeOr : public JZNodeDoubleOperator
{
public:
    JZNodeOr();

    bool compiler(JZNodeCompiler *c, QString &error);
};


//JZNodeBitReverse
class JZNodeBitReverse : public JZNodeOperator
{
public:
    JZNodeBitReverse();

    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;
};

//JZNodeNot
class JZNodeNot : public JZNodeOperator
{
public:
    JZNodeNot();

    bool compiler(JZNodeCompiler *c, QString &error);
};

//JZNodeNeg
class JZNodeNeg : public JZNodeOperator
{
public:
    JZNodeNeg();

    bool compiler(JZNodeCompiler *c, QString &error);
};

#endif
