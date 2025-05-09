#ifndef JZNODE_EVENT_H_
#define JZNODE_EVENT_H_

#include <QJsonObject>
#include "JZNode.h"
#include "JZNodeObject.h"

struct SignalConnectInfo
{
    QString connectFunction;        
    QList<JZNodeIRParam> irList;
    QJsonObject param;
};

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
    JZNodeShowEvent();

    virtual JZFunctionDefine function() override;
    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;
};

//JZNodeCloseEvent
class JZNodeCloseEvent : public JZNodeEvent
{
public:
    JZNodeCloseEvent();

    virtual JZFunctionDefine function() override;
    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;
};

//JZNodeResizeEvent
class JZNodeResizeEvent : public JZNodeEvent
{
public:
    JZNodeResizeEvent();

    virtual JZFunctionDefine function() override;
    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;
};

//JZNodePaintEvent
class JZNodePaintEvent : public JZNodeEvent
{
public:
    JZNodePaintEvent();

    virtual JZFunctionDefine function() override;
    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;
};


//JZNodeMousePressEvent
class JZNodeMousePressEvent : public JZNodeEvent
{
public:
    JZNodeMousePressEvent();

    virtual JZFunctionDefine function() override;
    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;
};

//JZNodeMouseReleaseEvent
class JZNodeMouseReleaseEvent : public JZNodeEvent
{
public:
    JZNodeMouseReleaseEvent();

    virtual JZFunctionDefine function() override;
    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;
};

//JZNodeMouseMoveEvent
class JZNodeMouseMoveEvent : public JZNodeEvent
{
public:
    JZNodeMouseMoveEvent();

    virtual JZFunctionDefine function() override;
    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;
};

//JZNodeKeyPressEvent
class JZNodeKeyPressEvent : public JZNodeEvent
{
public:
    JZNodeKeyPressEvent();

    virtual JZFunctionDefine function() override;
    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;
};

//JZNodeKeyReleaseEvent
class JZNodeKeyReleaseEvent : public JZNodeEvent
{
public:
    JZNodeKeyReleaseEvent();

    virtual JZFunctionDefine function() override;
    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;
};

//JZNodeSignalEvent
class JZNodeSignalEvent : public JZNodeEvent
{
public:
    virtual JZFunctionDefine function() override;
    bool compilerSignal(JZNodeCompiler* compiler, QString& error);

protected:    
    SignalConnectInfo m_connectInfo;
};

//JZNodeButtonClickedEvent
class JZNodeButtonClickedEvent : public JZNodeSignalEvent
{
public:
    JZNodeButtonClickedEvent();
    virtual ~JZNodeButtonClickedEvent();

    void setObject(QString name);
    QString object();

    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;
};

//JZNodeTimerEvent
class JZNodeTimerEvent : public JZNodeSignalEvent
{
public:
    JZNodeTimerEvent();
    virtual ~JZNodeTimerEvent();

    void setTimeOut(int ms);
    int timeOut();
    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;

protected:
    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);

    int m_timeout;
};


void JZNodeEventFunctionInit(JZScriptEnvironment *env);

#endif
