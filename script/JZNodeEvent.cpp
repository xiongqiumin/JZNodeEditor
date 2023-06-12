#include "JZNodeEvent.h"
#include "JZEvent.h"
#include "JZNodeCompiler.h"

// JZNodeEvent
JZNodeEvent::JZNodeEvent()
{
    m_type = Node_event;
    m_eventType = Event_none;

    addFlowOut();
}

JZNodeEvent::~JZNodeEvent()
{

}

void JZNodeEvent::saveToStream(QDataStream &s) const
{
    JZNode::saveToStream(s);
}

void JZNodeEvent::loadFromStream(QDataStream &s)
{
    JZNode::loadFromStream(s);
}

void JZNodeEvent::setEventType(int eventType)
{
    m_eventType = eventType;            
}

int JZNodeEvent::eventType() const
{
    return m_eventType;
}

bool JZNodeEvent::compiler(JZNodeCompiler *c,QString &error)
{
    c->addFlowOutput(m_id);
    c->addJumpNode(flowOut());
    return true;
}
