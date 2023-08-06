#include <QApplication>
#include "Russian.h"
#include "JZNodeBuilder.h"
#include "JZNodeVM.h"
#include "UiCommon.h"
#include "JZNodeFunction.h"
#include "JZNodeValue.h"
#include "JZUiFile.h"
#include <QDir>

static QString xml = 
R"(<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>widget</class>
 <widget class="QWidget" name="widget">
  <property name="geometry">
   <rect>
    <x>0</x>
    <y>0</y>
    <width>430</width>
    <height>552</height>
   </rect>
  </property>
  <widget class="QPushButton" name="btnStart">
   <property name="geometry">
    <rect>
     <x>300</x>
     <y>137</y>
     <width>111</width>
     <height>41</height>
    </rect>
   </property>
   <property name="text">
    <string>开始</string>
   </property>
   <property name="focusPolicy">
    <enum>Qt::NoFocus</enum>
   </property>
  </widget>
  <widget class="QPushButton" name="btnStop">
   <property name="geometry">
    <rect>
     <x>300</x>
     <y>190</y>
     <width>111</width>
     <height>41</height>
    </rect>
   </property>
   <property name="text">
    <string>结束</string>
   </property>
   <property name="focusPolicy">
    <enum>Qt::NoFocus</enum>
   </property>
  </widget>
  <widget class="QLabel" name="label">
   <property name="geometry">
    <rect>
     <x>300</x>
     <y>30</y>
     <width>72</width>
     <height>15</height>
    </rect>
   </property>
   <property name="text">
    <string>得分</string>
   </property>
  </widget>
  
 </widget>
 <resources/>
 <connections/>
</ui>
)";

SampleRussian::SampleRussian()
{
    m_project.initUi();    
    m_script = dynamic_cast<JZScriptFile*>(m_project.getItem("./mainwindow/事件"));

    JZUiFile *ui_file = dynamic_cast<JZUiFile*>(m_project.getItem("./mainwindow/mainwindow.ui"));
    ui_file->setXml(xml);
    m_project.saveItem(ui_file);

    auto class_file = m_project.getClass("mainwindow");
    class_file->addMemberVariable("map", Type_list);
    class_file->addMemberVariable("colors", Type_list);
    class_file->addMemberVariable("timer", Type_timer);
    class_file->addMemberVariable("isRect", Type_bool);
    class_file->addMemberVariable("listRow", Type_list);
    class_file->addMemberVariable("listCol", Type_list);
    
    addInitFunction();
    addMapGet();
    addMapSet();
    addMoveFunction();
    addPaintEvent();    
    addCreateRect();
    addRectDown();
    addButtonClicked();
    addGameLoop();
    addKeyEvent();

    auto main = dynamic_cast<JZScriptFile*>(m_project.getItem(m_project.mainScript()));
    main->removeNode(3);
    auto node = main->getNode(2);
    JZNodeParam *node_main = new JZNodeParam();
    node_main->setVariable("mainwindow");

    JZNodeFunction *node_show = new JZNodeFunction();
    node_show->setFunction(JZNodeFunctionManager::instance()->function("Widget.show"));

    JZNodeFunction *init_func = new JZNodeFunction();
    init_func->setFunction(&class_file->getMemberFunction("init")->function());

    main->addNode(JZNodePtr(init_func));
    main->addNode(JZNodePtr(node_main));
    main->addNode(JZNodePtr(node_show));
    main->addConnect(node_main->paramOutGemo(0), init_func->paramInGemo(0));
    main->addConnect(node->flowOutGemo(0), init_func->flowInGemo());
    main->addConnect(node_main->paramOutGemo(0), node_show->paramInGemo(0));
    main->addConnect(init_func->flowOutGemo(0), node_show->flowInGemo());
}

SampleRussian::~SampleRussian()
{

}

bool SampleRussian::run()
{
    QString program_path = qApp->applicationDirPath() + "/russian.bin";

    JZNodeBuilder builder;
    JZNodeProgram program;
    if (!builder.build(&m_project, &program))
    {
        qDebug().noquote() << builder.error();
        return false;
    }    
    if (!program.save(program_path))
    {
        qDebug() << "save failed";
        return false;
    }
    qDebug().noquote() << program.dump();
    m_project.saveAllItem();

    QString dir = qApp->applicationDirPath() + "/sample";
    if (!QDir().exists(dir))
        QDir().mkdir(dir);
    QString path = qApp->applicationDirPath() + "/sample/russion.jzproject";
    m_project.saveAs(path);

    JZNodeVM vm;
    QString error;
    if (!vm.init(program_path, false, error))
    {
        QMessageBox::information(nullptr, "", "init program \"" + program_path + "\" failed\n" + error);
        return 1;
    }
    return qApp->exec();
}

void SampleRussian::addInitFunction()
{
    auto func_inst = JZNodeFunctionManager::instance();
    auto color_meta = JZNodeObjectManager::instance()->meta("Color");
    auto list_meta = JZNodeObjectManager::instance()->meta("List");

    auto class_file = m_project.getClass("mainwindow");
    auto mainwindow_meta = JZNodeObjectManager::instance()->meta("mainwindow");

    FunctionDefine define;
    define.name = "init";
    define.isFlowFunction = true;
    define.paramIn.push_back(JZParamDefine("obj",mainwindow_meta->id));

    auto script = class_file->addMemberFunction(define);
    JZNode *func_start = script->getNode(0);

    JZNodeSetParam *set_map = new JZNodeSetParam();
    set_map->setVariable("this.map");
    
    JZNodeSetParam *set_color_list = new JZNodeSetParam();
    set_color_list->setVariable("this.colors");

    JZNodeCreate *create_map = new JZNodeCreate();
    create_map->setClassName("List");

    JZNodeCreate *create_color_list = new JZNodeCreate();
    create_color_list->setClassName("List");    

    JZNodeParam *node_color_list = new JZNodeParam();
    node_color_list->setVariable("this.colors");
    
    script->addNode(JZNodePtr(set_map));
    script->addNode(JZNodePtr(set_color_list));
    script->addNode(JZNodePtr(create_map));
    script->addNode(JZNodePtr(create_color_list));

    script->addNode(JZNodePtr(node_color_list));    

    script->addConnect(func_start->flowOutGemo(0), create_map->flowInGemo());
    script->addConnect(create_map->paramOutGemo(0), set_map->paramInGemo(0));
    script->addConnect(create_map->flowOutGemo(0), set_map->flowInGemo());
    script->addConnect(set_map->flowOutGemo(0), create_color_list->flowInGemo());
    script->addConnect(create_color_list->paramOutGemo(0), set_color_list->paramInGemo(0));
    script->addConnect(create_color_list->flowOutGemo(0), set_color_list->flowInGemo());

    JZNodeFunction *color_resize = new JZNodeFunction();
    color_resize->setFunction(func_inst->function("List.resize"));
    script->addNode(JZNodePtr(color_resize));

    script->addConnect(node_color_list->paramOutGemo(0), color_resize->paramInGemo(0));
    color_resize->setPropValue(color_resize->paramIn(1), 5);
    script->addConnect(set_color_list->flowOutGemo(0), color_resize->flowInGemo());

    QColor color_list[] = { Qt::red,Qt::green,Qt::blue,Qt::yellow,Qt::cyan };
    JZNode *pre_node = color_resize;
    //create color
    for (int i = 0; i < 5; i++)
    {
        int r = color_list[i].red();
        int g = color_list[i].green();
        int b = color_list[i].blue();

        JZNodeFunction *color_create = new JZNodeFunction();
        color_create->setFunction(color_meta->function("create"));
        color_create->setPropValue(color_create->paramIn(0), r);
        color_create->setPropValue(color_create->paramIn(1), g);
        color_create->setPropValue(color_create->paramIn(2), b);

        JZNodeFunction *list_set = new JZNodeFunction();
        list_set->setFunction(list_meta->function("set"));
        script->addNode(JZNodePtr(color_create));
        script->addNode(JZNodePtr(list_set));

        script->addConnect(pre_node->flowOutGemo(0), list_set->flowInGemo());
        script->addConnect(node_color_list->paramOutGemo(0), list_set->paramInGemo(0));
        list_set->setPropValue(list_set->paramIn(1), i);
        script->addConnect(color_create->paramOutGemo(0), list_set->paramInGemo(2));
        pre_node = list_set;
    }

    JZNodeParam *node_map = new JZNodeParam();
    node_map->setVariable("this.map");

    JZNodeFor *for_row = new JZNodeFor();    
    for_row->setRange(0, m_row);

    JZNodeFor *for_col = new JZNodeFor();
    for_col->setRange(0, m_col);

    JZNodeFunction *row_push = new JZNodeFunction();
    row_push->setFunction(func_inst->function("List.push_back"));

    JZNodeCreate *create_col = new JZNodeCreate();
    create_col->setClassName("List");

    JZNodeFunction *col_push = new JZNodeFunction();
    col_push->setFunction(func_inst->function("List.push_back"));
    col_push->setPropValue(col_push->paramIn(1), -1);

    script->addNode(JZNodePtr(node_map));
    script->addNode(JZNodePtr(for_col));
    script->addNode(JZNodePtr(row_push));
    script->addNode(JZNodePtr(for_row));
    script->addNode(JZNodePtr(create_col));
    script->addNode(JZNodePtr(col_push));
    script->addConnect(pre_node->flowOutGemo(0), for_row->flowInGemo());

    script->addConnect(for_row->subFlowOutGemo(0), create_col->flowInGemo());
    script->addConnect(create_col->flowOutGemo(0), for_col->flowInGemo());

    script->addConnect(create_col->paramOutGemo(0), col_push->paramInGemo(0));
    script->addConnect(for_col->subFlowOutGemo(0), col_push->flowInGemo());

    script->addConnect(node_map->paramOutGemo(0), row_push->paramInGemo(0));
    script->addConnect(create_col->paramOutGemo(0), row_push->paramInGemo(1));
    script->addConnect(for_col->flowOutGemo(0), row_push->flowInGemo());
    
    JZNodeCreate *create_timer = new JZNodeCreate();
    create_timer->setClassName("Timer");

    JZNodeSetParam *set_timer = new JZNodeSetParam();
    set_timer->setVariable("this.timer");

    script->addNode(JZNodePtr(create_timer));
    script->addNode(JZNodePtr(set_timer));

    script->addConnect(for_row->flowOutGemo(0), create_timer->flowInGemo());
    script->addConnect(create_timer->paramOutGemo(0), set_timer->paramInGemo(0));
    script->addConnect(create_timer->flowOutGemo(0), set_timer->flowInGemo());

    // create list row
    JZNodeCreate *create_list_row = new JZNodeCreate();
    create_list_row->setClassName("List");

    JZNodeSetParam *set_list_row = new JZNodeSetParam();
    set_list_row->setVariable("this.listRow");

    script->addNode(JZNodePtr(create_list_row));
    script->addNode(JZNodePtr(set_list_row));

    script->addConnect(set_timer->flowOutGemo(0), create_list_row->flowInGemo());
    script->addConnect(create_list_row->paramOutGemo(0), set_list_row->paramInGemo(0));
    script->addConnect(create_list_row->flowOutGemo(0), set_list_row->flowInGemo());

    // create list col
    JZNodeCreate *create_list_col = new JZNodeCreate();
    create_list_col->setClassName("List");

    JZNodeSetParam *set_list_col = new JZNodeSetParam();
    set_list_col->setVariable("this.listCol");

    script->addNode(JZNodePtr(create_list_col));
    script->addNode(JZNodePtr(set_list_col));

    script->addConnect(set_list_row->flowOutGemo(0), create_list_col->flowInGemo());
    script->addConnect(create_list_col->paramOutGemo(0), set_list_col->paramInGemo(0));
    script->addConnect(create_list_col->flowOutGemo(0), set_list_col->flowInGemo());
}

void SampleRussian::addMapGet()
{
    auto func_inst = JZNodeFunctionManager::instance();
    auto mainwindow_id = JZNodeObjectManager::instance()->getClassId("mainwindow");
    auto class_file = m_project.getClass("mainwindow");

    FunctionDefine define;
    define.name = "getMap";
    define.isFlowFunction = false;
    define.paramIn.push_back(JZParamDefine("obj", mainwindow_id));
    define.paramIn.push_back(JZParamDefine("row", Type_int));
    define.paramIn.push_back(JZParamDefine("col", Type_int));
    define.paramOut.push_back(JZParamDefine("", Type_int));

    auto script = class_file->addMemberFunction(define);
    JZNode *func_start = script->getNode(0);    

    JZNodeParam *node_row = new JZNodeParam();
    node_row->setVariable("row");

    JZNodeParam *node_col = new JZNodeParam();
    node_col->setVariable("col");

    JZNodeParam *node_map = new JZNodeParam();
    node_map->setVariable("this.map");

    JZNodeFunction *row_get = new JZNodeFunction();
    row_get->setFunction(func_inst->function("List.get"));

    JZNodeFunction *col_get = new JZNodeFunction();
    col_get->setFunction(func_inst->function("List.get"));

    JZNodeReturn *node_return = new JZNodeReturn();
    node_return->setFunction(&define);

    script->addNode(JZNodePtr(node_row));
    script->addNode(JZNodePtr(node_col));
    script->addNode(JZNodePtr(node_map));
    script->addNode(JZNodePtr(row_get));
    script->addNode(JZNodePtr(col_get));
    script->addNode(JZNodePtr(node_return));
        
    script->addConnect(node_map->paramOutGemo(0), row_get->paramInGemo(0));
    script->addConnect(node_row->paramOutGemo(0), row_get->paramInGemo(1));
    script->addConnect(row_get->paramOutGemo(0), col_get->paramInGemo(0));    
    script->addConnect(node_col->paramOutGemo(0), col_get->paramInGemo(1));

    script->addConnect(col_get->paramOutGemo(0), node_return->paramInGemo(0));
    script->addConnect(func_start->flowOutGemo(0), node_return->flowInGemo());
}

void SampleRussian::addMapSet()
{
    auto func_inst = JZNodeFunctionManager::instance();
    auto mainwindow_id = JZNodeObjectManager::instance()->getClassId("mainwindow");
    auto class_file = m_project.getClass("mainwindow");

    FunctionDefine define;
    define.name = "setMap";
    define.isFlowFunction = true;
    define.paramIn.push_back(JZParamDefine("obj", mainwindow_id));
    define.paramIn.push_back(JZParamDefine("row", Type_int));
    define.paramIn.push_back(JZParamDefine("col", Type_int));
    define.paramIn.push_back(JZParamDefine("value", Type_int));

    auto script = class_file->addMemberFunction(define);
    JZNode *func_start = script->getNode(0);

    JZNodeParam *node_row = new JZNodeParam();
    node_row->setVariable("row");

    JZNodeParam *node_col = new JZNodeParam();
    node_col->setVariable("col");

    JZNodeParam *node_value = new JZNodeParam();
    node_value->setVariable("value");

    JZNodeParam *node_map = new JZNodeParam();
    node_map->setVariable("this.map");

    JZNodeFunction *row_get = new JZNodeFunction();
    row_get->setFunction(func_inst->function("List.get"));

    JZNodeFunction *col_set = new JZNodeFunction();
    col_set->setFunction(func_inst->function("List.set"));
    
    script->addNode(JZNodePtr(node_row));
    script->addNode(JZNodePtr(node_col));
    script->addNode(JZNodePtr(node_map));
    script->addNode(JZNodePtr(row_get));
    script->addNode(JZNodePtr(col_set));    
    script->addNode(JZNodePtr(node_value));

    script->addConnect(node_map->paramOutGemo(0), row_get->paramInGemo(0));
    script->addConnect(node_row->paramOutGemo(0), row_get->paramInGemo(1));

    script->addConnect(row_get->paramOutGemo(0), col_set->paramInGemo(0));
    script->addConnect(node_col->paramOutGemo(0), col_set->paramInGemo(1));
    script->addConnect(node_value->paramOutGemo(0), col_set->paramInGemo(2));
    script->addConnect(func_start->flowOutGemo(0), col_set->flowInGemo());
}

void SampleRussian::addPaintEvent()
{
    auto func_inst = JZNodeFunctionManager::instance();

    auto meta = JZNodeObjectManager::instance()->meta("mainwindow");
    auto rect_meta = JZNodeObjectManager::instance()->meta("Rect");
    auto widget = JZNodeObjectManager::instance()->meta("Widget");    
    auto painter = JZNodeObjectManager::instance()->meta("Painter");

    m_script->addLocalVariable("painter", painter->id);

    JZNodeQtEvent *node_paint = new JZNodeQtEvent();
    node_paint->setEvent("mainwindow", meta->event("paintEvent"));
    m_script->addNode(JZNodePtr(node_paint));    

    JZNodeFunction *rect_create = new JZNodeFunction();
    rect_create->setFunction(rect_meta->function("create"));

    JZNodeFunction *painter_create = new JZNodeFunction();
    painter_create->setFunction(painter->function("create"));    

    JZNodeFunction *rect_create_big = new JZNodeFunction();
    rect_create_big->setFunction(rect_meta->function("create"));

    JZNodeFunction *draw_rect = new JZNodeFunction();
    draw_rect->setFunction(painter->function("drawRect"));

    JZNodeFunction *fill_rect = new JZNodeFunction();
    fill_rect->setFunction(painter->function("fillRect"));

    JZNodeSetParam *set = new JZNodeSetParam();
    JZNodeThis *node_this = new JZNodeThis();    

    set->setVariable("painter");

    m_script->addNode(JZNodePtr(set));
    m_script->addNode(JZNodePtr(painter_create));
    m_script->addNode(JZNodePtr(node_this));
    m_script->addNode(JZNodePtr(fill_rect));        

    //create create
    m_script->addConnect(node_this->paramOutGemo(0), painter_create->paramInGemo(0));
    m_script->addConnect(painter_create->paramOutGemo(0), set->paramInGemo(0));    
            
    m_script->addConnect(node_paint->flowOutGemo(), painter_create->flowInGemo());
    m_script->addConnect(painter_create->flowOutGemo(), set->flowInGemo());    
            
    JZNodeFor *for_row = new JZNodeFor();
    JZNodeFor *for_col = new JZNodeFor();
    for_row->setRange(0, m_row);
    for_col->setRange(0, m_col);

    m_script->addNode(JZNodePtr(for_row));
    m_script->addNode(JZNodePtr(for_col));    
    m_script->addNode(JZNodePtr(rect_create_big));
    m_script->addNode(JZNodePtr(draw_rect));
    
    int m_offsetX = 20;
    int m_offsetY = 10;
    rect_create_big->setPropValue(rect_create_big->paramIn(0), m_offsetX - 1);
    rect_create_big->setPropValue(rect_create_big->paramIn(1), m_offsetY - 1);
    rect_create_big->setPropValue(rect_create_big->paramIn(2), m_blockSize * m_col + 2);
    rect_create_big->setPropValue(rect_create_big->paramIn(3), m_blockSize * m_row + 2);
        
    m_script->addConnect(set->flowOutGemo(), draw_rect->flowInGemo());

    m_script->addConnect(set->paramOutGemo(0), draw_rect->paramInGemo(0));
    m_script->addConnect(rect_create_big->paramOutGemo(0), draw_rect->paramInGemo(1));    

    m_script->addConnect(draw_rect->flowOutGemo(), for_row->flowInGemo());
    m_script->addConnect(for_row->subFlowOutGemo(0), for_col->flowInGemo());

    JZNodeExpression *expr_row = new JZNodeExpression();
    JZNodeExpression *expr_col = new JZNodeExpression();
    m_script->addNode(JZNodePtr(expr_row));
    m_script->addNode(JZNodePtr(expr_col));
    m_script->addNode(JZNodePtr(rect_create));

    JZNodeParam *node_color_list = new JZNodeParam();
    node_color_list->setVariable("this.colors");

    JZNodeFunction *color_get = new JZNodeFunction();
    color_get->setFunction(func_inst->function("List.get"));

    m_script->addNode(JZNodePtr(node_color_list));
    m_script->addNode(JZNodePtr(color_get));    

    JZNodeParam *node_map = new JZNodeParam();
    node_map->setVariable("this.map");

    JZNodeFunction *row_get = new JZNodeFunction();
    row_get->setFunction(func_inst->function("List.get"));

    JZNodeFunction *col_get = new JZNodeFunction();
    col_get->setFunction(func_inst->function("List.get"));

    JZNodeBranch *branch = new JZNodeBranch();

    JZNodeGE *node_ge = new JZNodeGE();
    node_ge->setPropValue(node_ge->paramIn(1), 0);
    m_script->addNode(JZNodePtr(node_ge));
    m_script->addNode(JZNodePtr(branch));

    m_script->addNode(JZNodePtr(node_map));
    m_script->addNode(JZNodePtr(row_get));
    m_script->addNode(JZNodePtr(col_get));

    m_script->addConnect(node_map->paramOutGemo(0), row_get->paramInGemo(0));
    m_script->addConnect(for_row->paramOutGemo(0),row_get->paramInGemo(1));

    m_script->addConnect(row_get->paramOutGemo(0), col_get->paramInGemo(0));
    m_script->addConnect(for_col->paramOutGemo(0),col_get->paramInGemo(1));

    m_script->addConnect(node_color_list->paramOutGemo(0), color_get->paramInGemo(0));
    m_script->addConnect(col_get->paramOutGemo(0), color_get->paramInGemo(1));
    
    m_script->addConnect(col_get->paramOutGemo(0), node_ge->paramInGemo(0));
    m_script->addConnect(node_ge->paramOutGemo(0), branch->paramInGemo(0));

    QString error;
    QString exrp1 = QString::asprintf("y = row * %d + %d", m_blockSize, m_offsetY);
    QString exrp2 = QString::asprintf("x = col * %d + %d", m_blockSize, m_offsetX);
    bool ret1 = expr_row->setExpr(exrp1, error);
    bool ret2 = expr_col->setExpr(exrp2, error);
    Q_ASSERT(ret1 && ret2);

    m_script->addConnect(for_row->paramOutGemo(0), expr_row->paramInGemo(0));
    m_script->addConnect(for_col->paramOutGemo(0), expr_col->paramInGemo(0));    
    m_script->addConnect(expr_col->paramOutGemo(0), rect_create->paramInGemo(0));
    m_script->addConnect(expr_row->paramOutGemo(0), rect_create->paramInGemo(1));
    rect_create->setPropValue(rect_create->paramIn(2), m_blockSize);
    rect_create->setPropValue(rect_create->paramIn(3), m_blockSize);

    m_script->addConnect(for_col->subFlowOutGemo(0), branch->flowInGemo());
    m_script->addConnect(branch->flowOutGemo(0), fill_rect->flowInGemo());

    //fill rect    
    m_script->addConnect(set->paramOutGemo(0), fill_rect->paramInGemo(0));
    m_script->addConnect(rect_create->paramOutGemo(0), fill_rect->paramInGemo(1));    
    m_script->addConnect(color_get->paramOutGemo(0), fill_rect->paramInGemo(2));
}

void SampleRussian::addButtonClicked()
{
    auto btn_meta = JZNodeObjectManager::instance()->meta("PushButton");
    auto meta = JZNodeObjectManager::instance()->meta("mainwindow");

    //btnStart
    JZNodeSingleEvent *btnStart = new JZNodeSingleEvent();
    btnStart->setSingle(btn_meta->className, btn_meta->single("clicked"));
    btnStart->setVariable("this.btnStart");

    m_script->addNode(JZNodePtr(btnStart));    

    JZNodeParam *timer = new JZNodeParam();
    timer->setVariable("this.timer");

    JZNodeFunction *start = new JZNodeFunction();
    start->setFunction(JZNodeFunctionManager::instance()->function("Timer.start"));
    start->setPropValue(start->paramIn(1), 500);

    m_script->addNode(JZNodePtr(timer));
    m_script->addNode(JZNodePtr(start));

    m_script->addConnect(timer->paramOutGemo(0), start->paramInGemo(0));
    m_script->addConnect(btnStart->flowOutGemo(), start->flowInGemo());

    //*btnStop
    JZNodeSingleEvent *btnStop = new JZNodeSingleEvent();
    btnStop->setSingle(btn_meta->className, btn_meta->single("clicked"));
    btnStop->setVariable("this.btnStop");

    JZNodeFunction *stop = new JZNodeFunction();
    stop->setFunction(JZNodeFunctionManager::instance()->function("Timer.stop"));

    m_script->addNode(JZNodePtr(btnStop));
    m_script->addNode(JZNodePtr(stop));

    m_script->addConnect(timer->paramOutGemo(0), stop->paramInGemo(0));
    m_script->addConnect(btnStop->flowOutGemo(), stop->flowInGemo());
}

void SampleRussian::addCreateRect()
{
    auto mainwindow_id = JZNodeObjectManager::instance()->getClassId("mainwindow");    

    FunctionDefine define;
    define.name = "createRect";
    define.isFlowFunction = true;
    define.paramIn.push_back(JZParamDefine("obj", mainwindow_id));
    define.paramOut.push_back(JZParamDefine("ok", Type_bool));

    auto class_file = m_project.getClass("mainwindow");
    auto script = class_file->addMemberFunction(define);
    auto meta = JZNodeObjectManager::instance()->meta("mainwindow");

    JZNode *node_start = script->getNode(0);

    JZNodeSetParam *node_isRect = new JZNodeSetParam();
    node_isRect->setVariable("this.isRect");
    node_isRect->setPropValue(node_isRect->paramIn(0), true);

    JZNodeThis *node_this = new JZNodeThis();
    JZNodeParam *node_list_x = new JZNodeParam();
    node_list_x->setVariable("this.listCol");
    JZNodeParam *node_list_y = new JZNodeParam();
    node_list_y->setVariable("this.listRow");

    JZNodeReturn *node_ret = new JZNodeReturn();
    node_ret->setFunction(JZNodeFunctionManager::instance()->function("mainwindow.createRect"));
    script->addNode(JZNodePtr(node_ret));
    script->addNode(JZNodePtr(node_isRect));
    script->addNode(JZNodePtr(node_this));
    script->addNode(JZNodePtr(node_list_x));
    script->addNode(JZNodePtr(node_list_y));

    auto func_inst = JZNodeFunctionManager::instance();

    JZNodeFunction *set_list_row = new JZNodeFunction();
    set_list_row->setFunction(func_inst->function("List.push_back"));

    JZNodeFunction *set_list_col = new JZNodeFunction();
    set_list_col->setFunction(func_inst->function("List.push_back"));
    script->addNode(JZNodePtr(set_list_col));
    script->addNode(JZNodePtr(set_list_row));
    
    script->addConnect(node_start->flowOutGemo(), node_isRect->flowInGemo());

    script->addConnect(node_list_y->paramOutGemo(0), set_list_row->paramInGemo(0));
    set_list_row->setParamInValue(1,0);
    script->addConnect(node_isRect->flowOutGemo(), set_list_row->flowInGemo());

    script->addConnect(node_list_x->paramOutGemo(0), set_list_col->paramInGemo(0));
    set_list_col->setParamInValue(1,0);
    script->addConnect(set_list_row->flowOutGemo(), set_list_col->flowInGemo());
    
    //set map
    JZNodeFor *for_list = new JZNodeFor();
    for_list->setParamInValue(0, 0);
    for_list->setParamInValue(1, 1);

    JZNodeFunction *list_count = new JZNodeFunction();
    list_count->setFunction(func_inst->function("List.size"));    

    JZNodeFunction *get_x = new JZNodeFunction();
    get_x->setFunction(func_inst->function("List.get"));

    JZNodeFunction *get_y = new JZNodeFunction();
    get_y->setFunction(func_inst->function("List.get"));

    JZNodeFunction *set_map = new JZNodeFunction();
    set_map->setFunction(meta->function("setMap"));
    
    script->addNode(JZNodePtr(for_list));
    script->addNode(JZNodePtr(list_count));
    script->addNode(JZNodePtr(get_x));
    script->addNode(JZNodePtr(get_y));
    script->addNode(JZNodePtr(set_map));

    script->addConnect(set_list_col->flowOutGemo(), for_list->flowInGemo());

    script->addConnect(node_list_x->paramOutGemo(0), list_count->paramInGemo(0));
    script->addConnect(list_count->paramOutGemo(0), for_list->paramInGemo(2));
    script->addConnect(for_list->subFlowOutGemo(0), set_map->flowInGemo());

    script->addConnect(node_list_x->paramOutGemo(0), get_x->paramInGemo(0));
    script->addConnect(for_list->paramOutGemo(0), get_x->paramInGemo(1));
    script->addConnect(node_list_y->paramOutGemo(0), get_y->paramInGemo(0));
    script->addConnect(for_list->paramOutGemo(0), get_y->paramInGemo(1));
            
    script->addConnect(node_this->paramOutGemo(0), set_map->paramInGemo(0));
    script->addConnect(get_y->paramOutGemo(0), set_map->paramInGemo(1));
    script->addConnect(get_x->paramOutGemo(0), set_map->paramInGemo(2));
    set_map->setPropValue(set_map->paramIn(3), 3);

    //return
    node_ret->setPropValue(node_ret->paramIn(0), false);
    script->addConnect(for_list->flowOutGemo(), node_ret->flowInGemo());
}

void SampleRussian::addRectDown()
{
    auto mainwindow_id = JZNodeObjectManager::instance()->getClassId("mainwindow");
    auto func_inst = JZNodeFunctionManager::instance();

    FunctionDefine define;
    define.name = "rectDown";
    define.isFlowFunction = true;
    define.paramIn.push_back(JZParamDefine("obj", mainwindow_id));
    define.paramOut.push_back(JZParamDefine("ok", Type_bool));

    auto class_file = m_project.getClass("mainwindow");    
    auto script = class_file->addMemberFunction(define);
    script->addLocalVariable("value", Type_int);

    auto meta = JZNodeObjectManager::instance()->meta("mainwindow");

    JZNodeThis *node_this = new JZNodeThis();
    JZNode *node_start = script->getNode(0);

    JZNodeReturn *node_ret = new JZNodeReturn();
    node_ret->setFunction(meta->function("rectDown"));

    JZNodeFunction *canDown = new JZNodeFunction();
    canDown->setFunction(meta->function("canMoveDown"));

    JZNodeFunction *moveDown= new JZNodeFunction();
    moveDown->setFunction(meta->function("moveDown"));

    JZNodeBranch *branch = new JZNodeBranch();    
    script->addNode(JZNodePtr(node_this));
    script->addNode(JZNodePtr(canDown));
    script->addNode(JZNodePtr(moveDown));
    script->addNode(JZNodePtr(branch));
    script->addNode(JZNodePtr(node_ret));

    script->addConnect(node_this->paramOutGemo(0), canDown->paramInGemo(0));
    script->addConnect(node_this->paramOutGemo(0), moveDown->paramInGemo(0));

    // if canDown
    script->addConnect(canDown->paramOutGemo(0), branch->paramInGemo(0));
    script->addConnect(node_start->flowOutGemo(0), branch->flowInGemo());
    script->addConnect(branch->flowOutGemo(0), moveDown->flowInGemo());
    
    // else    
    JZNodeFor *for_list = new JZNodeFor();
    for_list->setParamInValue(0, 0);
    for_list->setParamInValue(1, 1);

    JZNodeFunction *get_x = new JZNodeFunction();
    get_x->setFunction(func_inst->function("List.get"));

    JZNodeFunction *get_y = new JZNodeFunction();
    get_y->setFunction(func_inst->function("List.get"));

    JZNodeFunction *clear_x = new JZNodeFunction();
    clear_x->setFunction(func_inst->function("List.clear"));

    JZNodeFunction *clear_y = new JZNodeFunction();
    clear_y->setFunction(func_inst->function("List.clear"));
        
    JZNodeParam *node_list_x = new JZNodeParam();
    node_list_x->setVariable("this.listCol");
    JZNodeParam *node_list_y = new JZNodeParam();
    node_list_y->setVariable("this.listRow");

    JZNodeFunction *list_count = new JZNodeFunction();
    list_count->setFunction(func_inst->function("List.size"));

    JZNodeSetParam *set_if_rect = new JZNodeSetParam();
    set_if_rect->setVariable("this.isRect");
    set_if_rect->setParamInValue(0, false);

    JZNodeFunction *set_map = new JZNodeFunction();
    set_map->setFunction(meta->function("setMap"));

    script->addNode(JZNodePtr(for_list));
    script->addNode(JZNodePtr(get_x));
    script->addNode(JZNodePtr(get_y));
    script->addNode(JZNodePtr(clear_x));
    script->addNode(JZNodePtr(clear_y));    
    script->addNode(JZNodePtr(node_list_x));
    script->addNode(JZNodePtr(node_list_y));
    script->addNode(JZNodePtr(list_count));
    script->addNode(JZNodePtr(set_if_rect));
    script->addNode(JZNodePtr(set_map));

    script->addConnect(branch->flowOutGemo(1), for_list->flowInGemo());
    script->addConnect(node_list_x->paramOutGemo(0), list_count->paramInGemo(0));
    script->addConnect(list_count->paramOutGemo(0), for_list->paramInGemo(2));

    script->addConnect(node_list_x->paramOutGemo(0), get_x->paramInGemo(0));
    script->addConnect(for_list->paramOutGemo(0), get_x->paramInGemo(1));
    script->addConnect(node_list_y->paramOutGemo(0), get_y->paramInGemo(0));
    script->addConnect(for_list->paramOutGemo(0), get_y->paramInGemo(1));

    script->addConnect(node_list_x->paramOutGemo(0), clear_x->paramInGemo(0));
    script->addConnect(node_list_y->paramOutGemo(0), clear_y->paramInGemo(0));

    script->addConnect(for_list->subFlowOutGemo(0), set_map->flowInGemo());

    script->addConnect(node_this->paramOutGemo(0), set_map->paramInGemo(0));
    script->addConnect(get_y->paramOutGemo(0), set_map->paramInGemo(1));
    script->addConnect(get_x->paramOutGemo(0), set_map->paramInGemo(2));
    set_map->setParamInValue(3, 0);

    script->addConnect(for_list->flowOutGemo(0), clear_x->flowInGemo());
    script->addConnect(clear_x->flowOutGemo(0), clear_y->flowInGemo());
    script->addConnect(clear_y->flowOutGemo(0), set_if_rect->flowInGemo());

    // return
    node_ret->setPropValue(node_ret->paramIn(0), false);        
    script->addConnect(moveDown->flowOutGemo(), node_ret->flowInGemo());
    script->addConnect(set_if_rect->flowOutGemo(), node_ret->flowInGemo());
}

void SampleRussian::addMoveFunction()
{
    auto mainwindow_id = JZNodeObjectManager::instance()->getClassId("mainwindow");
    auto class_file = m_project.getClass("mainwindow");
    QStringList functions = { "canMoveDown","canMoveLeft", "canMoveRight" , "moveDown", "moveLeft", "moveRight" };

    for (int i = 0; i < functions.size(); i++)
    {
        bool isCan = functions[i].startsWith("can");

        FunctionDefine define;
        define.name = functions[i];
        define.isFlowFunction = !isCan;
        define.paramIn.push_back(JZParamDefine("obj", mainwindow_id));
        if(isCan)
            define.paramOut.push_back(JZParamDefine("ok", Type_bool));
        
        class_file->addMemberFunction(define);
    }   
    auto meta = JZNodeObjectManager::instance()->meta("mainwindow");
    auto func_inst = JZNodeFunctionManager::instance();

    for (int i = 0; i < functions.size(); i++)
    {
        QString function_name = functions[i];
        bool isCan = functions[i].startsWith("can");
        auto script = class_file->getMemberFunction(functions[i]);
        JZNode *node_start = script->getNode(0);

        JZNodeThis *node_this = new JZNodeThis();
        JZNodeParam *node_list_x = new JZNodeParam();
        node_list_x->setVariable("this.listCol");
        JZNodeParam *node_list_y = new JZNodeParam();
        node_list_y->setVariable("this.listRow");
                
        JZNodeFunction *list_count = new JZNodeFunction();
        list_count->setFunction(func_inst->function("List.size"));

        JZNodeFunction *get_map = new JZNodeFunction();
        get_map->setFunction(meta->function("getMap"));

        JZNodeFunction *get_x = new JZNodeFunction();
        get_x->setFunction(func_inst->function("List.get"));

        JZNodeFunction *get_y = new JZNodeFunction();
        get_y->setFunction(func_inst->function("List.get"));

        JZNodeFor *for_list = new JZNodeFor();
        for_list->setParamInValue(0, 0);
        for_list->setParamInValue(1, 1);

        script->addNode(JZNodePtr(node_this));
        script->addNode(JZNodePtr(node_list_y));
        script->addNode(JZNodePtr(node_list_x));
        script->addNode(JZNodePtr(list_count));
        script->addNode(JZNodePtr(for_list));
        script->addNode(JZNodePtr(get_map));
        script->addNode(JZNodePtr(get_x));
        script->addNode(JZNodePtr(get_y));

        script->addConnect(node_start->flowOutGemo(), for_list->flowInGemo());
        script->addConnect(node_list_x->paramOutGemo(0), list_count->paramInGemo(0));
        script->addConnect(list_count->paramOutGemo(0), for_list->paramInGemo(2));            
    
        script->addConnect(node_list_x->paramOutGemo(0), get_x->paramInGemo(0));
        script->addConnect(for_list->paramOutGemo(0), get_x->paramInGemo(1));
        script->addConnect(node_list_y->paramOutGemo(0), get_y->paramInGemo(0));
        script->addConnect(for_list->paramOutGemo(0), get_y->paramInGemo(1));

        script->addConnect(node_this->paramOutGemo(0), get_map->paramInGemo(0));
        

        if (isCan)
        {
            JZNodeReturn *node_return1 = new JZNodeReturn();
            node_return1->setFunction(meta->function(functions[i]));
            JZNodeReturn *node_return2 = new JZNodeReturn();
            node_return2->setFunction(meta->function(functions[i]));
            script->addNode(JZNodePtr(node_return1));
            script->addNode(JZNodePtr(node_return2));

            //if(row + 1 < 20 && getMap(row+1,col) == -1)
            JZNodeBranch *branch = new JZNodeBranch();
            script->addNode(JZNodePtr(branch));            

            script->addConnect(for_list->subFlowOutGemo(0), branch->flowInGemo());


            JZNodeAdd *node_add = new JZNodeAdd();
            script->addNode(JZNodePtr(node_add));
            if (functions[i] == "canMoveDown" || functions[i] == "canMoveRight")
                node_add->setParamInValue(1, 1);
            else
                node_add->setParamInValue(1, -1);
            if (functions[i] == "canMoveDown")
            {
                script->addConnect(get_y->paramOutGemo(0), node_add->paramInGemo(0));

                script->addConnect(node_add->paramOutGemo(0), get_map->paramInGemo(1));
                script->addConnect(get_x->paramOutGemo(0), get_map->paramInGemo(2));
            }
            else
            {
                script->addConnect(get_x->paramOutGemo(0), node_add->paramInGemo(0));

                script->addConnect(get_y->paramOutGemo(0), get_map->paramInGemo(1));
                script->addConnect(node_add->paramOutGemo(0), get_map->paramInGemo(2));
            }

            JZNode *node_gt = nullptr;
            if (functions[i] == "canMoveDown" || functions[i] == "canMoveRight")
            {
                node_gt = new JZNodeGE();
                if (functions[i] == "canMoveDown")
                    node_gt->setParamInValue(1, m_row);
                else
                    node_gt->setParamInValue(1, m_col);
            }
            else
            {
                node_gt = new JZNodeLT();
                node_gt->setParamInValue(1, 0);
            }
            JZNodeNE *node_ne = new JZNodeNE();
            JZNodeOr *node_or = new JZNodeOr();
            script->addNode(JZNodePtr(node_gt));
            script->addNode(JZNodePtr(node_ne));
            script->addNode(JZNodePtr(node_or));

            script->addConnect(node_add->paramOutGemo(0), node_gt->paramInGemo(0));            

            script->addConnect(get_map->paramOutGemo(0), node_ne->paramInGemo(0));
            node_ne->setParamInValue(1, -1);

            script->addConnect(node_gt->paramOutGemo(0), node_or->paramInGemo(0));
            script->addConnect(node_ne->paramOutGemo(0), node_or->paramInGemo(1));
            script->addConnect(node_or->paramOutGemo(0), branch->paramInGemo(0));

            node_return1->setParamInValue(0, false);
            script->addConnect(branch->flowOutGemo(0), node_return1->flowInGemo());

            //return true
            node_return2->setParamInValue(0, true);
            script->addConnect(for_list->flowOutGemo(0), node_return2->flowInGemo());
        }
        else
        {
            script->addConnect(get_y->paramOutGemo(0), get_map->paramInGemo(1));
            script->addConnect(get_x->paramOutGemo(0), get_map->paramInGemo(2));

            JZNodeFunction *set_map1 = new JZNodeFunction();
            set_map1->setFunction(meta->function("setMap"));

            JZNodeFunction *set_map2 = new JZNodeFunction();
            set_map2->setFunction(meta->function("setMap"));

            JZNodeAdd *node_row_add = new JZNodeAdd();
            
            JZNodeFunction *set_list = new JZNodeFunction();
            set_list->setFunction(func_inst->function("List.set"));

            script->addNode(JZNodePtr(set_map1));
            script->addNode(JZNodePtr(set_map2));
            script->addNode(JZNodePtr(node_row_add));
            script->addNode(JZNodePtr(set_list));

            script->addConnect(node_this->paramOutGemo(0), set_map1->paramInGemo(0));
            script->addConnect(get_y->paramOutGemo(0), set_map1->paramInGemo(1));
            script->addConnect(get_x->paramOutGemo(0), set_map1->paramInGemo(2));
            set_map1->setParamInValue(3, -1);
            script->addConnect(for_list->subFlowOutGemo(0), set_map1->flowInGemo());            
                        
            if(function_name == "moveLeft")
                node_row_add->setParamInValue(1, -1);
            else
                node_row_add->setParamInValue(1, 1);
            if (function_name == "moveDown")
            {
                script->addConnect(get_y->paramOutGemo(0), node_row_add->paramInGemo(0));                
                script->addConnect(node_row_add->paramOutGemo(0), set_map2->paramInGemo(1));
                script->addConnect(get_x->paramOutGemo(0), set_map2->paramInGemo(2));

                script->addConnect(node_list_y->paramOutGemo(0), set_list->paramInGemo(0));
            }
            else
            {
                script->addConnect(get_x->paramOutGemo(0), node_row_add->paramInGemo(0));
                script->addConnect(get_y->paramOutGemo(0), set_map2->paramInGemo(1));
                script->addConnect(node_row_add->paramOutGemo(0), set_map2->paramInGemo(2));

                script->addConnect(node_list_x->paramOutGemo(0), set_list->paramInGemo(0));
            }

            script->addConnect(node_this->paramOutGemo(0), set_map2->paramInGemo(0));            
            set_map2->setParamInValue(3, 3);
            script->addConnect(set_map1->flowOutGemo(0), set_map2->flowInGemo());
                        
            script->addConnect(for_list->paramOutGemo(0), set_list->paramInGemo(1));
            script->addConnect(node_row_add->paramOutGemo(0), set_list->paramInGemo(2));
            script->addConnect(set_map2->flowOutGemo(0), set_list->flowInGemo());
        }
    }    
}

void SampleRussian::addGameLoop()
{
    auto timer_meta = JZNodeObjectManager::instance()->meta("Timer");
    auto meta = JZNodeObjectManager::instance()->meta("mainwindow");

    JZNodeSingleEvent *onTimer = new JZNodeSingleEvent();
    onTimer->setSingle(timer_meta->className,timer_meta->single("timeout"));
    onTimer->setVariable("this.timer");

    m_script->addNode(JZNodePtr(onTimer));

    JZNodeThis *node_this = new JZNodeThis();

    JZNodeParam *is_rect = new JZNodeParam();
    is_rect->setVariable("this.isRect");

    JZNodeBranch *is_rect_branch = new JZNodeBranch();
    m_script->addNode(JZNodePtr(is_rect));
    m_script->addNode(JZNodePtr(is_rect_branch));

    m_script->addConnect(is_rect->paramOutGemo(0), is_rect_branch->paramInGemo(0));
    m_script->addConnect(onTimer->flowOutGemo(), is_rect_branch->flowInGemo());
    
    JZNodeFunction *rect_down = new JZNodeFunction();    
    rect_down->setFunction(meta->function("rectDown"));

    m_script->addNode(JZNodePtr(node_this));    
    m_script->addNode(JZNodePtr(rect_down));

    m_script->addConnect(node_this->paramOutGemo(0), rect_down->paramInGemo(0));
    m_script->addConnect(is_rect_branch->flowOutGemo(0), rect_down->flowInGemo());    

    JZNodeFunction *create_rect = new JZNodeFunction();
    create_rect->setFunction(meta->function("createRect"));
    m_script->addNode(JZNodePtr(create_rect));

    m_script->addConnect(node_this->paramOutGemo(0), create_rect->paramInGemo(0));
    m_script->addConnect(is_rect_branch->flowOutGemo(1), create_rect->flowInGemo());    

    JZNodeFunction *update = new JZNodeFunction();
    update->setFunction(JZNodeFunctionManager::instance()->function("Widget.update"));
    m_script->addNode(JZNodePtr(update));

    JZNodeParam *timer = new JZNodeParam();
    timer->setVariable("this.timer");
    m_script->addNode(JZNodePtr(timer));

    JZNodeFunction *stop_timer = new JZNodeFunction();
    stop_timer->setFunction(JZNodeFunctionManager::instance()->function("Timer.stop"));
    m_script->addNode(JZNodePtr(stop_timer));    

    m_script->addConnect(rect_down->flowOutGemo(0), update->flowInGemo());
    m_script->addConnect(create_rect->flowOutGemo(0), update->flowInGemo());

    //is game end
    JZNodeBranch *is_game_end = new JZNodeBranch();
    m_script->addNode(JZNodePtr(is_game_end));
    m_script->addConnect(rect_down->paramOutGemo(0), is_game_end->paramInGemo(0));
    m_script->addConnect(create_rect->paramOutGemo(0), is_game_end->paramInGemo(0));

    m_script->addConnect(node_this->paramOutGemo(0), update->paramInGemo(0));
    m_script->addConnect(update->flowOutGemo(0), is_game_end->flowInGemo());

    JZNodeFunction *message = new JZNodeFunction();
    message->setFunction(JZNodeFunctionManager::instance()->function("MessageBox.information"));
    message->setPropValue(message->paramIn(1), "Game over");    
    m_script->addNode(JZNodePtr(message));

    m_script->addConnect(timer->paramOutGemo(0), stop_timer->paramInGemo(0));
    m_script->addConnect(is_game_end->flowOutGemo(), stop_timer->flowInGemo());
    m_script->addConnect(node_this->paramOutGemo(0), message->paramInGemo(0));
    m_script->addConnect(stop_timer->flowOutGemo(), message->flowInGemo());
    
}

void SampleRussian::addKeyEvent()
{
    auto func_inst = JZNodeFunctionManager::instance();

    auto meta = JZNodeObjectManager::instance()->meta("mainwindow");    
    auto key_meta = JZNodeObjectManager::instance()->meta("KeyEvent");    

    JZNodeQtEvent *node_keyPress = new JZNodeQtEvent();
    node_keyPress->setEvent("mainwindow", meta->event("keyPressEvent"));
    m_script->addNode(JZNodePtr(node_keyPress));

    JZNodeThis *node_this = new JZNodeThis();

    JZNodeFunction *canRight = new JZNodeFunction();
    canRight->setFunction(meta->function("canMoveRight"));

    JZNodeFunction *moveRight = new JZNodeFunction();
    moveRight->setFunction(meta->function("moveRight"));

    JZNodeBranch *branch_right = new JZNodeBranch();


    JZNodeFunction *key_code = new JZNodeFunction();
    key_code->setFunction(key_meta->function("key"));

    JZNodeEQ *key_eq = new JZNodeEQ();
    
    m_script->addNode(JZNodePtr(node_this));
    m_script->addNode(JZNodePtr(canRight));
    m_script->addNode(JZNodePtr(moveRight));
    m_script->addNode(JZNodePtr(branch_right));
    m_script->addNode(JZNodePtr(key_code));
    m_script->addNode(JZNodePtr(key_eq));

    m_script->addConnect(node_keyPress->paramOutGemo(0), key_code->paramInGemo(0));
    m_script->addConnect(key_code->paramOutGemo(0), key_eq->paramInGemo(0));
    key_eq->setPropValue(1, Qt::Key_D);

    m_script->addConnect(node_this->paramOutGemo(0), canRight->paramInGemo(0));
    m_script->addConnect(node_this->paramOutGemo(0), moveRight->paramInGemo(0));

    m_script->addConnect(node_keyPress->flowOutGemo(0), branch_right->flowInGemo());

    m_script->addConnect(key_eq->paramOutGemo(0), branch_right->paramInGemo(0));
    //m_script->addConnect(canRight->paramOutGemo(0), branch_right->paramInGemo(0));
    m_script->addConnect(branch_right->flowOutGemo(0), moveRight->flowInGemo());
}