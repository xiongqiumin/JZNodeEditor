#ifndef JZEVENT_H_
#define JZEVENT_H_

#include <QEvent>
#include <QVariantList>
#include "JZNodeFunctionDefine.h"

enum {
    Event_none,
    Event_idlePause,
    Event_programStart,
    Event_functionStart,
    Event_libraryLoad,
    Event_paramChanged,
    
    Event_paint,
    Event_show,
    Event_resize,
    Event_close,
    Event_keyPress,
    Event_keyRelease,
    Event_mousePress,
    Event_mouseMove,
    Event_mouseRelease,    

    Event_single_start,
    /* widget */
    Event_buttonClicked,
    Event_valueChanged,
    Event_comboxSelectChanged,    

    /* widget item */    
    Event_itemSelected,
    Event_itemClicked,
    Event_itemDoubleClicked,
    Event_itemChanged,

    Event_timeout,
};

class JZNodeObject;
class JZEvent: public QEvent
{
public:
    static bool isQtEvent(int event);
    static bool isSingleEvent(int event);

    static int Event;
    JZEvent();
    virtual ~JZEvent();
    
    int eventType;    
    JZNodeObject *sender;
    QVariantList params;    
};


#endif
