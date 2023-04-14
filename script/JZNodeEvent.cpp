#include "JZNodeEvent.h"

// JZNodeEvent
JZNodeEvent::JZNodeEvent()
{
    m_type = Node_event;
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

bool JZNodeEvent::compiler(JZNodeCompiler *compiler,QString &error)
{
    return true;
}
