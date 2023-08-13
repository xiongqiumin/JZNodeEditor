#include <QResizeEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QShowEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QMessageBox>
#include <QWidget>
#include <QLineEdit>
#include <QRadioButton>
#include <QCheckBox>
#include <QPushButton>
#include <QTimer>

#include "JZNodeQtWrapper.h"
#include "JZNodeObject.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeBind.h"

void checkSize(int index, int size)
{
    if (index < 0 || index >= size)
    {
        QString error = QString::asprintf("index %d out of range %d", index, size);
        throw std::runtime_error(qPrintable(error));
    }
}

//JZNodeListIterator
class JZNodeListIterator
{
public:
    void next() { it++; }
    bool atEnd() { return (it == list->end()); }
    QVariant key() { return (it - list->begin()); }
    QVariant value() { return *it; }

    QVariantList *list;
    QVariantList::iterator it;
};

//JZNodeMapIterator
class JZNodeMapIterator
{
public:
    void next() { it++; }
    bool atEnd() { return (it == map->end()); }
    QVariant key() { return it.value(); }
    QVariant value() { return it.key(); }

    QVariantMap *map;
    QVariantMap::iterator it;
};

void initBase()
{
    jzbind::ClassDelcare<QString>("string", Type_string);

    jzbind::ClassBind<QPoint> cls_pt("Point");
    cls_pt.def("create", false, [](int x, int y)->QPoint { return QPoint(x, y); });
    cls_pt.def("x", false, &QPoint::x);
    cls_pt.def("y", false, &QPoint::y);
    cls_pt.def("setX", true, &QPoint::setX);
    cls_pt.def("setY", true, &QPoint::setY);
    cls_pt.regist();

    jzbind::ClassBind<QPointF> cls_ptf("PointF");
    cls_ptf.def("create", false, [](double x, double y)->QPointF { return QPointF(x, y); });
    cls_ptf.def("x", false, &QPointF::x);
    cls_ptf.def("y", false, &QPointF::y);
    cls_ptf.def("setX", true, &QPointF::setX);
    cls_ptf.def("setY", true, &QPointF::setY);
    cls_ptf.regist();

    jzbind::ClassBind<QRect> cls_rect("Rect");
    cls_rect.def("create", false, [](int x, int y, int w, int h)->QRect {
        return QRect(x, y, w, h);
    });
    cls_rect.regist();

    jzbind::ClassBind<QRectF> cls_rectf("RectF");
    cls_rectf.def("create", false, [](double x, double y, double w, double h)->QRectF {
        return QRectF(x, y, w, h);
    });
    cls_rectf.regist();

    jzbind::ClassBind<QColor> cls_color("Color");
    cls_color.def("create", false, [](int r, int g, int b)->QColor {
        return QColor(r, g, b);
    });
    cls_color.def("red", false, &QColor::red);
    cls_color.def("green", false, &QColor::green);
    cls_color.def("blue", false, &QColor::blue);    
    cls_color.regist();    
}

void initCore()
{
    auto funcInst = JZNodeFunctionManager::instance();
    jzbind::ClassBind<QObject> cls_object("Object");
    cls_object.regist(Type_object);

    //stringlist
    jzbind::ClassBind<QStringList> cls_string_list("StringList");
    cls_string_list.def("join", false, [](const QStringList *list, const QString &sep)->QString {
        return (*list).join(sep);
    });
    cls_string_list.def("push_back", false, [](QStringList *list, const QString &str){
        list->push_back(str);
    });
    cls_string_list.def("set", false, [](QStringList *list, int index, const QString &str){
        checkSize(index, list->size());
        (*list)[index] = str;
    });
    cls_string_list.def("get", false, [](const QStringList *list,int index)->QString {
        checkSize(index, list->size());
        return (*list)[index];
    });
    cls_string_list.regist();

    //string È«²¿Ö»¶Á
    funcInst->registCFunction("string.append", false, jzbind::createFuncion([](const QString &a, const QString &b)->QString {
        return a + b;
    }));
    funcInst->registCFunction("string.left", false, jzbind::createFuncion([](const QString &a, int num)->QString {
        return a.left(num);
    }));
    funcInst->registCFunction("string.right", false, jzbind::createFuncion([](const QString &a, int num)->QString {
        return a.right(num);
    }));
    funcInst->registCFunction("string.size", false, jzbind::createFuncion([](const QString &a)->int {
        return a.size();
    }));
    funcInst->registCFunction("string.replace", false, jzbind::createFuncion([](const QString &in, const QString &before, const QString &after)->QString {
        QString ret = in;
        return ret.replace(before, after);
    }));

    //list
    jzbind::ClassBind<JZNodeListIterator> cls_list_it("ListIterator");
    cls_list_it.def("next", true, &JZNodeListIterator::next);
    cls_list_it.def("atEnd", true, &JZNodeListIterator::atEnd);
    cls_list_it.def("key", true, &JZNodeListIterator::key);
    cls_list_it.def("value", true, &JZNodeListIterator::value);
    cls_list_it.regist();

    jzbind::ClassBind<QVariantList> cls_list("List");
    cls_list.def("iterator", true, [](QVariantList *list)->QVariant {
        JZNodeObjectPtr it_ptr = JZObjectCreate<JZNodeListIterator>();
        auto list_it = (JZNodeListIterator*)it_ptr->cobj;
        list_it->it = list->begin();
        list_it->list = list;
        return QVariant::fromValue(it_ptr);
    });
    cls_list.def("resize", true, [](QVariantList *list, int size) {
        while (list->size() > size) {
            list->pop_back();
        }
        while (list->size() < size) {
            list->push_back(QVariant());
        }
    });
    cls_list.def("set", true, [](QVariantList *list, int index, const QVariant &value)
    {
        checkSize(index, list->size());
        (*list)[index] = value;
    });
    cls_list.def("get", false, [](QVariantList *list, int index)->QVariant {
        checkSize(index, list->size());
        return list->at(index);
    });
    cls_list.def("push_front", true, &QVariantList::push_front);
    cls_list.def("pop_front", true, [](QVariantList *list)
    {
        if (list->size() == 0)
            throw std::runtime_error("list is empty");
        list->pop_front();
    });
    cls_list.def("push_back", true, &QVariantList::push_back);
    cls_list.def("pop_back", true, [](QVariantList *list) {
        if (list->size() == 0)
            throw std::runtime_error("list is empty");
        list->pop_back();
    });
    cls_list.def("resize", true, [](QVariantList *list, int size) {
        while (list->size() > size)
            list->pop_back();
        while (list->size() < size)
            list->push_back(QVariant());
    });
    cls_list.def("size", false, &QVariantList::size);
    cls_list.def("clear", true, &QVariantList::clear);
    cls_list.regist(Type_list);

    //map
    jzbind::ClassBind<JZNodeMapIterator> map_it("MapIterator");
    map_it.def("next", true, &JZNodeListIterator::next);
    map_it.def("atEnd", true, &JZNodeListIterator::atEnd);
    map_it.def("key", true, &JZNodeListIterator::key);
    map_it.def("value", true, &JZNodeListIterator::value);
    map_it.regist();

    jzbind::ClassBind<QVariantMap> cls_map("Map");
    cls_map.def("iterator", true, [](QVariantMap *map)->QVariant {
        JZNodeObjectPtr it_ptr = JZObjectCreate<JZNodeMapIterator>();
        auto map_it = (JZNodeMapIterator*)it_ptr->cobj;
        map_it->it = map->begin();
        map_it->map = map;
        return QVariant::fromValue(it_ptr);
    });    
    cls_map.def("set", true, [](QVariantMap *map, const QString &key, const QVariant &value) { map->insert(key, value); });
    cls_map.def("get", false, [](QVariantMap *map, const QString &key)->QVariant {
        auto it = map->find(key);
        if (it == map->end())
        {
            QString error = "no such element, key = " + key;
            throw std::runtime_error(qPrintable(error));
        }

        return it.value();
    });
    cls_map.def("size", false, &QVariantMap::size);
    cls_map.def("clear", true, &QVariantMap::clear);
    cls_map.regist(Type_map);
}


void initEvent()
{
    jzbind::registEnum<Qt::KeyboardModifiers>(QString("KeypadModifier"));

    jzbind::ClassBind<QEvent> cls_event("Event");
    cls_event.regist();

    jzbind::ClassBind<QResizeEvent> cls_resize_event("ResizeEvent");
    cls_resize_event.regist();

    jzbind::ClassBind<QShowEvent> cls_show_event("ShowEvent");
    cls_show_event.regist();

    jzbind::ClassBind<QPaintEvent> cls_paint_event("PaintEvent");
    cls_paint_event.regist();

    jzbind::ClassBind<QCloseEvent> cls_close_event("CloseEvent");
    cls_close_event.regist();

    jzbind::ClassBind<QKeyEvent> cls_key_event("KeyEvent");
    cls_key_event.def("key", false, &QKeyEvent::key);
    cls_key_event.def("modifiers", false, &QKeyEvent::modifiers);
    cls_key_event.regist();

    jzbind::ClassBind<QMouseEvent> cls_mouse_event("MouseEvent");
    cls_mouse_event.def("pos", false, &QMouseEvent::pos);
    cls_mouse_event.def("x", false, &QMouseEvent::x);
    cls_mouse_event.def("y", false, &QMouseEvent::y);
    cls_mouse_event.regist();
}

void initObjects()
{
    jzbind::ClassBind<QTimer> cls_timer("Timer", "Object");
    cls_timer.def("start", true, QOverload<int>::of(&QTimer::start));
    cls_timer.def("stop", true, &QTimer::stop);
    cls_timer.def("isActive", false, &QTimer::isActive);
    cls_timer.defPrivateSingle("timeout", Event_timeout, &QTimer::timeout);
    cls_timer.regist(Type_timer);
}

void initWidgets()
{
    //widget
    jzbind::ClassBind<QWidget> cls_widget("Widget", "Object");
    cls_widget.def("setVisible", true, &QWidget::setVisible);
    cls_widget.def("show", true, &QWidget::show);
    cls_widget.def("hide", true, &QWidget::hide);
    cls_widget.def("close", true, &QWidget::close);
    cls_widget.def("rect", false, &QWidget::rect);
    cls_widget.def("update", true, QOverload<>::of(&QWidget::update));
    cls_widget.regist();

    //lineedit
    jzbind::ClassBind<QLineEdit> cls_lineEdit("LineEdit", "Widget");
    cls_lineEdit.def("text", false, &QLineEdit::text);
    cls_lineEdit.def("setText", true, &QLineEdit::setText);;
    cls_lineEdit.regist();

    //abs_button
    jzbind::ClassBind<QAbstractButton> cls_abs_button("AbstractButton", "Widget");
    cls_abs_button.def("text", false, &QAbstractButton::text);
    cls_abs_button.def("setText", true, &QAbstractButton::setText);
    cls_abs_button.def("isChecked", false, &QAbstractButton::isChecked);
    cls_abs_button.def("setChecked", true, &QAbstractButton::setChecked);
    cls_abs_button.defSingle("clicked", Event_buttonClicked, &QAbstractButton::clicked);
    cls_abs_button.regist();

    //button
    jzbind::ClassBind<QPushButton> cls_button("PushButton", "AbstractButton");
    cls_button.regist();

    //button
    jzbind::ClassBind<QCheckBox> cls_check_box("RadioButton", "AbstractButton");
    cls_check_box.regist();

    //painter
    jzbind::ClassBind<QPainter> cls_painter("Painter");
    cls_painter.def("create", true, [](QWidget *w)->QPainter* { return new QPainter(w); }, false);
    cls_painter.def("drawRect", true, QOverload<const QRect&>::of(&QPainter::drawRect));
    cls_painter.def("fillRect", true, QOverload<const QRect&, const QColor&>::of(&QPainter::fillRect));
    cls_painter.regist();

    JZNodeFunctionManager::instance()->registCFunction("MessageBox.information", true, jzbind::createFuncion([](QWidget *w, QString text) {
        QMessageBox::information(w, "", text);
    }));
}

void registQtClass()
{    
    initBase();
    initEvent();
    initCore();
    initObjects();
    initWidgets();
}