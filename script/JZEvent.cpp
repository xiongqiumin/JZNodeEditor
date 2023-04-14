#include "JZEvent.h"

int JZEvent::Event = registerEventType(JZEvent::Event);

JZEvent::JZEvent()
    :QEvent((QEvent::Type)JZEvent::Event)
{

}

int JZEvent::eventType()
{
    return m_eventType;        
}

void JZEvent::setEventType(int type)
{
    m_eventType = type;
}
