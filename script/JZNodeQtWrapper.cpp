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
#include <QToolButton>
#include <QCheckBox>
#include <QPushButton>
#include <QTimer>
#include <QDialogButtonBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QProgressDialog>
#include <QApplication>
#include <QStackedWidget>
#include <QTreeWidget>
#include <QTableWidget>
#include <QListWidget>
#include <QComboBox>

#include "JZNodeQtWrapper.h"
#include "JZNodeObject.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeBind.h"
#include "JZRegExpHelp.h"
#include "JZContainer.h"
#include "JZNodeObjectParser.h"
#include "JZScriptEnvironment.h"

void checkSize(int index, int size)
{
    if (index < 0 || index >= size)
    {
        QString error = QString::asprintf("index %d out of range %d", index, size);
        throw std::runtime_error(qPrintable(error));
    }
}

class QtWrapper
{
public:
    void regist(JZScriptEnvironment *env);    

    void initEnum();
    void initBase();
    void initEvent();
    void initCore();
    void initObjects();
    void initLayout();
    void initWidgets();
    void initDialogs();
    void initPainter();
    void initFiles();
    void registConvert();

protected:
    JZScriptEnvironment *m_env;
    JZNodeObjectManager *m_objInst;
    JZNodeFunctionManager *m_funcInst;
};

void QtWrapper::regist(JZScriptEnvironment *env)
{
    m_env = env;
    m_objInst = env->objectManager();
    m_funcInst = env->functionManager();

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

void QtWrapper::initEnum()
{
    jzbind::registEnum<Qt::KeyboardModifiers>("Qt::KeyboardModifiers");
    jzbind::registEnum<Qt::SplitBehavior>("Qt::SplitBehavior");
    jzbind::registEnum<Qt::PenStyle>("Qt::PenStyle");
    jzbind::registEnum<Qt::Orientation>("Qt::Orientation");
    jzbind::registEnum<Qt::SortOrder>("Qt::SortOrder");
    jzbind::registEnum<Qt::Alignment>("Qt::Alignment");
    jzbind::registEnum<Qt::GlobalColor>("Qt::GlobalColor");

    jzbind::registEnum<Qt::Key>("Qt::Key", Type_keyCode);
        
    QStringList filter_key_list;
    QVector<int> filter_value_list;
    JZNodeEnumDefine filter_define; 
    filter_key_list << "Dirs";
    filter_key_list << "Files";
    filter_key_list << "NoDotAndDotDot";
    filter_value_list << QDir::Dirs;
    filter_value_list << QDir::Files;
    filter_value_list << QDir::NoDotAndDotDot;
    filter_define.init("QDir::Filter", filter_key_list, filter_value_list);
    int filter_id = m_objInst->registCEnum(filter_define,typeid(QDir::Filter).name());
    
    filter_define.init("QDir::Filters", filter_key_list, filter_value_list);
    filter_define.setFlag(true, filter_id);
    m_objInst->registCEnum(filter_define,typeid(QDir::Filters).name());    
}

void QtWrapper::initBase()
{
    m_objInst->delcareCClass("stringList",typeid(QStringList).name() ,Type_stringList);

    registContainer(m_env,"QList<int>",Type_intList);
    registContainer(m_env,"QList<double>",Type_doubleList);
    registContainer(m_env,"QList<any>",Type_varList);

    registContainer(m_env,"QMap<int,int>",Type_intIntMap);
    registContainer(m_env,"QMap<int,string>",Type_intStringMap);
    registContainer(m_env,"QMap<string,int>",Type_stringIntMap);
    registContainer(m_env,"QMap<string,string>",Type_stringStringMap);
    registContainer(m_env,"QMap<string,any>",Type_varMap);

    //string 全部只读
    jzbind::ClassBind<QString> cls_string(Type_string,"string");
    cls_string.def("format", false, [](const QString &format)->QString {
        QString fmt = format;
        return fmt;
    });
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

    //Point
    jzbind::ClassBind<QPoint> cls_pt(Type_point,"QPoint");
    cls_pt.setValueType(true);
    cls_pt.def("create", false, [](int x, int y)->QPoint { return QPoint(x, y); });
    cls_pt.def("__fromString__", false, [](const QString &text)->QPoint
    {
        JZNodeObjectParser parser;
        QVariantList list;
        if(!parser.parseVariantList("i,i",text,list)){
            throw std::runtime_error(qUtf8Printable(parser.error()));
        }

        QPoint pt;
        pt.setX(list[0].toInt());
        pt.setY(list[1].toInt());
        return pt;
    });
    cls_pt.def("__toString__", false, [](QPoint *pt)->QString{ 
        return QString::number(pt->x()) + "," + QString::number(pt->y());
    });
    cls_pt.def("x", false, &QPoint::x);
    cls_pt.def("y", false, &QPoint::y);
    cls_pt.def("setX", true, &QPoint::setX);
    cls_pt.def("setY", true, &QPoint::setY);
    cls_pt.regist();            

    jzbind::ClassBind<QPointF> cls_ptf(Type_pointF,"QPointF");
    cls_ptf.setValueType(true);
    cls_ptf.def("create", false, [](double x, double y)->QPointF { return QPointF(x, y); });
    cls_ptf.def("__fromString__", false, [](const QString &text)->QPointF {
        JZNodeObjectParser parser;
        QVariantList list;
        if(!parser.parseVariantList("i,i",text,list)){
            throw std::runtime_error(qUtf8Printable(parser.error()));
        }

        QPointF pt;
        pt.setX(list[0].toDouble());
        pt.setY(list[1].toDouble());
        return pt;
    });
    cls_ptf.def("__toString__", false, [](QPointF *pt)->QString {
        return QString::number(pt->x()) + "," + QString::number(pt->y());
    });
    cls_ptf.def("x", false, &QPointF::x);
    cls_ptf.def("y", false, &QPointF::y);
    cls_ptf.def("setX", true, &QPointF::setX);
    cls_ptf.def("setY", true, &QPointF::setY);
    cls_ptf.regist();

    jzbind::ClassBind<QRect> cls_rect(Type_rect,"QRect");
    cls_rect.setValueType(true);
    cls_rect.def("create", false, [](int x, int y, int w, int h)->QRect {
        return QRect(x, y, w, h);
    });
    cls_rect.def("__fromString__", false, [](const QString &text)->QRect { 
        JZNodeObjectParser parser;
        QVariantList list;
        if(!parser.parseVariantList("i,i,i,i",text,list)){
            throw std::runtime_error(qUtf8Printable(parser.error()));
        }

        auto x = list[0].toInt();
        auto y = list[1].toInt();
        auto w = list[2].toInt();
        auto h = list[3].toInt();
        return QRect(x,y,w,h);
    });
    cls_rect.regist();

    jzbind::ClassBind<QRectF> cls_rectf(Type_rectF,"QRectF");
    cls_rectf.setValueType(true);
    cls_rectf.def("create", false, [](double x, double y, double w, double h)->QRectF {
        return QRectF(x, y, w, h);
    });
    cls_rectf.def("__fromString__", false, [](const QString &text)->QRectF {  
        JZNodeObjectParser parser;
        QVariantList list;
        if(!parser.parseVariantList("i,i,i,i",text,list)){
            throw std::runtime_error(qUtf8Printable(parser.error()));
        }

        auto x = list[0].toDouble();
        auto y = list[1].toDouble();
        auto w = list[2].toDouble();
        auto h = list[3].toDouble();
        return QRectF(x,y,w,h);    
    });
    cls_rectf.regist();

    jzbind::ClassBind<QColor> cls_color("QColor");
    cls_color.setValueType(true);
    cls_color.def("create", false, [](int r, int g, int b)->QColor {
        return QColor(r, g, b);
    });
    cls_color.def("__fromString__", false, [](const QString &text)->QColor { 
        JZNodeObjectParser parser;
        QVariantList list;
        if(!parser.parseVariantList("i,i,i",text,list)){
            throw std::runtime_error(qUtf8Printable(parser.error()));
        } 

        auto r = list[0].toInt();
        auto g = list[1].toInt();
        auto b = list[2].toInt();
        return QColor(r, g, b);
    });
    cls_color.def("red", false, &QColor::red);
    cls_color.def("green", false, &QColor::green);
    cls_color.def("blue", false, &QColor::blue);    
    cls_color.regist();    
}

void QtWrapper::initCore()
{
    jzbind::ClassBind<QObject> cls_object(Type_object,"QObject");
    cls_object.def("setObjectName",true,&QObject::setObjectName);
    cls_object.def("objectName",false,&QObject::objectName);
    cls_object.regist();    
    
    //app
    jzbind::ClassBind<QApplication> cls_app("QApplication");    
    cls_app.def("setStyleSheet", true, [](const QString &style) { qApp->setStyleSheet(style); });
    cls_app.regist();

    //stringlist , 这里需要全部使用 lambda 因为 QList<QString> 未定义
    jzbind::ClassBind<QStringList> cls_string_list(Type_stringList,"stringList");
    cls_string_list.def("create", false, []()->QStringList {        
        return QStringList();
    });
    cls_string_list.def("__fromString__", false, [](const QString &text)->QStringList {
        QStringList list = text.split(",");
        return list;
    });
    cls_string_list.def("join", false, [](const QStringList *list, const QString &sep)->QString {
        return (*list).join(sep);
    });
    cls_string_list.def("push_back", true,[](QStringList *list,const QString &text){ 
        list->push_back(text); 
    });
    cls_string_list.def("pop_back", true, [](QStringList *list){
        if(list->size() > 0)
            list->pop_back();
    });
    cls_string_list.def("push_front", true, [](QStringList *list,const QString &text){ 
        list->push_front(text); 
    });
    cls_string_list.def("pop_front", true, [](QStringList *list){
        if(list->size() > 0)
            list->pop_back();
    });
    cls_string_list.def("removeAt", true, [](QStringList *list,int index){
        list->removeAt(index);
    });
    cls_string_list.def("removeOne",true, [](QStringList *list,const QString &text){
        list->removeOne(text);
    });
    cls_string_list.def("removeAll",true, [](QStringList *list,const QString &text){
        list->removeAll(text);
    });
    cls_string_list.def("indexOf", false, [](const QStringList *list, const QString &text)->int{
        return list->indexOf(text);
    });
    cls_string_list.def("set", true, [](QStringList *list, int index, const QString &str){
        checkSize(index, list->size());
        (*list)[index] = str;
    });
    cls_string_list.def("get", false, [](const QStringList *list,int index)->QString {
        checkSize(index, list->size());
        return (*list)[index];
    });
    cls_string_list.def("size", false, [](QStringList *list)->int{
        return list->size();
    });
    cls_string_list.def("clear",true, [](QStringList *list){
        list->clear();
    });
    cls_string_list.regist();    

    jzbind::ClassBind<QByteArray> cls_byte_array(Type_byteArray,"ByteArray");
    cls_byte_array.regist();

    jzbind::ClassBind<QDataStream> cls_data_stream(Type_dataStream,"DataStream");
    cls_data_stream.regist();
}

void QtWrapper::initEvent()
{    
    jzbind::ClassBind<QEvent> cls_event(Type_event,"QEvent");
    cls_event.regist();

    jzbind::ClassBind<QResizeEvent> cls_resize_event(Type_resizeEvent,"QResizeEvent");
    cls_resize_event.regist();

    jzbind::ClassBind<QShowEvent> cls_show_event(Type_showEvent,"QShowEvent");
    cls_show_event.regist();

    jzbind::ClassBind<QPaintEvent> cls_paint_event(Type_paintEvent,"QPaintEvent");
    cls_paint_event.regist();

    jzbind::ClassBind<QCloseEvent> cls_close_event(Type_closeEvent,"QCloseEvent");
    cls_close_event.regist();

    jzbind::ClassBind<QKeyEvent> cls_key_event(Type_keyEvent,"QKeyEvent");
    cls_key_event.def("key", false, &QKeyEvent::key);
    cls_key_event.def("modifiers", false, &QKeyEvent::modifiers);
    cls_key_event.regist();

    jzbind::ClassBind<QMouseEvent> cls_mouse_event(Type_mouseEvent,"QMouseEvent");
    cls_mouse_event.def("pos", false, &QMouseEvent::pos);
    cls_mouse_event.def("x", false, &QMouseEvent::x);
    cls_mouse_event.def("y", false, &QMouseEvent::y);
    cls_mouse_event.regist();
}

void QtWrapper::initObjects()
{
    jzbind::ClassBind<QTimer> cls_timer(Type_timer,"QTimer", "QObject");
    cls_timer.def("start",true, QOverload<int>::of(&QTimer::start));
    cls_timer.def("stop", true, &QTimer::stop);
    cls_timer.def("isActive", false, &QTimer::isActive);
    cls_timer.defPrivateSingle("timeout", &QTimer::timeout);
    cls_timer.regist();
}

void QtWrapper::initLayout()
{
    m_objInst->delcareCClass("QWidget", typeid(QWidget).name(), Type_widget);

    jzbind::ClassBind<QLayout> cls_layout(Type_layout,"QLayout", "QObject");    
    cls_layout.def("setContentsMargins", true,QOverload<int,int,int,int>::of(&QLayout::setContentsMargins));
    cls_layout.regist();

    jzbind::ClassBind<QBoxLayout> cls_box_layout(Type_boxLayout,"QBoxLayout", "QLayout");
    cls_box_layout.def("addLayout", true, [](QBoxLayout *l, QLayout *sub) { l->addLayout(sub); });
    cls_box_layout.def("addWidget", true, [](QBoxLayout *l, QWidget *w) { l->addWidget(w); });
    cls_box_layout.regist();

    jzbind::ClassBind<QHBoxLayout> cls_hbox_layout(Type_hBoxLayout,"QHBoxLayout", "QBoxLayout");     
    cls_hbox_layout.regist();

    jzbind::ClassBind<QVBoxLayout> cls_vbox_layout(Type_vBoxLayout,"QVBoxLayout", "QBoxLayout");    
    cls_vbox_layout.regist();

    jzbind::ClassBind<QGridLayout> cls_gird_layout(Type_gridLayout,"QGridLayout", "QBoxLayout");    
    cls_gird_layout.regist();
}

void QtWrapper::initWidgets()
{
    //widget
    jzbind::ClassBind<QWidget> cls_widget(Type_widget, "QWidget", "QObject");
    cls_widget.def("setVisible", true, &QWidget::setVisible);
    cls_widget.def("show", true, &QWidget::show);
    cls_widget.def("hide", true, &QWidget::hide);
    cls_widget.def("close", true, &QWidget::close);
    cls_widget.def("rect", false, &QWidget::rect);
    cls_widget.def("update", true, QOverload<>::of(&QWidget::update));
    cls_widget.def("setLayout", true, &QWidget::setLayout);
    cls_widget.regist();

    //QFrame
    jzbind::ClassBind<QFrame> cls_frame(Type_frame,"QFrame", "QWidget");
    cls_frame.regist();

    //AbstractScrollArea
    jzbind::ClassBind<QAbstractScrollArea> cls_abs_scroll("QAbstractScrollArea", "QFrame");
    cls_abs_scroll.regist();

    //QLabel
    jzbind::ClassBind<QLabel> cls_label("QLabel", "QWidget");
    cls_label.def("text", false, &QLabel::text);
    cls_label.def("setText", true, &QLabel::setText);;
    cls_label.regist();

    //lineedit
    jzbind::ClassBind<QLineEdit> cls_lineEdit(Type_lineEdit,"QLineEdit", "QWidget");
    cls_lineEdit.def("text", false, &QLineEdit::text);
    cls_lineEdit.def("setText", true, &QLineEdit::setText);;
    cls_lineEdit.regist();

    //textedit
    jzbind::ClassBind<QTextEdit> cls_textEdit(Type_textEdit,"QTextEdit", "QWidget");
    cls_textEdit.def("toPlainText", false, &QTextEdit::toPlainText);
    cls_textEdit.def("setText", true, &QTextEdit::setText);
    cls_textEdit.def("append", true, &QTextEdit::append);
    cls_textEdit.regist();

    //combobox
    jzbind::ClassBind<QComboBox> cls_comboBox(Type_comboBox,"QComboBox", "QWidget");
    cls_comboBox.def("setCurrentIndex", false, &QComboBox::setCurrentIndex);
    cls_comboBox.def("currentIndex", false, &QComboBox::currentIndex);
    cls_comboBox.def("addItem", true, [](QComboBox *box,const QString &item){ box->addItem(item); });
    cls_comboBox.def("addItems", true, &QComboBox::addItems);
    cls_comboBox.regist();

    //abs_button
    jzbind::ClassBind<QAbstractButton> cls_abs_button("QAbstractButton", "QWidget");
    cls_abs_button.def("text", false, &QAbstractButton::text);
    cls_abs_button.def("setText", true, &QAbstractButton::setText);
    cls_abs_button.def("isChecked", false, &QAbstractButton::isChecked);
    cls_abs_button.def("setChecked", true, &QAbstractButton::setChecked);
    cls_abs_button.defSingle("clicked", &QAbstractButton::clicked);
    cls_abs_button.regist();

    jzbind::ClassBind<QPushButton> cls_button(Type_pushButton,"QPushButton", "QAbstractButton");
    cls_button.regist();

    jzbind::ClassBind<QRadioButton> cls_radio_btn(Type_radioButton,"QRadioButton", "QAbstractButton");
    cls_radio_btn.regist();

    jzbind::ClassBind<QToolButton> cls_tool_btn(Type_toolButton,"QToolButton", "QAbstractButton");
    cls_tool_btn.regist();

    jzbind::ClassBind<QCheckBox> cls_check_box(Type_checkBox,"QCheckBox", "QAbstractButton");
    cls_check_box.regist();

    //stack
    jzbind::ClassBind<QStackedWidget> cls_stacked("QStackedWidget", "QWidget");
    cls_stacked.def("addWidget", true, &QStackedWidget::addWidget);
    cls_stacked.def("insertWidget", true, &QStackedWidget::insertWidget);
    cls_stacked.def("removeWidget", true, &QStackedWidget::removeWidget);
    cls_stacked.def("currentIndex", true, &QStackedWidget::currentIndex);
    cls_stacked.def("setCurrentIndex", true, &QStackedWidget::setCurrentIndex);
    cls_stacked.def("currentWidget", true, &QStackedWidget::currentWidget, true);
    cls_stacked.def("setCurrentWidget", true, &QStackedWidget::setCurrentWidget);
    cls_stacked.regist();

    //table
    jzbind::ClassBind<QTableWidgetItem> cls_table_item(Type_tableWidgetItem,"QTableWidgetItem");
    cls_table_item.def("setText", true, &QTableWidgetItem::setText);
    cls_table_item.def("text", false, &QTableWidgetItem::text);
    cls_table_item.regist();

    jzbind::ClassBind<QTableWidget> cls_table(Type_tableWidget,"QTableWidget", "QWidget");
    cls_table.def("setColumnCount", true, &QTableWidget::setColumnCount);
    cls_table.def("setHorizontalHeaderLabels", true, &QTableWidget::setHorizontalHeaderLabels);
    cls_table.def("setRowCount", true, &QTableWidget::setRowCount);
    cls_table.def("rowCount", false, &QTableWidget::rowCount);
    cls_table.def("currentRow", false, &QTableWidget::currentRow);
    cls_table.def("clearContents", true, &QTableWidget::clearContents);
    cls_table.def("setItem", true, &QTableWidget::setItem);
    cls_table.def("item", false, &QTableWidget::item, false);
    cls_table.regist();

    //list
    jzbind::ClassBind<QListWidgetItem> cls_list_item(Type_listWidgetItem,"QListWidgetItem");
    cls_list_item.regist();

    jzbind::ClassBind<QListWidget> cls_list(Type_listWidget,"QListWidget", "QWidget");
    cls_list.regist();

    //tree
    jzbind::ClassBind<QTreeWidgetItem> cls_tree_item(Type_treeWidgetItem,"QTreeWidgetItem");
    cls_tree_item.regist();

    jzbind::ClassBind<QTreeWidget> cls_tree(Type_treeWidget,"QTreeWidget", "QWidget");
    cls_tree.regist(); 
}

void QtWrapper::initDialogs()
{
    jzbind::ClassBind<QDialog> cls_dlg("QDialog", "QWidget");
    cls_dlg.def("exec",true, &QDialog::exec);
    cls_dlg.regist();
    
    jzbind::ClassBind<QMessageBox> cls_msg_box("QMessageBox", "QDialog");
    cls_msg_box.def("information", true, [](QWidget *w, QString title,QString text) {
        QMessageBox::information(w, title, text);
    });
    cls_msg_box.regist();    
    
    int file_dlg_option = jzbind::registEnum<QFileDialog::Option>("QFileDialog.Option");
    int file_dlg_options = jzbind::registEnum<QFileDialog::Options>("QFileDialog.Options");
    m_objInst->enumMeta(file_dlg_options)->setFlag(true, file_dlg_option);

    jzbind::ClassBind<QFileDialog> cls_file_dlg("QFileDialog", "Dialog");
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

    jzbind::ClassBind<QProgressDialog> cls_progress_dlg("QProgressDialog", "Dialog");
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

void QtWrapper::initPainter()
{
    //pen
    jzbind::ClassBind<QPen> cls_pen(Type_pen,"QPen");
    cls_pen.def("create", false, [](QColor c,int width,Qt::PenStyle style)->QPen*{ return new QPen(c, width, style); }, false);
    cls_pen.regist();

    //brush
    jzbind::ClassBind<QBrush> cls_brush(Type_brush,"QBrush");
    cls_brush.def("create", false, [](QColor c)->QBrush* { return new QBrush(c); }, false);
    cls_brush.regist();

    //painter
    jzbind::ClassBind<QPainter> cls_painter(Type_painter,"QPainter");
    cls_painter.def("create", true, [](QWidget *w)->QPainter* { return new QPainter(w); }, false);
    cls_painter.def("drawRect", true, QOverload<const QRect&>::of(&QPainter::drawRect));
    cls_painter.def("fillRect", true, QOverload<const QRect&, const QColor&>::of(&QPainter::fillRect));
    cls_painter.regist();

    //image
    jzbind::ClassBind<QImage> cls_image(Type_image,"QImage");
    cls_image.setValueType(true);
    cls_image.def("create", false, [](QString filename)->QImage{ return QImage(filename); });
    cls_image.def("load", true, [](QImage *image,QString filename)->bool { return image->load(filename); });
    cls_image.def("save", true, [](QImage *image, QString filename)->bool { return image->save(filename); });
    cls_image.regist();
}

void QtWrapper::initFiles()
{   
    auto obj_inst = m_objInst;

    QStringList file_mode_key_list;
    QVector<int> file_mode_value_list;
    file_mode_key_list << "ReadOnly" << "WriteOnly" << "ReadWrite" << "Append" << "Truncate" << "Text";
    file_mode_value_list << QFile::ReadOnly << QFile::WriteOnly << QFile::ReadWrite << QFile::Append << QFile::Truncate << QFile::Text;
    
    JZNodeEnumDefine file_mode_define; 
    file_mode_define.init("QFile.OpenModeFlag", file_mode_key_list, file_mode_value_list);
    int file_mode_enum = obj_inst->registCEnum(file_mode_define,typeid(QFile::OpenModeFlag).name());

    JZNodeEnumDefine file_flag_define; 
    file_flag_define.init("QFile.OpenMode",{},{});
    int file_mode_flag = obj_inst->registCEnum(file_mode_define,typeid(QFile::OpenMode).name());
    obj_inst->enumMeta(file_mode_enum)->setFlag(true, file_mode_flag);

    jzbind::ClassBind<QFile> cls_file("QFile");
    cls_file.def("exists", false, QOverload<const QString&>::of(&QFile::exists));
    cls_file.def("open", true, [](QFile *file,QFile::OpenMode mode)->bool{ return file->open(mode); });
    cls_file.def("close", true, [](QFile *file){ file->close(); });
    cls_file.def("read", true, [](QFile *file,qint64 size){ file->read(size); });
    cls_file.def("readAll", true, [](QFile *file)->QByteArray{ return file->readAll(); });
    cls_file.def("write", true, [](QFile *file,const QByteArray &buffer){ file->write(buffer); });
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
    
    jzbind::ClassBind<QDir> cls_dir("QDir");
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

QVariant colorEnum_to_color(JZScriptEnvironment *env,const QVariant &v)
{
    QColor *color = new QColor((Qt::GlobalColor)v.toInt());
    auto ptr = env->objectRefrence(color,true);
    return QVariant::fromValue(ptr);
}

QVariant colorEnum_to_brush(JZScriptEnvironment *env,const QVariant &v)
{
    QBrush *brush = new QBrush((Qt::GlobalColor)v.toInt());
    auto ptr = env->objectRefrence(brush,true);
    return QVariant::fromValue(ptr);
}

QVariant color_to_brush(JZScriptEnvironment *env,const QVariant &v)
{    
    QColor *c = jzbind::fromVariant<QColor*>(v);
    QBrush *brush = new QBrush(*c);
    auto ptr = env->objectRefrence(brush,true);
    return QVariant::fromValue(ptr);
}

void QtWrapper::registConvert()
{
    auto inst = m_objInst;
    int from_id = inst->getEnumId("Qt::GlobalColor");
    int to_id = inst->getClassId("QBrush");

    m_env->registConvert(from_id, to_id, colorEnum_to_brush);
    to_id = inst->getClassId("QColor");
    m_env->registConvert(from_id, to_id, colorEnum_to_color);

    from_id = inst->getClassId("QColor");
    to_id = inst->getClassId("QBrush");
    m_env->registConvert(from_id, to_id, color_to_brush);
}

void registQtClass(JZScriptEnvironment *env)
{    
    QtWrapper wrapper;
    wrapper.regist(env);    
}
