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
    
    virtual void saveToStream(QDataStream &s) const override;
    virtual void loadFromStream(QDataStream &s) override;

    void setEventType(int eventType);
    int eventType() const;    

    virtual JZFunctionDefine function() = 0;

protected:    
    int m_eventType;    
};

//JZNodeStartEvent
class JZNodeStartEvent : public JZNodeEvent
{
public:
    JZNodeStartEvent();
    
    virtual JZFunctionDefine function() override;
    virtual bool compiler(JZNodeCompiler *compiler, QString &error);
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

//JZNodeSingleEvent
class JZNodeSingleEvent : public JZNodeEvent
{
public:
    JZNodeSingleEvent();
    virtual ~JZNodeSingleEvent();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error);
    virtual void saveToStream(QDataStream &s) const override;
    virtual void loadFromStream(QDataStream &s) override;
    virtual JZFunctionDefine function() override;

    void setSingle(const SingleDefine *single);
    QString single();

    void setVariable(const QString &name);
    QString variable() const;

    virtual void drag(const QVariant &value) override;

protected:    
    QString m_single;
};

//JZNodeSingleConnect
class JZNodeSingleConnect : public JZNode
{
public:
    JZNodeSingleConnect();
    virtual ~JZNodeSingleConnect();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error);
};

//JZNodeQtEvent
class JZNodeQtEvent : public JZNodeEvent
{
public:
    JZNodeQtEvent();
    virtual ~JZNodeQtEvent();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error);
    virtual void saveToStream(QDataStream &s) const override;
    virtual void loadFromStream(QDataStream &s) override;
    virtual JZFunctionDefine function() override;

    void setEvent(QString className, const EventDefine *func);
    QString event();

protected:
    QString m_className;
    QString m_event;
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
