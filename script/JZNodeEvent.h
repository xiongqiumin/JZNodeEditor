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

    virtual QList<FunctionParam> params();

protected:    
    int m_eventType;
    QList<FunctionParam> m_params;
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
    virtual QList<FunctionParam> params() override;

    void setSingle(QString className,const SingleDefine *single);
    QString single();

    void setVariable(const QString &name);
    QString variable() const;
    virtual void drag(const QVariant &value) override;

protected:
    QString m_className;
    QString m_single;
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
