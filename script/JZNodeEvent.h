#ifndef JZNODE_EVENT_H_
#define JZNODE_EVENT_H_

#include "JZNode.h"
#include "JZEvent.h"

//JZNodeEvent
class JZNodeEvent : public JZNode
{
public:
    JZNodeEvent();
    virtual ~JZNodeEvent();
    
    virtual JZFunctionDefine function() = 0;
};


//JZNodeFunctionStart
class JZNodeFunctionStart : public JZNodeEvent
{
public:
    JZNodeFunctionStart();
    virtual ~JZNodeFunctionStart();

    virtual JZFunctionDefine function() override;
    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;

protected:

};

//JZNodeSignalConnect
class JZNodeSignalConnect : public JZNode
{
public:
    JZNodeSignalConnect();
    virtual ~JZNodeSignalConnect();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error);
};

//JZNodeSignalDisconnect
class JZNodeSignalDisconnect : public JZNode
{
public:
    JZNodeSignalDisconnect();
    virtual ~JZNodeSignalDisconnect();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error);
};

//JZNodeParamChangedEvent
class JZNodeParamChangedEvent : public JZNodeEvent
{
public:
    JZNodeParamChangedEvent();
    virtual ~JZNodeParamChangedEvent();

    virtual void setVariable(const QString &name);
    virtual QString variable() const;

    virtual JZFunctionDefine function() override;
    virtual bool compiler(JZNodeCompiler *compiler, QString &error);
};

#endif
