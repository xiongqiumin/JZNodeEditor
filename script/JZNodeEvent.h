#ifndef JZNODE_EVENT_H_
#define JZNODE_EVENT_H_

#include "JZNode.h"
#include "JZNodeObject.h"

//JZNodeSignalConnect
class JZNodeSignalConnect : public JZNode
{
public:
    JZNodeSignalConnect();
    virtual ~JZNodeSignalConnect();

    virtual bool compiler(JZNodeCompiler* compiler, QString& error);
};

//JZNodeSignalDisconnect
class JZNodeSignalDisconnect : public JZNode
{
public:
    JZNodeSignalDisconnect();
    virtual ~JZNodeSignalDisconnect();

    virtual bool compiler(JZNodeCompiler* compiler, QString& error);
};

//JZNodeEvent
class JZNodeEvent : public JZNode
{
public:
    JZNodeEvent();
    virtual ~JZNodeEvent();
    
    virtual JZFunctionDefine function() = 0;

protected:
    const JZNodeObjectDefine *classMeta();
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

//JZNodeShowEvent
class JZNodeShowEvent : public JZNodeEvent
{
public:
    virtual JZFunctionDefine function() override;
    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;
};

//JZNodeCloseEvent
class JZNodeCloseEvent : public JZNodeEvent
{
public:
    virtual JZFunctionDefine function() override;
    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;
};

//JZNodeResizeEvent
class JZNodeResizeEvent : public JZNodeEvent
{
public:
    virtual JZFunctionDefine function() override;
    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;
};

//JZNodePaintEvent
class JZNodePaintEvent : public JZNodeEvent
{
public:
    virtual JZFunctionDefine function() override;
    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;
};


//JZNodeMousePressEvent
class JZNodeMousePressEvent : public JZNodeEvent
{
public:
    virtual JZFunctionDefine function() override;
    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;
};

//JZNodeMouseReleaseEvent
class JZNodeMouseReleaseEvent : public JZNodeEvent
{
public:
    virtual JZFunctionDefine function() override;
    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;
};

//JZNodeMouseMoveEvent
class JZNodeMouseMoveEvent : public JZNodeEvent
{
public:
    virtual JZFunctionDefine function() override;
    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;
};

//JZNodeKeyPressEvent
class JZNodeKeyPressEvent : public JZNodeEvent
{
public:
    virtual JZFunctionDefine function() override;
    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;
};

//JZNodeButtonClickedEvent
class JZNodeButtonClickedEvent : public JZNodeEvent
{
public:
    JZNodeButtonClickedEvent();
    virtual ~JZNodeButtonClickedEvent();

    void setObject(QString name);
    QString object();

    virtual JZFunctionDefine function() override;
    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;

protected:
    QString m_object;
};

//JZNodeTimerEvent
class JZNodeTimerEvent : public JZNodeEvent
{
public:
    JZNodeTimerEvent();
    virtual ~JZNodeTimerEvent();

    void setTimeOut(int ms);
    int timeOut();

    virtual JZFunctionDefine function() override;
    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;

protected:
    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);

    int m_timeout;
};


void JZNodeEventFunctionInit(JZScriptEnvironment *env);

#endif
