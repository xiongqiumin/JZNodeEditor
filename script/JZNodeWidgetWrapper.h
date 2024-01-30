#ifndef JZNODE_WIDGET_WRAPPER_H_
#define JZNODE_WIDGET_WRAPPER_H_

#include "JZEvent.h"
#include <QSet>

namespace jzbind
{

#define DEFINE_EVENT(type,func,qevent)    \
    void func(##qevent *e){               \
        if (!eventFilter.contains(type))  \
        {                                 \
            Class::##func(e);             \
            return;                       \
        }                                 \
                                          \
        JZEvent event;                                             \
        event.eventType = type;                                    \
        event.sender = jzobject;                                   \
        event.params.push_back(QVariant::fromValue(event.sender)); \
        event.params.push_back(getReturn(e, true));                \
        JZObjectEvent(&event);                                     \
    }                                                              \
    static void call_##func##_help(Class *ptr, ##qevent *event)    \
    {                                                              \
        T* obj = (T*)ptr;                                          \
        obj->call_##func(event);                                   \
    }                                                              \
    void call_##func(##qevent *event)                              \
    {                                                              \
        Class::##func(event);                                      \
    }                                                                   

template<class Class>
class WidgetWrapper : public Class
{
public:
    using T = WidgetWrapper<Class>;    

    WidgetWrapper() 
    {
        jzobject = nullptr;
    }

    DEFINE_EVENT(Event_paint, paintEvent, QPaintEvent)
    DEFINE_EVENT(Event_show, showEvent, QShowEvent)
    DEFINE_EVENT(Event_resize, resizeEvent, QResizeEvent)
    DEFINE_EVENT(Event_close, closeEvent, QCloseEvent)
    DEFINE_EVENT(Event_keyPress, keyPressEvent, QKeyEvent)
    DEFINE_EVENT(Event_keyRelease, keyReleaseEvent, QKeyEvent)
    DEFINE_EVENT(Event_mousePress, mousePressEvent, QMouseEvent)
    DEFINE_EVENT(Event_mouseMove, mouseMoveEvent, QMouseEvent)
    DEFINE_EVENT(Event_mouseRelease, mouseReleaseEvent, QMouseEvent)

    void setJZObject(JZNodeObject *obj)
    {
        jzobject = obj;
    }

    void addEventFilter(int event)
    {
        eventFilter.insert(event);
    }

private:
    JZNodeObject *jzobject;
    QSet<int> eventFilter;
};


}

#endif