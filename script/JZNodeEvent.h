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

    virtual bool compiler(JZNodeCompiler *compiler,QString &error);
    virtual void saveToStream(QDataStream &s) const override;
    virtual void loadFromStream(QDataStream &s) override;

    void setEventType(int eventType);
    int eventType() const;    

    virtual QList<JZParamDefine> params();

protected:    
    int m_eventType;
    QList<JZParamDefine> m_params;
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
    virtual QList<JZParamDefine> params() override;

    void setSingle(QString className,const SingleDefine *single);
    QString single();

    virtual void setVariable(const QString &name) override;
    virtual QString variable() const override;
    virtual int variableType() const;

    virtual void drag(const QVariant &value) override;

protected:
    QString m_className;
    QString m_single;
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
    virtual QList<JZParamDefine> params() override;

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

};

#endif
