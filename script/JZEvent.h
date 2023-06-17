#ifndef JZEVENT_H_
#define JZEVENT_H_

#include <QEvent>
#include <QVariantList>
#include "JZNodeFunctionDefine.h"

enum{
    Event_none,
    Event_programStart,
    Event_libraryLoad,
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

class JZNodeObject;
class JZEvent: public QEvent
{
public:
    static int Event;
    JZEvent();

    int eventType;
    JZNodeObject *sender;
    QVariantList params;    
};


#endif
