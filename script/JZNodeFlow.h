#ifndef JZNODE_FLOW_H_
#define JZNODE_FLOW_H_

#include "JZNode.h"

//JZNodeNop
class JZNodeNop : public JZNode
{
public:
    JZNodeNop();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;

protected:
};

//JZNodeContinue
class JZNodeContinue : public JZNode
{
public:
    JZNodeContinue();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:
};

//JZNodeBreak
class JZNodeBreak : public JZNode
{
public:
    JZNodeBreak();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:   
};

//JZNodeReturn
class JZNodeReturn : public JZNode
{
public:
    JZNodeReturn();
    
    virtual bool updateNode(QString &error) override;
    void setFunction(const JZFunctionDefine *def);
    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;    

protected:     

};

//JZNodeSequence
class JZNodeSequence : public JZNode
{
public:
    JZNodeSequence();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

    int addSequeue();
    void removeSequeue(int id);    

protected:
    void updateSeqName();
};

//JZNodeParallel
class JZNodeParallel : public JZNode
{
public:
    JZNodeParallel();    

protected:   
};

//JZNodeFor
class JZNodeFor: public JZNode
{
public:
    JZNodeFor();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

    void setRange(int start, int end);
    void setRange(int start, int step, int end);
    void setStart(int start);
    void setStep(int step);
    void setEnd(int end);

    JZNodeIRType op();
    void setOp(JZNodeIRType op);

protected:    
    virtual void saveToStream(QDataStream& s) const override;
    virtual void loadFromStream(QDataStream &s) override;

    QList<JZNodeIRType> m_condOpList;        
    JZNodeIRType m_condOp;
};

//JZNodeForEach
class JZNodeForEach: public JZNode
{
public:
    JZNodeForEach();
    ~JZNodeForEach();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:
    virtual bool updateNode(QString& error) override;
    virtual void onPinLinked(int pin_id);
    virtual void onPinUnlinked(int pin_id);

    bool getInputType(int &class_type, int& from_id, QString& error);
};

//JZNodeWhile
class JZNodeWhile: public JZNode
{
public:
    JZNodeWhile();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:

};

//JZNodeIf
class JZNodeIf : public JZNode
{
public:
    JZNodeIf();

    void addCondPin();    
    void removeCond(int index);
    int  condCount();

    bool hasElse();
    void addElsePin();
    void removeElse();

protected:
    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;    
    void updateCondName();
};

//JZNodeSwitch
class JZNodeSwitch : public JZNode
{
public:
    JZNodeSwitch();
    void addCase();
    void addDefault();
    void removeCase(int index);
    void removeDefault();
    bool hasDefault();
    int caseCount();
    void clearCaseAndDefault();

    void setCaseValue(int index, const QString &v);

protected:
    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;    
    QStringList m_caseType;
};


//JZNodeTryCatch
class JZNodeTryCatch : public JZNode
{
public:
    JZNodeTryCatch();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:

};


//JZNodeThrow
class JZNodeThrow : public JZNode
{
public:
    JZNodeThrow();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;

protected:

};


//JZNodeAssert
class JZNodeAssert : public JZNode
{
public:
    JZNodeAssert();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;

protected:

};

//JZNodeMainLoop
class JZNodeMainLoop : public JZNode
{
public:
    JZNodeMainLoop();
    virtual ~JZNodeMainLoop();
    
    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;

protected:

};

#endif