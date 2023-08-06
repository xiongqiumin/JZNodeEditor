#ifndef JZNODE_WIDGET_WRAPPER_H_
#define JZNODE_WIDGET_WRAPPER_H_

#include "JZEvent.h"
#include <QSet>

namespace jzbind
{

template<class Class>
class WidgetWrapper : public Class
{
public:
    using T = WidgetWrapper<Class>;

    void addEventFilter(void *ptr, int event)
    {
        object = (JZNodeObject*)ptr;
        eventFilter << event;
    }

    QSet<int> eventFilter;
    JZNodeObject *object;

    // Event_paint
    static void callPaintEventHelp(Class *ptr, QPaintEvent *event)
    {
        T* obj = (T*)ptr;
        obj->callPaintEvent(event);
    }

    void callPaintEvent(QPaintEvent *event)
    {
        Class::paintEvent(event);
    }

    virtual void paintEvent(QPaintEvent *e) override
    {
        if (!eventFilter.contains(Event_paint))
        {
            Class::paintEvent(e);
            return;
        }

        JZEvent event;
        event.sender = object;
        event.eventType = Event_paint;
        event.params.push_back(QVariant::fromValue(event.sender));
        event.params.push_back(getReturn(e, true));
        JZObjectEvent(&event);
    }

    // Event_show
    static void callShowEventHelp(Class *ptr, QShowEvent *event)
    {
        T* obj = (T*)ptr;
        obj->callShowEvent(event);
    }

    void callShowEvent(QShowEvent *event)
    {
        Class::showEvent(event);
    }

    virtual void showEvent(QShowEvent *e) override
    {
        if (!eventFilter.contains(Event_show))
        {
            Class::showEvent(e);
            return;
        }

        JZEvent event;
        event.sender = object;
        event.eventType = Event_show;
        event.params.push_back(QVariant::fromValue(event.sender));
        event.params.push_back(getReturn(e, true));
        JZObjectEvent(&event);
    }

    //Event_resize,
    static void callResizeEventHelp(Class *ptr, QResizeEvent *event)
    {
        T* obj = (T*)ptr;
        obj->callResizeEvent(event);
    }

    void callResizeEvent(QResizeEvent *event)
    {
        Class::resizeEvent(event);
    }

    virtual void resizeEvent(QResizeEvent *e) override
    {
        if (!eventFilter.contains(Event_resize))
        {
            Class::resizeEvent(e);
            return;
        }

        JZEvent event;
        event.sender = object;
        event.eventType = Event_resize;
        event.params.push_back(QVariant::fromValue(event.sender));
        event.params.push_back(getReturn(e, true));
        JZObjectEvent(&event);
    }
        
    //Event_close
    static void callCloseEventHelp(Class *ptr, QCloseEvent *event)
    {
        T* obj = (T*)ptr;
        obj->callCloseEvent(event);
    }

    void callCloseEvent(QCloseEvent *event)
    {
        Class::closeEvent(event);
    }

    virtual void closeEvent(QCloseEvent *e) override
    {
        if (!eventFilter.contains(Event_close))
        {
            Class::closeEvent(e);
            return;
        }

        JZEvent event;
        event.sender = object;
        event.eventType = Event_close;
        event.params.push_back(QVariant::fromValue(event.sender));
        event.params.push_back(getReturn(e, true));
        JZObjectEvent(&event);
    }

    //Event_KeyPress
    static void callKeyPressHelp(Class *ptr, QKeyEvent *event)
    {
        T* obj = (T*)ptr;
        obj->callKeyPress(event);
    }

    void callKeyPress(QKeyEvent *event)
    {
        Class::keyPressEvent(event);
    }

    virtual void keyPressEvent(QKeyEvent *e) override
    {
        if (!eventFilter.contains(Event_keyPress))
        {
            Class::keyPressEvent(e);
            return;
        }

        JZEvent event;
        event.sender = object;
        event.eventType = Event_keyPress;
        event.params.push_back(QVariant::fromValue(event.sender));
        event.params.push_back(getReturn(e, true));
        JZObjectEvent(&event);
    }

    //Event_KeyRelease
    static void callKeyReleaseHelp(Class *ptr, QKeyEvent *event)
    {
        T* obj = (T*)ptr;
        obj->callKeyRelease(event);
    }

    void callKeyRelease(QKeyEvent *event)
    {
        Class::keyReleaseEvent(event);
    }

    virtual void keyReleaseEvent(QKeyEvent *e) override
    {
        if (!eventFilter.contains(Event_keyRelease))
        {
            Class::keyReleaseEvent(e);
            return;
        }

        JZEvent event;
        event.sender = object;
        event.eventType = Event_keyRelease;
        event.params.push_back(QVariant::fromValue(event.sender));
        event.params.push_back(getReturn(e, true));
        JZObjectEvent(&event);
    }

    //Event_mousePress
    static void callMousePressHelp(Class *ptr, QMouseEvent *event)
    {
        T* obj = (T*)ptr;
        obj->callMousePress(event);
    }

    void callMousePress(QMouseEvent *event)
    {
        Class::mousePressEvent(event);
    }

    virtual void mousePressEvent(QMouseEvent *e) override
    {
        if (!eventFilter.contains(Event_mousePress))
        {
            Class::mousePressEvent(e);
            return;
        }

        JZEvent event;
        event.sender = object;
        event.eventType = Event_mousePress;
        event.params.push_back(QVariant::fromValue(event.sender));
        event.params.push_back(getReturn(e, true));
        JZObjectEvent(&event);
    }

    //Event_mouseMove
    static void callMouseMoveHelp(Class *ptr, QMouseEvent *event)
    {
        T* obj = (T*)ptr;
        obj->callMouseMove(event);
    }

    void callMouseMove(QMouseEvent *event)
    {
        Class::mouseMoveEvent(event);
    }

    virtual void mouseMoveEvent(QMouseEvent *e) override
    {
        if (!eventFilter.contains(Event_mouseMove))
        {
            Class::mouseMoveEvent(e);
            return;
        }

        JZEvent event;
        event.sender = object;
        event.eventType = Event_mouseMove;
        event.params.push_back(QVariant::fromValue(event.sender));
        event.params.push_back(getReturn(e, true));
        JZObjectEvent(&event);
    }

    //Event_mouseRelease
    static void callMouseReleaseHelp(Class *ptr, QMouseEvent *event)
    {
        T* obj = (T*)ptr;
        obj->callMouseReleaseEvent(event);
    }

    void callMouseReleaseEvent(QMouseEvent *event)
    {
        Class::mouseReleaseEvent(event);
    }

    virtual void mouseReleaseEvent(QMouseEvent *e) override
    {
        if (!eventFilter.contains(Event_mouseRelease))
        {
            Class::mouseReleaseEvent(e);
            return;
        }

        JZEvent event;
        event.sender = object;
        event.eventType = Event_mouseRelease;
        event.params.push_back(QVariant::fromValue(event.sender));
        event.params.push_back(getReturn(e, true));
        JZObjectEvent(&event);
    }    
};


}

#endif