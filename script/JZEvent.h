#ifndef JZEVENT_H_
#define JZEVENT_H_

#include <QEvent>
#include <QVariantList>
#include "JZNodeFunctionDefine.h"

enum {
    Event_none,
    Event_idlePause,
    Event_programStart,
    Event_libraryLoad,
    Event_paramChanged,

    Event_timeout,

    /* widget */
    Event_buttonClicked,
    Event_valueChanged,
    Event_comboxSelectChanged,

    Event_paint,
    Event_show,
    Event_resize,
    Event_close,
    Event_keyPress,
    Event_keyRelease,
    Event_mousePress,
    Event_mouseMove,
    Event_mouseRelease,    

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
    static bool isQtEvent(int event);

    static int Event;
    JZEvent();

    int eventType;
    JZNodeObject *sender;
    QVariantList params;    
};


#endif
