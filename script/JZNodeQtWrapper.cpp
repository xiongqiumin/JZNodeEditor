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
#include <QDialogButtonBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QFileInfo>
#include <QDir>

#include "JZNodeQtWrapper.h"
#include "JZNodeObject.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeBind.h"
#include "JZRegExpHelp.h"

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

void initEnum()
{
    jzbind::registEnum<Qt::KeyboardModifiers>("KeyboardModifiers");
    jzbind::registEnum<Qt::SplitBehavior>("SplitBehavior");
    jzbind::registEnum<Qt::PenStyle>("PenStyle");
    jzbind::registEnum<Qt::Orientation>("Orientation");
    jzbind::registEnum<Qt::SortOrder>("SortOrder");
    jzbind::registEnum<Qt::Alignment>("Alignment");
    jzbind::registEnum<Qt::GlobalColor>("GlobalColor");

    JZNodeEnumDefine keyCode;
    QStringList key_codes;
    QVector<int> key_values;
    for (int i = 0; i < 10; i++)
    {
        key_codes.push_back(QChar('0' + i));
        key_values.push_back('0' + i);
    }
    for (int i = 0; i < 26; i++)
    {
        key_codes.push_back(QChar('A' + i));
        key_values.push_back('A' + i);
    }
    keyCode.init("Key", key_codes, key_values);
    JZNodeObjectManager::instance()->registEnum(keyCode);
}

void initBase()
{
    jzbind::ClassBind<QPoint> cls_pt("Point");
    cls_pt.def("create", false, [](int x, int y)->QPoint { return QPoint(x, y); });
    cls_pt.def("fromString", false, [](QPoint *pt,const QString &text){
        bool ok1, ok2;
        QStringList list = text.split(",");
        if (list.size() != 2)
            return;

        int x = list[0].toInt(&ok1);
        if (!ok1)
            return;
        int y = list[1].toInt(&ok2);
        if (!ok2)
            return;

        pt->setX(x);
        pt->setY(y);
    });
    cls_pt.def("toString", false, [](QPoint *pt)->QString{ 
        return QString::number(pt->x()) + "," + QString::number(pt->y());
    });
    cls_pt.def("x", false, &QPoint::x);
    cls_pt.def("y", false, &QPoint::y);
    cls_pt.def("setX", true, &QPoint::setX);
    cls_pt.def("setY", true, &QPoint::setY);
    cls_pt.regist();

    jzbind::ClassBind<QPointF> cls_ptf("PointF");
    cls_ptf.def("create", false, [](double x, double y)->QPointF { return QPointF(x, y); });
    cls_ptf.def("fromString", false, [](QPointF *pt, const QString &text) {
        bool ok1, ok2;
        QStringList list = text.split(",");
        if (list.size() != 2)
            return;

        double x = list[0].toDouble(&ok1);
        if (!ok1)
            return;
        double y = list[1].toDouble(&ok2);
        if (!ok2)
            return;

        pt->setX(x);
        pt->setY(y);
    });
    cls_ptf.def("toString", false, [](QPointF *pt)->QString {
        return QString::number(pt->x()) + "," + QString::number(pt->y());
    });
    cls_ptf.def("x", false, &QPointF::x);
    cls_ptf.def("y", false, &QPointF::y);
    cls_ptf.def("setX", true, &QPointF::setX);
    cls_ptf.def("setY", true, &QPointF::setY);
    cls_ptf.regist();

    jzbind::ClassBind<QRect> cls_rect("Rect");
    cls_rect.def("create", false, [](int x, int y, int w, int h)->QRect {
        return QRect(x, y, w, h);
    });
    cls_rect.def("createFromString", false, [](const QString &text)->QRect { return QRect(); });
    cls_rect.regist();

    jzbind::ClassBind<QRectF> cls_rectf("RectF");
    cls_rectf.def("create", false, [](double x, double y, double w, double h)->QRectF {
        return QRectF(x, y, w, h);
    });
    cls_rectf.def("createFromString", false, [](const QString &text)->QRectF { return QRectF(); });
    cls_rectf.regist();

    jzbind::ClassBind<QColor> cls_color("Color");
    cls_color.def("create", false, [](int r, int g, int b)->QColor {
        return QColor(r, g, b);
    });
    cls_color.def("createFromString", false, [](const QString &text)->QColor { return QColor(); });
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
    cls_string_list.def("createFromString", false, [](const QString &text)->QStringList {
        QStringList list = text.split(",");
        return list;
    });
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
    jzbind::ClassBind<QString> cls_string("String");
    cls_string.def("append", false, [](const QString &a, const QString &b)->QString {
        return a + b;
    });
    cls_string.def("left", false, [](const QString &a, int num)->QString {
        return a.left(num);
    });
    cls_string.def("right", false, [](const QString &a, int num)->QString {
        return a.right(num);
    });
    cls_string.def("size", false, [](const QString &a)->int {
        return a.size();
    });
    cls_string.def("replace", false, [](const QString &in, const QString &before, const QString &after)->QString {
        QString ret = in;
        return ret.replace(before, after);
    });
    cls_string.def("split", false, [](const QString &in, const QString &before, Qt::SplitBehavior behavior)->QStringList {        
        return in.split(before, behavior);
    });
    cls_string.regist();

    //list
    jzbind::ClassBind<JZNodeListIterator> cls_list_it("ListIterator");
    cls_list_it.def("next", true, &JZNodeListIterator::next);
    cls_list_it.def("atEnd", true, &JZNodeListIterator::atEnd);
    cls_list_it.def("key", true, &JZNodeListIterator::key);
    cls_list_it.def("value", true, &JZNodeListIterator::value);
    cls_list_it.regist();

    jzbind::ClassBind<QVariantList> cls_list("List");
    cls_list.def("createFromString", false, [](const QString &text)->QVariantList {
        QVariantList ret;
        QStringList list = text.split(",");
        for (int i = 0; i < list.size(); i++)
        {
            if (JZRegExpHelp::isInt(list[i]))
                ret.push_back(list[i].toInt());
            else if (JZRegExpHelp::isHex(list[i]))
                ret.push_back(list[i].toInt(nullptr, 16));
            else if (JZRegExpHelp::isFloat(list[i]))
                ret.push_back(list[i].toDouble());
            else
                ret.push_back(list[i]);
        }
        return ret;
    });
    cls_list.def("iterator", true, [](QVariantList *list)->JZNodeListIterator* {        
        JZNodeListIterator *list_it = new JZNodeListIterator();
        list_it->it = list->begin();
        list_it->list = list;
        return list_it;
    }, true);
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
    cls_map.def("createFromString", false, [](const QString &text)->QVariantMap {
        QVariantMap ret;
        QStringList list = text.split(",");
        for (int i = 0; i < list.size(); i++)
        {        
        }
        return ret;
    });
    cls_map.def("iterator", true, [](QVariantMap *map)->JZNodeMapIterator*{
        JZNodeMapIterator *map_it = new JZNodeMapIterator();        
        map_it->it = map->begin();
        map_it->map = map;
        return map_it;
    },true);    
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
    cls_timer.def("start",true, QOverload<int>::of(&QTimer::start));
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
    cls_widget.regist(Type_widget);

    //lineedit
    jzbind::ClassBind<QLineEdit> cls_lineEdit("LineEdit", "Widget");
    cls_lineEdit.def("text", false, &QLineEdit::text);
    cls_lineEdit.def("setText", true, &QLineEdit::setText);;
    cls_lineEdit.regist();

    //textedit
    jzbind::ClassBind<QTextEdit> cls_textEdit("QTextEdit", "Widget");
    cls_textEdit.def("toPlainText", false, &QTextEdit::toPlainText);
    cls_textEdit.def("setText", true, &QTextEdit::setText);
    cls_textEdit.def("append", true, &QTextEdit::append);
    cls_textEdit.regist();

    //abs_button
    jzbind::ClassBind<QAbstractButton> cls_abs_button("Button", "Widget");
    cls_abs_button.def("text", false, &QAbstractButton::text);
    cls_abs_button.def("setText", true, &QAbstractButton::setText);
    cls_abs_button.def("isChecked", false, &QAbstractButton::isChecked);
    cls_abs_button.def("setChecked", true, &QAbstractButton::setChecked);
    cls_abs_button.defSingle("clicked", Event_buttonClicked, &QAbstractButton::clicked);
    cls_abs_button.regist();
    
    jzbind::ClassBind<QPushButton> cls_button("PushButton", "Button");
    cls_button.regist();
    
    jzbind::ClassBind<QRadioButton> cls_radio_btn("RadioButton", "Button");
    cls_radio_btn.regist();

    jzbind::ClassBind<QCheckBox> cls_check_box("CheckBox", "Button");
    cls_check_box.regist();
    
    jzbind::ClassBind<QMessageBox> cls_msg_box("MessageBox", "Widget");
    cls_msg_box.def("information", true, [](QWidget *w, QString text) {
        QMessageBox::information(w, "", text);
    });
    cls_msg_box.regist();
}

void initPainter()
{
    //pen
    jzbind::ClassBind<QPen> cls_pen("Pen");
    cls_pen.def("create", false, [](QColor c,int width,Qt::PenStyle style)->QPen*{ return new QPen(c, width, style); }, false);
    cls_pen.regist();

    //brush
    jzbind::ClassBind<QBrush> cls_brush("Brush");
    cls_brush.def("create", false, [](QColor c)->QBrush* { return new QBrush(c); }, false);
    cls_brush.regist();

    //painter
    jzbind::ClassBind<QPainter> cls_painter("Painter");
    cls_painter.def("create", true, [](QWidget *w)->QPainter* { return new QPainter(w); }, false);
    cls_painter.def("drawRect", true, QOverload<const QRect&>::of(&QPainter::drawRect));
    cls_painter.def("fillRect", true, QOverload<const QRect&, const QColor&>::of(&QPainter::fillRect));
    cls_painter.regist();
}

void initFiles()
{
    jzbind::ClassBind<QFile> cls_file("File");
    cls_file.regist();

    jzbind::ClassBind<QFileInfo> cls_fileInfo("FileInfo");
    cls_fileInfo.regist();

    jzbind::ClassBind<QDir> cls_dir("Dir");
    cls_dir.regist();
}

QVariant colorEnum_to_color(const QVariant &v)
{
    QColor *color = new QColor((Qt::GlobalColor)v.toInt());
    auto ptr = JZObjectRefrence(color);
    return QVariant::fromValue(ptr);
}

QVariant colorEnum_to_brush(const QVariant &v)
{
    QBrush *brush = new QBrush((Qt::GlobalColor)v.toInt());
    auto ptr = JZObjectRefrence(brush);
    return QVariant::fromValue(ptr);
}

QVariant color_to_brush(const QVariant &v)
{    
    QColor *c = jzbind::getValue<QColor*>(v);
    QBrush *brush = new QBrush(*c);
    auto ptr = JZObjectRefrence(brush);
    return QVariant::fromValue(ptr);
}

void registConvert()
{
    auto inst = JZNodeObjectManager::instance();
    int from_id = inst->getEnumId("GlobalColor");
    int to_id = inst->getClassId("Brush");

    JZNodeType::registConvert(from_id, to_id, colorEnum_to_brush);
    to_id = inst->getClassId("Color");
    JZNodeType::registConvert(from_id, to_id, colorEnum_to_color);

    from_id = inst->getClassId("Color");
    to_id = inst->getClassId("Brush");
    JZNodeType::registConvert(from_id, to_id, color_to_brush);
}

void registQtClass()
{    
    initEnum();
    initBase();
    initEvent();
    initCore();
    initObjects();
    initWidgets();
    initPainter();
    initFiles();
    registConvert();
}