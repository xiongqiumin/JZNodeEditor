#include "JZEvent.h"

int JZEvent::Event = registerEventType(JZEvent::Event);

bool JZEvent::isQtEvent(int event_type)
{
    return (event_type >= Event_paint && event_type <= Event_mouseRelease);
}

JZEvent::JZEvent()
    :QEvent((QEvent::Type)JZEvent::Event)
{    
    sender = nullptr;
}
