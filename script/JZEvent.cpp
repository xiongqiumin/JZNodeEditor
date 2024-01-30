#include "JZEvent.h"

int JZEvent::Event = registerEventType(JZEvent::Event);

bool JZEvent::isQtEvent(int event_type)
{
    return (event_type >= Event_paint && event_type <= Event_mouseRelease);
}

bool JZEvent::isSingleEvent(int event_type)
{
    return (event_type > Event_single_start);
}

JZEvent::JZEvent()
    :QEvent((QEvent::Type)JZEvent::Event)
{    
    sender = nullptr;
}

JZEvent::~JZEvent()
{
}