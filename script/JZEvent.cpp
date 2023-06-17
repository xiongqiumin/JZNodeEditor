#include "JZEvent.h"

int JZEvent::Event = registerEventType(JZEvent::Event);

JZEvent::JZEvent()
    :QEvent((QEvent::Type)JZEvent::Event)
{    
    sender = nullptr;
}
