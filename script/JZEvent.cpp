#include "JZEvent.h"

int JZEvent::Event = QEvent::registerEventType(JZEvent::Event);

JZEvent::JZEvent()
    :QEvent((QEvent::Type)JZEvent::Event)
{    
    sender = nullptr;
}

JZEvent::~JZEvent()
{
}