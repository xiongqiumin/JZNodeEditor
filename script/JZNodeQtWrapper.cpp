﻿#include <QResizeEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QShowEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QMessageBox>
#include <QWidget>
#include <QLineEdit>
#include <QRadioButton>
#include <QToolButton>
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
#include <QFileDialog>
#include <QProgressDialog>
#include <QApplication>
#include <QStackedWidget>

#include "JZNodeQtWrapper.h"
#include "JZNodeObject.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeBind.h"
#include "JZRegExpHelp.h"

typedef QList<JZNodeVariantAny>         JZVariantList;
typedef QMap<QString, JZNodeVariantAny> JZVariantMap;

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
    int key() { return (it - list->begin()); }
    JZNodeVariantAny value() { return *it; }

    JZVariantList *list;
    JZVariantList::iterator it;
};

//JZNodeMapIterator
class JZNodeMapIterator
{
public:
    void next() { it++; }
    bool atEnd() { return (it == map->end()); }
    QString key() { return it.key(); }
    JZNodeVariantAny value() { return it.value(); }

    JZVariantMap *map;
    JZVariantMap::iterator it;
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

    jzbind::registEnum<Qt::Key>("Key", Type_keyCode);
    int file_dlg_option = jzbind::registEnum<QFileDialog::Option>("FileDialog.Option");
    int file_dlg_options = jzbind::registEnum<QFileDialog::Options>("FileDialog.Options");
    JZNodeObjectManager::instance()->enumMeta(file_dlg_options)->setFlag(true, file_dlg_option);
        
    QStringList filter_key_list;
    QVector<int> filter_value_list;
    JZNodeEnumDefine filter_define; 
    filter_key_list << "Dirs";
    filter_key_list << "Files";
    filter_key_list << "NoDotAndDotDot";
    filter_value_list << QDir::Dirs;
    filter_value_list << QDir::Files;
    filter_value_list << QDir::NoDotAndDotDot;
    filter_define.init("Dir::Filter", filter_key_list, filter_value_list);
    int filter_id = JZNodeObjectManager::instance()->registCEnum(filter_define,typeid(QDir::Filter).name());
    
    filter_define.init("Dir::Filters", filter_key_list, filter_value_list);
    filter_define.setFlag(true, filter_id);
    JZNodeObjectManager::instance()->registCEnum(filter_define,typeid(QDir::Filters).name());    
}

void initBase()
{
    JZNodeObjectManager::instance()->delcare("StringList",typeid(QStringList).name() ,Type_stringList);

    //string 全部只读
    jzbind::ClassBind<QString> cls_string(Type_string,"string");
    cls_string.def("append", false, [](const QString &a, const QString &b)->QString {
        return a + b;
    });
    cls_string.def("left", false, &QString::left);
    cls_string.def("right", false, &QString::right);
    cls_string.def("size", false, &QString::size);
    cls_string.def("mid", false, &QString::mid);
    cls_string.def("replace", false, [](const QString &in, const QString &before, const QString &after)->QString {
        QString ret = in;
        return ret.replace(before, after);
    });
    cls_string.def("split", false, [](const QString &in, const QString &before, Qt::SplitBehavior behavior)->QStringList {
        return in.split(before, behavior);
    });
    cls_string.def("isEmpty", false, &QString::isEmpty);
    cls_string.regist();

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
    jzbind::ClassBind<QObject> cls_object(Type_object,"Object");
    cls_object.regist();    
    
    //app
    jzbind::ClassBind<QApplication> cls_app("Application");    
    cls_app.def("setStyleSheet", true, [](const QString &style) { qApp->setStyleSheet(style); });
    cls_app.regist();

    //stringlist
    jzbind::ClassBind<QStringList> cls_string_list(Type_stringList,"StringList");
    cls_string_list.def("create", false, []()->QStringList {        
        return QStringList();
    });
    cls_string_list.def("createFromString", false, [](const QString &text)->QStringList {
        QStringList list = text.split(",");
        return list;
    });
    cls_string_list.def("join", false, [](const QStringList *list, const QString &sep)->QString {
        return (*list).join(sep);
    });
    cls_string_list.def("push_back", true, [](QStringList *list, const QString &str){
        list->push_back(str);
    });
    cls_string_list.def("set", true, [](QStringList *list, int index, const QString &str){
        checkSize(index, list->size());
        (*list)[index] = str;
    });
    cls_string_list.def("get", false, [](const QStringList *list,int index)->QString {
        checkSize(index, list->size());
        return (*list)[index];
    });
    cls_string_list.def("size", false, [](const QStringList *list)->int { return list->size(); });
    cls_string_list.regist();    

    //list
    jzbind::ClassBind<JZNodeListIterator> cls_list_it("ListIterator");
    cls_list_it.def("next", true, &JZNodeListIterator::next);
    cls_list_it.def("atEnd", true, &JZNodeListIterator::atEnd);
    cls_list_it.def("key", true, &JZNodeListIterator::key);
    cls_list_it.def("value", true, &JZNodeListIterator::value);
    cls_list_it.regist();

    jzbind::ClassBind<JZVariantList> cls_list(Type_list,"List");
    cls_list.def("createFromString", false, [](const QString &text)->JZVariantList {
        JZVariantList ret;
        QStringList list = text.split(",");
        for (int i = 0; i < list.size(); i++)
        {
            JZNodeVariantAny var;
            if (JZRegExpHelp::isInt(list[i]))
                var.value = list[i].toInt();
            else if (JZRegExpHelp::isHex(list[i]))
                var.value = list[i].toInt(nullptr, 16);
            else if (JZRegExpHelp::isFloat(list[i]))
                var.value = list[i].toDouble();
            else
                var.value = list[i];
            ret.push_back(var);
        }
        return ret;
    });
    cls_list.def("iterator", true, [](JZVariantList *list)->JZNodeListIterator* {        
        JZNodeListIterator *list_it = new JZNodeListIterator();
        list_it->it = list->begin();
        list_it->list = list;
        return list_it;
    }, true);
    cls_list.def("set", true, [](JZVariantList *list, int index, const JZNodeVariantAny &value)
    {
        checkSize(index, list->size());
        (*list)[index] = value;
    });
    cls_list.def("get", false, [](JZVariantList *list, int index)->JZNodeVariantAny {
        checkSize(index, list->size());
        return list->at(index);
    });
    cls_list.def("push_front", true, &JZVariantList::push_front);
    cls_list.def("pop_front", true, [](JZVariantList *list)
    {
        if (list->size() == 0)
            throw std::runtime_error("list is empty");
        list->pop_front();
    });
    cls_list.def("push_back", true, &JZVariantList::push_back);
    cls_list.def("pop_back", true, [](JZVariantList *list) {
        if (list->size() == 0)
            throw std::runtime_error("list is empty");
        list->pop_back();
    });
    cls_list.def("resize", true, [](JZVariantList *list, int size) {
        while (list->size() > size)
            list->pop_back();
        while (list->size() < size)
            list->push_back(JZNodeVariantAny());
    });
    cls_list.def("size", false, &JZVariantList::size);
    cls_list.def("clear", true, &JZVariantList::clear);
    cls_list.regist();

    //map
    jzbind::ClassBind<JZNodeMapIterator> map_it(Type_map,"MapIterator");
    map_it.def("next", true, &JZNodeListIterator::next);
    map_it.def("atEnd", true, &JZNodeListIterator::atEnd);
    map_it.def("key", true, &JZNodeListIterator::key);
    map_it.def("value", true, &JZNodeListIterator::value);
    map_it.regist();

    jzbind::ClassBind<JZVariantMap> cls_map("Map");
    cls_map.def("createFromString", false, [](const QString &text)->JZVariantMap {
        JZVariantMap ret;
        QStringList list = text.split(",");
        for (int i = 0; i < list.size(); i++)
        {        
        }
        return ret;
    });
    cls_map.def("iterator", true, [](JZVariantMap *map)->JZNodeMapIterator*{
        JZNodeMapIterator *map_it = new JZNodeMapIterator();        
        map_it->it = map->begin();
        map_it->map = map;
        return map_it;
    },true);    
    cls_map.def("set", true, [](JZVariantMap *map, const QString &key, const JZNodeVariantAny &value) { map->insert(key, value); });
    cls_map.def("get", false, [](JZVariantMap *map, const QString &key)->JZNodeVariantAny {
        auto it = map->find(key);
        if (it == map->end())
        {
            QString error = "no such element, key = " + key;
            throw std::runtime_error(qPrintable(error));
        }

        return it.value();
    });
    cls_map.def("size", false, &JZVariantMap::size);
    cls_map.def("clear", true, &JZVariantMap::clear);
    cls_map.regist();    
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
    jzbind::ClassBind<QTimer> cls_timer(Type_timer,"Timer", "Object");
    cls_timer.def("start",true, QOverload<int>::of(&QTimer::start));
    cls_timer.def("stop", true, &QTimer::stop);
    cls_timer.def("isActive", false, &QTimer::isActive);
    cls_timer.defPrivateSingle("timeout", &QTimer::timeout);
    cls_timer.regist();
}

void initLayout()
{
    JZNodeObjectManager::instance()->delcare("Widget", typeid(QWidget).name(), Type_widget);

    jzbind::ClassBind<QLayout> cls_layout("Layout", "Object");    
    cls_layout.def("setContentsMargins", true,QOverload<int,int,int,int>::of(&QLayout::setContentsMargins));
    cls_layout.regist();

    jzbind::ClassBind<QBoxLayout> cls_box_layout("BoxLayout", "Layout");
    cls_box_layout.def("addLayout", true, [](QBoxLayout *l, QLayout *sub) { l->addLayout(sub); });
    cls_box_layout.def("addWidget", true, [](QBoxLayout *l, QWidget *w) { l->addWidget(w); });
    cls_box_layout.regist();

    jzbind::ClassBind<QHBoxLayout> cls_hbox_layout("HBoxLayout", "BoxLayout");     
    cls_hbox_layout.regist();

    jzbind::ClassBind<QVBoxLayout> cls_vbox_layout("VBoxLayout", "BoxLayout");    
    cls_vbox_layout.regist();
}

void initWidgets()
{
    //widget
    jzbind::ClassBind<QWidget> cls_widget(Type_widget, "Widget", "Object");
    cls_widget.def("setVisible", true, &QWidget::setVisible);
    cls_widget.def("show", true, &QWidget::show);
    cls_widget.def("hide", true, &QWidget::hide);
    cls_widget.def("close", true, &QWidget::close);
    cls_widget.def("rect", false, &QWidget::rect);
    cls_widget.def("update", true, QOverload<>::of(&QWidget::update));
    cls_widget.def("setLayout", true, &QWidget::setLayout);
    cls_widget.regist();

    //QFrame
    jzbind::ClassBind<QFrame> cls_frame("Frame", "Widget");
    cls_frame.regist();

    //QFrame
    jzbind::ClassBind<QAbstractScrollArea> cls_abs_scroll("AbstractScrollArea", "Frame");
    cls_abs_scroll.regist();

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
    jzbind::ClassBind<QAbstractButton> cls_abs_button("AbstractButton", "Widget");
    cls_abs_button.def("text", false, &QAbstractButton::text);
    cls_abs_button.def("setText", true, &QAbstractButton::setText);
    cls_abs_button.def("isChecked", false, &QAbstractButton::isChecked);
    cls_abs_button.def("setChecked", true, &QAbstractButton::setChecked);
    cls_abs_button.defSingle("clicked", &QAbstractButton::clicked);
    cls_abs_button.regist();

    jzbind::ClassBind<QPushButton> cls_button("PushButton", "AbstractButton");
    cls_button.regist();

    jzbind::ClassBind<QRadioButton> cls_radio_btn("RadioButton", "AbstractButton");
    cls_radio_btn.regist();

    jzbind::ClassBind<QToolButton> cls_tool_btn("ToolButton", "AbstractButton");
    cls_tool_btn.regist();

    jzbind::ClassBind<QCheckBox> cls_check_box("CheckBox", "AbstractButton");
    cls_check_box.regist();

    //stack
    jzbind::ClassBind<QStackedWidget> cls_stacked("StackedWidget", "Widget");
    cls_stacked.def("addWidget", true, &QStackedWidget::addWidget);
    cls_stacked.def("insertWidget", true, &QStackedWidget::insertWidget);
    cls_stacked.def("removeWidget", true, &QStackedWidget::removeWidget);
    cls_stacked.def("currentIndex", true, &QStackedWidget::currentIndex);
    cls_stacked.def("setCurrentIndex", true, &QStackedWidget::setCurrentIndex);
    cls_stacked.def("currentWidget", true, &QStackedWidget::currentWidget, false);
    cls_stacked.def("setCurrentWidget", true, &QStackedWidget::setCurrentWidget);
    cls_stacked.regist();
}

void initDialogs()
{
    jzbind::ClassBind<QDialog> cls_dlg("Dialog", "Widget");
    cls_dlg.def("exec",true, &QDialog::exec);
    cls_dlg.regist();
    
    jzbind::ClassBind<QMessageBox> cls_msg_box("MessageBox", "Dialog");
    cls_msg_box.def("information", true, [](QWidget *w, QString text) {
        QMessageBox::information(w, "", text);
    });
    cls_msg_box.regist();    
    
    jzbind::ClassBind<QFileDialog> cls_file_dlg("FileDialog", "Dialog");
    auto open_file_def = cls_file_dlg.def("getOpenFileName", true, [](QWidget *parent,QString caption,QString dir,QString filter)->QString 
    {
        return QFileDialog::getOpenFileName(parent, caption, dir, filter);
    });
    open_file_def->setDefaultValue(0, {"null","","",""});
    auto open_dlg_def = cls_file_dlg.def("getExistingDirectory", true, [](QWidget *parent, QString caption, QString dir)->QString
    {
        return QFileDialog::getExistingDirectory(parent, caption, dir);
    });
    open_dlg_def->setDefaultValue(0, { "null","",""});
    cls_file_dlg.regist();

    jzbind::ClassBind<QProgressDialog> cls_progress_dlg("ProgressDialog", "Dialog");
    cls_progress_dlg.def("create", false, []()->QProgressDialog* { 
        auto dlg = new QProgressDialog();
        dlg->setWindowModality(Qt::WindowModal);
        return dlg;
    }, false);        
    cls_progress_dlg.def("setRange", true, &QProgressDialog::setRange);
    cls_progress_dlg.def("setLabelText", true, &QProgressDialog::setLabelText);
    cls_progress_dlg.def("wasCanceled", false, &QProgressDialog::wasCanceled);
    cls_progress_dlg.def("value", false, &QProgressDialog::value);
    cls_progress_dlg.def("setValue", true, &QProgressDialog::setValue);
    cls_progress_dlg.regist();
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

    //image
    jzbind::ClassBind<QImage> cls_image("Image");
    cls_image.def("create", false, [](QString filename)->QImage{ return QImage(filename); });
    cls_image.def("load", true, [](QImage *image,QString filename)->bool { return image->load(filename); });
    cls_image.def("save", true, [](QImage *image, QString filename)->bool { return image->save(filename); });
    cls_image.regist();
}

void initFiles()
{
    jzbind::ClassBind<QFile> cls_file("File");
    cls_file.def("exists", false, QOverload<const QString&>::of(&QFile::exists));
    cls_file.regist();

    jzbind::ClassBind<QFileInfo> cls_fileInfo("FileInfo");
    cls_fileInfo.def("create", false, [](QString filepath)->QFileInfo {
        return QFileInfo(filepath);
    });
    cls_fileInfo.def("setFile", true, QOverload<const QString&>::of(&QFileInfo::setFile));
    cls_fileInfo.def("isDir", false, &QFileInfo::isDir);
    cls_fileInfo.def("isFile", false, &QFileInfo::isFile);
    cls_fileInfo.def("fileName", false, &QFileInfo::fileName);
    cls_fileInfo.def("filePath", false, &QFileInfo::filePath);
    cls_fileInfo.def("path", false, &QFileInfo::path);
    cls_fileInfo.def("suffix", false, &QFileInfo::suffix);
    cls_fileInfo.regist();
    
    jzbind::ClassBind<QDir> cls_dir("Dir");
    cls_dir.def("create", false, [](QString path)->QDir { return QDir(path);  });
    auto entry_def = cls_dir.def("entryList", false, [](QDir *dir,QString nameFilter,int filter)->QStringList{
        auto list = dir->nameFiltersFromString(nameFilter);
        return dir->entryList(list, (QDir::Filters)filter);
    });
    entry_def->paramIn[2].type = "Dir::Filters";
    cls_dir.def("mkpath", true, &QDir::mkpath);
    cls_dir.def("mkdir", true, &QDir::mkdir);
    cls_dir.def("isExists", false, QOverload<>::of(&QDir::exists));
    cls_dir.def("exists", false, QOverload<const QString&>::of(&QDir::exists));    
    cls_dir.regist();
}

QVariant colorEnum_to_color(const QVariant &v)
{
    QColor *color = new QColor((Qt::GlobalColor)v.toInt());
    auto ptr = JZObjectRefrence(color,true);
    return QVariant::fromValue(ptr);
}

QVariant colorEnum_to_brush(const QVariant &v)
{
    QBrush *brush = new QBrush((Qt::GlobalColor)v.toInt());
    auto ptr = JZObjectRefrence(brush,true);
    return QVariant::fromValue(ptr);
}

QVariant color_to_brush(const QVariant &v)
{    
    QColor *c = jzbind::fromVariant<QColor*>(v);
    QBrush *brush = new QBrush(*c);
    auto ptr = JZObjectRefrence(brush,true);
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
    initLayout();
    initWidgets();
    initDialogs();
    initPainter();
    initFiles();
    registConvert();
}
