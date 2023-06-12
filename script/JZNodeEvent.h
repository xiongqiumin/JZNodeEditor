#ifndef JZNODE_EVENT_H_
#define JZNODE_EVENT_H_

#include "JZNode.h"

class JZNodeEvent : public JZNode
{
public:
    JZNodeEvent();
    ~JZNodeEvent();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error);
    virtual void saveToStream(QDataStream &s) const override;
    virtual void loadFromStream(QDataStream &s) override;

    void setEventType(int eventType);
    int eventType() const;

    int m_eventType;
    QString m_sender;
};

class JZNodeClickEvent : public JZNodeEvent
{
public:

};

#endif
