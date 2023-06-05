#ifndef JZEVENT_H_
#define JZEVENT_H_

#include <QEvent>
#include <QVariantList>

enum{
    Event_none,
    Event_programStart,
    Event_paramChanged,
    
    /* widget */
    Event_buttonClicked,
    Event_valueChanged,
    Event_comboxSelectChanged,

    /* widget item */    
    Event_itemSelected,
    Event_itemClicked,
    Event_itemDoubleClicked,
    Event_itemChanged,
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
