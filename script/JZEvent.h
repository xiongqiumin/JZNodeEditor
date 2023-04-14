#ifndef JZEVENT_H_
#define JZEVENT_H_

#include <QEvent>
#include <QVariantList>

enum{
    Event_none,
    Event_paramChanged,
};

class JZEvent: public QEvent
{
public:
    static int Event;
    JZEvent();

    int eventType();
    void setEventType(int type);

    QVariantList params;

protected:
    int m_eventType;    
};


#endif
