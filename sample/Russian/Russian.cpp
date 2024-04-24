#include <QApplication>
#include <QDir>
#include "Russian.h"
#include "JZNodeBuilder.h"
#include "JZNodeVM.h"
#include "UiCommon.h"
#include "JZNodeFunction.h"
#include "JZNodeValue.h"
#include "JZUiFile.h"
#include "JZNodeView.h"
#include "JZNodeUtils.h"

SampleRussian::SampleRussian()
{
    newProject("russian");        

    JZUiFile *ui_file = dynamic_cast<JZUiFile*>(m_project.getItem("./mainwindow.ui"));
    ui_file->setXml(loadUi("Russian.ui"));
    m_project.saveItem(ui_file);

    auto class_file = m_project.getClass("MainWindow");
    m_script = class_file->flow("�¼�");

    class_file->addMemberVariable("map", Type_list);
    class_file->addMemberVariable("colors", Type_list);
    class_file->addMemberVariable("timer", Type_timer);
    class_file->addMemberVariable("isRect", Type_bool);
    
    class_file->addMemberVariable("shape", Type_list);
    class_file->addMemberVariable("shape_index", Type_int);
    class_file->addMemberVariable("shape_row", Type_int);
    class_file->addMemberVariable("shape_col", Type_int);
    class_file->addMemberVariable("shape_color", Type_int);
    class_file->addMemberVariable("score", Type_int);

    JZNodeParamBind info;
    info.variable = "score";
    info.widget = "lineScore";
    info.dir = JZNodeParamBind::DataToUi;
    class_file->getParamFile()->addBind(info);
    
    addInitGame();
    addInitFunction();
    addMapGet();
    addMapSet();
    addClearLine();
    addCanPlaceShape();
    addCreateShape();
    addRotate();    
    addMoveFunction();
    addPaintEvent();    
    addCreateRect();
    addRectDown();
    addButtonClicked();        
    addGameLoop();
    addKeyEvent();
}

SampleRussian::~SampleRussian()
{

}

void SampleRussian::addInitGame()
{
    auto func_inst = JZNodeFunctionManager::instance();
    auto color_meta = JZNodeObjectManager::instance()->meta("Color");
    auto list_meta = JZNodeObjectManager::instance()->meta("List");

    auto class_file = m_project.getClass("MainWindow");
    auto mainwindow_meta = JZNodeObjectManager::instance()->meta("MainWindow");

    JZFunctionDefine define;
    define.name = "initGame";
    define.isFlowFunction = true;
    define.paramIn.push_back(JZParamDefine("this", mainwindow_meta->id));
    auto script = class_file->addMemberFunction(define);
    script->addLocalVariable(JZParamDefine("col_list", Type_list));

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

    JZNodeSetParam *set_score = new JZNodeSetParam();
    set_score->setVariable("this.score");
    set_score->setParamInValue(1, "0");

    script->addNode(set_map);
    script->addNode(set_color_list);
    script->addNode(create_map);
    script->addNode(create_color_list);
    script->addNode(node_color_list);
    script->addNode(set_score);

    script->addConnect(func_start->flowOutGemo(0), set_map->flowInGemo());
    script->addConnect(create_map->paramOutGemo(0), set_map->paramInGemo(1));    
    script->addConnect(set_map->flowOutGemo(0), set_score->flowInGemo());
    script->addConnect(set_score->flowOutGemo(), set_color_list->flowInGemo());
    script->addConnect(create_color_list->paramOutGemo(0), set_color_list->paramInGemo(1));

    JZNodeFunction *color_resize = new JZNodeFunction();
    color_resize->setFunction(func_inst->function("List.resize"));
    script->addNode(color_resize);

    script->addConnect(node_color_list->paramOutGemo(0), color_resize->paramInGemo(0));
    color_resize->setPinValue(color_resize->paramIn(1), "5");
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
        color_create->setPinValue(color_create->paramIn(0), QString::number(r));
        color_create->setPinValue(color_create->paramIn(1), QString::number(g));
        color_create->setPinValue(color_create->paramIn(2), QString::number(b));

        JZNodeFunction *list_set = new JZNodeFunction();
        list_set->setFunction(list_meta->function("set"));
        script->addNode(color_create);
        script->addNode(list_set);

        script->addConnect(pre_node->flowOutGemo(0), list_set->flowInGemo());
        script->addConnect(node_color_list->paramOutGemo(0), list_set->paramInGemo(0));
        list_set->setPinValue(list_set->paramIn(1), QString::number(i));
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

    JZNodeSetParam *set_col = new JZNodeSetParam();
    JZNodeParam *get_col = new JZNodeParam();
    set_col->setVariable("col_list");
    get_col->setVariable("col_list");

    JZNodeFunction *col_push = new JZNodeFunction();
    col_push->setFunction(func_inst->function("List.push_back"));
    col_push->setPinValue(col_push->paramIn(1), QString::number(-1));

    script->addNode(node_map);
    script->addNode(for_col);
    script->addNode(row_push);
    script->addNode(for_row);
    script->addNode(create_col);
    script->addNode(col_push);
    script->addNode(set_col);
    script->addNode(get_col);
    script->addConnect(pre_node->flowOutGemo(0), for_row->flowInGemo());

    script->addConnect(for_row->subFlowOutGemo(0), set_col->flowInGemo());
    
    script->addConnect(create_col->paramOutGemo(0), set_col->paramInGemo(1));
    script->addConnect(set_col->flowOutGemo(0), for_col->flowInGemo());

    script->addConnect(get_col->paramOutGemo(0), col_push->paramInGemo(0));
    script->addConnect(for_col->subFlowOutGemo(0), col_push->flowInGemo());

    script->addConnect(node_map->paramOutGemo(0), row_push->paramInGemo(0));
    script->addConnect(get_col->paramOutGemo(0), row_push->paramInGemo(1));
    script->addConnect(for_col->flowOutGemo(0), row_push->flowInGemo());

    JZNodeSetParam *node_isRect = new JZNodeSetParam();
    node_isRect->setVariable("this.isRect");
    node_isRect->setPinValue(node_isRect->paramIn(1), "false");
    script->addNode(node_isRect);

    script->addConnect(for_row->flowOutGemo(), node_isRect->flowInGemo());
}

void SampleRussian::addInitFunction()
{
    auto func_inst = JZNodeFunctionManager::instance();
    auto color_meta = JZNodeObjectManager::instance()->meta("Color");
    auto list_meta = JZNodeObjectManager::instance()->meta("List");

    auto class_file = m_project.getClass("MainWindow");
    auto mainwindow_meta = JZNodeObjectManager::instance()->meta("MainWindow");
    
    auto script = class_file->memberFunction("init");               

    JZNodeFunction *init_game = new JZNodeFunction();
    init_game->setFunction(mainwindow_meta->function("initGame"));

    JZNode *func_start = script->getNode(0);
    
    JZNodeCreate *create_timer = new JZNodeCreate();
    create_timer->setClassName("Timer");

    JZNodeSetParam *set_timer = new JZNodeSetParam();
    set_timer->setVariable("this.timer");

    script->addNode(init_game);
    script->addNode(create_timer);
    script->addNode(set_timer);

    script->addConnect(func_start->flowOutGemo(0), init_game->flowInGemo());
    script->addConnect(init_game->flowOutGemo(0), set_timer->flowInGemo());
    script->addConnect(create_timer->paramOutGemo(0), set_timer->paramInGemo(1));
}

void SampleRussian::addMapGet()
{
    auto func_inst = JZNodeFunctionManager::instance();
    auto mainwindow_id = JZNodeObjectManager::instance()->getClassId("MainWindow");
    auto class_file = m_project.getClass("MainWindow");

    JZFunctionDefine define;
    define.name = "getMap";
    define.isFlowFunction = false;
    define.paramIn.push_back(JZParamDefine("this", mainwindow_id));
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

    script->addNode(node_row);
    script->addNode(node_col);
    script->addNode(node_map);
    script->addNode(row_get);
    script->addNode(col_get);
    script->addNode(node_return);
        
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
    auto mainwindow_id = JZNodeObjectManager::instance()->getClassId("MainWindow");
    auto class_file = m_project.getClass("MainWindow");

    JZFunctionDefine define;
    define.name = "setMap";
    define.isFlowFunction = true;
    define.paramIn.push_back(JZParamDefine("this", mainwindow_id));
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
    
    script->addNode(node_row);
    script->addNode(node_col);
    script->addNode(node_map);
    script->addNode(row_get);
    script->addNode(col_set);    
    script->addNode(node_value);

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

    auto meta = JZNodeObjectManager::instance()->meta("MainWindow");
    auto rect_meta = JZNodeObjectManager::instance()->meta("Rect");
    auto widget = JZNodeObjectManager::instance()->meta("Widget");    
    auto painter = JZNodeObjectManager::instance()->meta("Painter");

    m_script->addLocalVariable(JZParamDefine("painter", "Painter"));

    JZNodeQtEvent *node_paint = new JZNodeQtEvent();
    node_paint->setEvent("MainWindow", meta->event("paintEvent"));
    m_script->addNode(node_paint);    

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

    m_script->addNode(set);
    m_script->addNode(painter_create);
    m_script->addNode(node_this);
    m_script->addNode(fill_rect);        

    //create create
    m_script->addConnect(node_this->paramOutGemo(0), painter_create->paramInGemo(0));
    m_script->addConnect(painter_create->paramOutGemo(0), set->paramInGemo(1));    
            
    m_script->addConnect(node_paint->flowOutGemo(), painter_create->flowInGemo());
    m_script->addConnect(painter_create->flowOutGemo(), set->flowInGemo());    
            
    JZNodeFor *for_row = new JZNodeFor();
    JZNodeFor *for_col = new JZNodeFor();
    for_row->setRange(0, m_row);
    for_col->setRange(0, m_col);

    m_script->addNode(for_row);
    m_script->addNode(for_col);    
    m_script->addNode(rect_create_big);
    m_script->addNode(draw_rect);
    
    int m_offsetX = 20;
    int m_offsetY = 10;
    rect_create_big->setPinValue(rect_create_big->paramIn(0), QString::number(m_offsetX - 1));
    rect_create_big->setPinValue(rect_create_big->paramIn(1), QString::number(m_offsetY - 1));
    rect_create_big->setPinValue(rect_create_big->paramIn(2), QString::number(m_blockSize * m_col + 2));
    rect_create_big->setPinValue(rect_create_big->paramIn(3), QString::number(m_blockSize * m_row + 2));
        
    m_script->addConnect(set->flowOutGemo(), draw_rect->flowInGemo());

    m_script->addConnect(set->paramOutGemo(0), draw_rect->paramInGemo(0));
    m_script->addConnect(rect_create_big->paramOutGemo(0), draw_rect->paramInGemo(1));    

    m_script->addConnect(draw_rect->flowOutGemo(), for_row->flowInGemo());
    m_script->addConnect(for_row->subFlowOutGemo(0), for_col->flowInGemo());

    JZNodeExpression *expr_row = new JZNodeExpression();
    JZNodeExpression *expr_col = new JZNodeExpression();
    m_script->addNode(expr_row);
    m_script->addNode(expr_col);
    m_script->addNode(rect_create);

    JZNodeParam *node_color_list = new JZNodeParam();
    node_color_list->setVariable("this.colors");

    JZNodeFunction *color_get = new JZNodeFunction();
    color_get->setFunction(func_inst->function("List.get"));

    m_script->addNode(node_color_list);
    m_script->addNode(color_get);    

    JZNodeParam *node_map = new JZNodeParam();
    node_map->setVariable("this.map");

    JZNodeFunction *row_get = new JZNodeFunction();
    row_get->setFunction(func_inst->function("List.get"));

    JZNodeFunction *col_get = new JZNodeFunction();
    col_get->setFunction(func_inst->function("List.get"));

    JZNodeBranch *branch = new JZNodeBranch();

    JZNodeGE *node_ge = new JZNodeGE();
    node_ge->setPinValue(node_ge->paramIn(1), "0");
    m_script->addNode(node_ge);
    m_script->addNode(branch);

    m_script->addNode(node_map);
    m_script->addNode(row_get);
    m_script->addNode(col_get);

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
    rect_create->setPinValue(rect_create->paramIn(2), QString::number(m_blockSize));
    rect_create->setPinValue(rect_create->paramIn(3), QString::number(m_blockSize));

    m_script->addConnect(for_col->subFlowOutGemo(0), branch->flowInGemo());
    m_script->addConnect(branch->flowOutGemo(0), fill_rect->flowInGemo());

    //fill rect    
    m_script->addConnect(set->paramOutGemo(0), fill_rect->paramInGemo(0));
    m_script->addConnect(rect_create->paramOutGemo(0), fill_rect->paramInGemo(1));    
    m_script->addConnect(color_get->paramOutGemo(0), fill_rect->paramInGemo(2));

    //draw shape
    JZNodeParam *shape = new JZNodeParam();
    shape->setVariable("this.shape");

    JZNodeParam *index = new JZNodeParam();
    index->setVariable("this.shape_index");

    JZNodeParam *shape_color = new JZNodeParam();
    shape_color->setVariable("this.shape_color");

    JZNodeFor *node_for = new JZNodeFor();

    JZNodeFunction *get_list = new JZNodeFunction();
    get_list->setFunction(func_inst->function("List.get"));

    JZNodeFunction *get_point = new JZNodeFunction();
    get_point->setFunction(func_inst->function("List.get"));

    JZNodeFunction *get_list_size = new JZNodeFunction();
    get_list_size->setFunction(func_inst->function("List.size"));

    JZNodeFunction *point_row = new JZNodeFunction();
    point_row->setFunction(func_inst->function("Point.x"));

    JZNodeFunction *point_col = new JZNodeFunction();
    point_col->setFunction(func_inst->function("Point.y"));    

    JZNodeParam *shape_row = new JZNodeParam();
    shape_row->setVariable("this.shape_row");

    JZNodeParam *shape_col = new JZNodeParam();
    shape_col->setVariable("this.shape_col");

    JZNodeAdd *shape_row_add = new JZNodeAdd();
    JZNodeAdd *shape_col_add = new JZNodeAdd();

    JZNodeEQ *shape_null_cmp = new JZNodeEQ();

    JZNodeBranch *shape_null = new JZNodeBranch();
    JZNodeReturn *shape_null_return = new JZNodeReturn();
    m_script->addNode(shape_null_cmp);
    m_script->addNode(shape_null);
    m_script->addNode(shape_null_return);
    m_script->addNode(node_for);
    m_script->addNode(shape_color);
    m_script->addNode(get_list);
    m_script->addNode(get_point);
    m_script->addNode(get_list_size);
    m_script->addNode(point_row);
    m_script->addNode(point_col);
    m_script->addNode(shape);
    m_script->addNode(index);

    m_script->addNode(shape_row);
    m_script->addNode(shape_col);
    m_script->addNode(shape_row_add);
    m_script->addNode(shape_col_add);

    JZNodeBranch *branch_is_rect = new JZNodeBranch();

    JZNodeParam *is_rect = new JZNodeParam();
    is_rect->setVariable("this.isRect");
    m_script->addNode(is_rect);
    m_script->addNode(branch_is_rect);

    m_script->addConnect(is_rect->paramOutGemo(0), branch_is_rect->paramInGemo(0));
    m_script->addConnect(for_row->flowOutGemo(), branch_is_rect->flowInGemo());
    m_script->addConnect(branch_is_rect->flowOutGemo(), shape_null->flowInGemo());
    m_script->addConnect(shape->paramOutGemo(0), shape_null_cmp->paramInGemo(0));

    JZNodeLiteral *node_nullptr = new JZNodeLiteral();
    node_nullptr->setDataType(Type_nullptr);
    m_script->addNode(node_nullptr);
    m_script->addConnect(node_nullptr->paramOutGemo(0), shape_null_cmp->paramInGemo(1));
    
    m_script->addConnect(shape_null_cmp->paramOutGemo(0), shape_null->paramInGemo(0));
    m_script->addConnect(shape_null->flowOutGemo(0), shape_null_return->flowInGemo());
    m_script->addConnect(shape_null->flowOutGemo(1), node_for->flowInGemo());

    node_for->setParamInValue(0, "0");
    node_for->setParamInValue(1, "1");
    m_script->addConnect(get_list->paramOutGemo(0), get_list_size->paramInGemo(0));
    m_script->addConnect(get_list_size->paramOutGemo(0), node_for->paramInGemo(2));

    m_script->addConnect(shape->paramOutGemo(0), get_list->paramInGemo(0));
    m_script->addConnect(index->paramOutGemo(0), get_list->paramInGemo(1));

    m_script->addConnect(get_list->paramOutGemo(0), get_point->paramInGemo(0));
    m_script->addConnect(node_for->paramOutGemo(0), get_point->paramInGemo(1));

    m_script->addConnect(get_point->paramOutGemo(0), point_row->paramInGemo(0));
    m_script->addConnect(get_point->paramOutGemo(0), point_col->paramInGemo(0));

    JZNodeExpression *pt_expr_row = new JZNodeExpression();
    JZNodeExpression *pt_expr_col = new JZNodeExpression();

    JZNodeFunction *pt_color_get = new JZNodeFunction();
    pt_color_get->setFunction(func_inst->function("List.get"));

    JZNodeFunction *pt_rect_create = new JZNodeFunction();
    pt_rect_create->setFunction(rect_meta->function("create"));

    ret1 = pt_expr_row->setExpr(exrp1, error);
    ret2 = pt_expr_col->setExpr(exrp2, error);
    Q_ASSERT(ret1 && ret2);

    JZNodeFunction *pt_fill_rect = new JZNodeFunction();
    pt_fill_rect->setFunction(painter->function("fillRect"));

    m_script->addNode(pt_rect_create);
    m_script->addNode(pt_fill_rect);
    m_script->addNode(pt_expr_row);
    m_script->addNode(pt_expr_col);
    m_script->addNode(pt_color_get);

    m_script->addConnect(shape_row->paramOutGemo(0), shape_row_add->paramInGemo(0));
    m_script->addConnect(point_row->paramOutGemo(0), shape_row_add->paramInGemo(1));

    m_script->addConnect(shape_col->paramOutGemo(0), shape_col_add->paramInGemo(0));
    m_script->addConnect(point_col->paramOutGemo(0), shape_col_add->paramInGemo(1));

    m_script->addConnect(shape_row_add->paramOutGemo(0), pt_expr_row->paramInGemo(0));
    m_script->addConnect(shape_col_add->paramOutGemo(0), pt_expr_col->paramInGemo(0));

    m_script->addConnect(pt_expr_col->paramOutGemo(0), pt_rect_create->paramInGemo(0));
    m_script->addConnect(pt_expr_row->paramOutGemo(0), pt_rect_create->paramInGemo(1));
    pt_rect_create->setPinValue(pt_rect_create->paramIn(2), QString::number(m_blockSize));
    pt_rect_create->setPinValue(pt_rect_create->paramIn(3), QString::number(m_blockSize));

    m_script->addConnect(node_for->subFlowOutGemo(0), pt_fill_rect->flowInGemo());
    m_script->addConnect(set->paramOutGemo(0), pt_fill_rect->paramInGemo(0));
    m_script->addConnect(pt_rect_create->paramOutGemo(0), pt_fill_rect->paramInGemo(1));
    
    m_script->addConnect(node_color_list->paramOutGemo(0), pt_color_get->paramInGemo(0));
    m_script->addConnect(shape_color->paramOutGemo(0), pt_color_get->paramInGemo(1));
    m_script->addConnect(pt_color_get->paramOutGemo(0), pt_fill_rect->paramInGemo(2));
}

void SampleRussian::addButtonClicked()
{
    auto btn_meta = JZNodeObjectManager::instance()->meta("PushButton");
    auto meta = JZNodeObjectManager::instance()->meta("MainWindow");

    //btnStart
    JZNodeSingleEvent *btnStart = new JZNodeSingleEvent();
    btnStart->setSingle(btn_meta->single("clicked"));
    btnStart->setVariable("this.btnStart");

    m_script->addNode(btnStart);    

    JZNodeFunction *init_game = new JZNodeFunction();
    init_game->setFunction(meta->function("initGame"));
    m_script->addNode(init_game);

    JZNodeParam *timer = new JZNodeParam();
    timer->setVariable("this.timer");

    JZNodeFunction *start = new JZNodeFunction();
    start->setFunction(JZNodeFunctionManager::instance()->function("Timer.start"));
    start->setPinValue(start->paramIn(1), "500");

    m_script->addNode(timer);
    m_script->addNode(start);

    m_script->addConnect(timer->paramOutGemo(0), start->paramInGemo(0));
    m_script->addConnect(btnStart->flowOutGemo(), init_game->flowInGemo());
    m_script->addConnect(init_game->flowOutGemo(), start->flowInGemo());

    //*btnStop
    JZNodeSingleEvent *btnStop = new JZNodeSingleEvent();
    btnStop->setSingle(btn_meta->single("clicked"));
    btnStop->setVariable("this.btnStop");

    JZNodeFunction *stop = new JZNodeFunction();
    stop->setFunction(JZNodeFunctionManager::instance()->function("Timer.stop"));

    m_script->addNode(btnStop);
    m_script->addNode(stop);

    m_script->addConnect(timer->paramOutGemo(0), stop->paramInGemo(0));
    m_script->addConnect(btnStop->flowOutGemo(), stop->flowInGemo());
}

void SampleRussian::addCreateRect()
{
    auto mainwindow_id = JZNodeObjectManager::instance()->getClassId("MainWindow");    

    JZFunctionDefine define;
    define.name = "createRect";
    define.isFlowFunction = true;
    define.paramIn.push_back(JZParamDefine("this", mainwindow_id));
    define.paramOut.push_back(JZParamDefine("ok", Type_bool));

    auto class_file = m_project.getClass("MainWindow");
    auto script = class_file->addMemberFunction(define);
    auto meta = JZNodeObjectManager::instance()->meta("MainWindow");

    JZNode *node_start = script->getNode(0);

    JZNodeSetParam *node_isRect = new JZNodeSetParam();
    node_isRect->setVariable("this.isRect");
    node_isRect->setPinValue(node_isRect->paramIn(1), "true");

    JZNodeReturn *node_ret = new JZNodeReturn();
    node_ret->setFunction(JZNodeFunctionManager::instance()->function("MainWindow.createRect"));

    JZNodeReturn *node_ret_over = new JZNodeReturn();
    node_ret_over->setFunction(JZNodeFunctionManager::instance()->function("MainWindow.createRect"));

    script->addNode(node_ret);
    script->addNode(node_ret_over);
    script->addNode(node_isRect);

    auto func_inst = JZNodeFunctionManager::instance();
   
    JZNodeFunction *create_shape = new JZNodeFunction();
    create_shape->setFunction(meta->function("createShape"));    

    JZNodeSetParam *set_shape = new JZNodeSetParam();
    set_shape->setVariable("this.shape");

    JZNodeSetParam *shape_index = new JZNodeSetParam();
    shape_index->setVariable("this.shape_index");

    JZNodeSetParam *shape_row = new JZNodeSetParam();
    shape_row->setVariable("this.shape_row");

    JZNodeSetParam *shape_col = new JZNodeSetParam();
    shape_col->setVariable("this.shape_col");

    JZNodeSetParam *shape_color = new JZNodeSetParam();
    shape_color->setVariable("this.shape_color");

    JZNodeFor *shape_for = new JZNodeFor();    

    script->addNode(create_shape);
    script->addNode(set_shape);
    script->addNode(shape_index);
    script->addNode(shape_row);
    script->addNode(shape_col);
    script->addNode(shape_color);
    
    shape_row->setParamInValue(1, "0");    
    shape_index->setParamInValue(1, "0");
    
    JZNodeFunction *node_rand = new JZNodeFunction();
    node_rand->setFunction(func_inst->function("rand"));

    JZNodeMod *color_mod = new JZNodeMod();
    JZNodeMod *shape_col_mod = new JZNodeMod();
    JZNodeAdd *color_add = new JZNodeAdd();
    script->addNode(color_add);
    script->addNode(color_mod);
    script->addNode(shape_col_mod);
    script->addNode(node_rand);
    
    script->addConnect(node_rand->paramOutGemo(0), color_mod->paramInGemo(0));
    color_mod->setParamInValue(1,"4");

    script->addConnect(color_mod->paramOutGemo(0), color_add->paramInGemo(0));
    color_add->setParamInValue(1,"1");

    script->addConnect(node_rand->paramOutGemo(0), shape_col_mod->paramInGemo(0));
    shape_col_mod->setParamInValue(1, "8");    

    JZNodeMod *type_mod = new JZNodeMod();
    script->addNode(type_mod);
    script->addConnect(node_rand->paramOutGemo(0), type_mod->paramInGemo(0));
    type_mod->setParamInValue(1, "5");

    //isRect
    script->addConnect(node_start->flowOutGemo(), set_shape->flowInGemo());

    script->addConnect(type_mod->paramOutGemo(0), create_shape->paramInGemo(1));
    script->addConnect(create_shape->paramOutGemo(0), set_shape->paramInGemo(1));    
    
    script->addConnect(set_shape->flowOutGemo(), shape_index->flowInGemo());    

    script->addConnect(shape_index->flowOutGemo(), shape_row->flowInGemo());
    
    script->addConnect(shape_col_mod->paramOutGemo(0), shape_col->paramInGemo(1));
    script->addConnect(shape_row->flowOutGemo(), shape_col->flowInGemo());
    
    script->addConnect(color_add->paramOutGemo(0), shape_color->paramInGemo(1));
    script->addConnect(shape_col->flowOutGemo(), shape_color->flowInGemo());       

    JZNodeBranch *branch_place = new JZNodeBranch();

    JZNodeFunction *place_shape = new JZNodeFunction();
    place_shape->setFunction(meta->function("canPlace"));
    script->addNode(branch_place);
    script->addNode(place_shape);

    script->addConnect(shape_index->paramOutGemo(0), place_shape->paramInGemo(1));
    script->addConnect(shape_row->paramOutGemo(0), place_shape->paramInGemo(2));
    script->addConnect(shape_col->paramOutGemo(0), place_shape->paramInGemo(3));
    script->addConnect(place_shape->paramOutGemo(0), branch_place->paramInGemo(0));

    script->addConnect(shape_color->flowOutGemo(0), branch_place->flowInGemo());

    //return
    script->addConnect(branch_place->flowOutGemo(0), node_isRect->flowInGemo());
    script->addConnect(node_isRect->flowOutGemo(), node_ret->flowInGemo());

    script->addConnect(branch_place->flowOutGemo(1), node_ret_over->flowInGemo());
    node_ret->setPinValue(node_ret->paramIn(0), "false");    
    node_ret_over->setPinValue(node_ret->paramIn(0), "true");
}

void SampleRussian::addRectDown()
{
    auto mainwindow_id = JZNodeObjectManager::instance()->getClassId("MainWindow");
    auto func_inst = JZNodeFunctionManager::instance();

    JZFunctionDefine define;
    define.name = "rectDown";
    define.isFlowFunction = true;
    define.paramIn.push_back(JZParamDefine("this", mainwindow_id));
    define.paramOut.push_back(JZParamDefine("ok", Type_bool));

    auto class_file = m_project.getClass("MainWindow");    
    auto script = class_file->addMemberFunction(define);
    script->addLocalVariable(JZParamDefine("value", "int"));

    auto meta = JZNodeObjectManager::instance()->meta("MainWindow");

    JZNodeThis *node_this = new JZNodeThis();
    JZNode *node_start = script->getNode(0);

    JZNodeReturn *node_ret = new JZNodeReturn();
    node_ret->setFunction(meta->function("rectDown"));

    JZNodeFunction *canDown = new JZNodeFunction();
    canDown->setFunction(meta->function("canMoveDown"));

    JZNodeFunction *moveDown= new JZNodeFunction();
    moveDown->setFunction(meta->function("moveDown"));

    JZNodeBranch *branch = new JZNodeBranch();    
    script->addNode(node_this);
    script->addNode(canDown);
    script->addNode(moveDown);
    script->addNode(branch);
    script->addNode(node_ret);

    script->addConnect(node_this->paramOutGemo(0), canDown->paramInGemo(0));
    script->addConnect(node_this->paramOutGemo(0), moveDown->paramInGemo(0));

    // if canDown
    script->addConnect(canDown->paramOutGemo(0), branch->paramInGemo(0));
    script->addConnect(node_start->flowOutGemo(0), branch->flowInGemo());
    script->addConnect(branch->flowOutGemo(0), moveDown->flowInGemo());
    
    // else        
    JZNodeSetParam *set_if_rect = new JZNodeSetParam();
    set_if_rect->setVariable("this.isRect");
    set_if_rect->setParamInValue(1, "false");
    script->addNode(set_if_rect);        

    //set map
    JZNodeParam *shape = new JZNodeParam();
    shape->setVariable("this.shape");

    JZNodeParam *index = new JZNodeParam();
    index->setVariable("this.shape_index");

    JZNodeParam *shape_color = new JZNodeParam();
    shape_color->setVariable("this.shape_color");

    JZNodeFor *node_for = new JZNodeFor();

    JZNodeFunction *get_list = new JZNodeFunction();
    get_list->setFunction(func_inst->function("List.get"));

    JZNodeFunction *get_point = new JZNodeFunction();
    get_point->setFunction(func_inst->function("List.get"));

    JZNodeFunction *get_list_size = new JZNodeFunction();
    get_list_size->setFunction(func_inst->function("List.size"));

    JZNodeFunction *point_row = new JZNodeFunction();
    point_row->setFunction(func_inst->function("Point.x"));

    JZNodeFunction *point_col = new JZNodeFunction();
    point_col->setFunction(func_inst->function("Point.y"));

    JZNodeParam *shape_row = new JZNodeParam();
    shape_row->setVariable("this.shape_row");

    JZNodeParam *shape_col = new JZNodeParam();
    shape_col->setVariable("this.shape_col");

    JZNodeAdd *shape_row_add = new JZNodeAdd();
    JZNodeAdd *shape_col_add = new JZNodeAdd();

    JZNodeFunction *set_map = new JZNodeFunction();
    set_map->setFunction(meta->function("setMap"));
    
    script->addNode(node_for);
    script->addNode(shape_color);
    script->addNode(get_list);
    script->addNode(get_point);
    script->addNode(get_list_size);
    script->addNode(point_row);
    script->addNode(point_col);
    script->addNode(shape);
    script->addNode(index);

    script->addNode(shape_row);
    script->addNode(shape_col);
    script->addNode(shape_row_add);
    script->addNode(shape_col_add);
    script->addNode(set_map);

    script->addConnect(branch->flowOutGemo(1), node_for->flowInGemo());

    node_for->setParamInValue(0, "0");
    node_for->setParamInValue(1, "1");
    script->addConnect(get_list->paramOutGemo(0), get_list_size->paramInGemo(0));
    script->addConnect(get_list_size->paramOutGemo(0), node_for->paramInGemo(2));

    script->addConnect(shape->paramOutGemo(0), get_list->paramInGemo(0));
    script->addConnect(index->paramOutGemo(0), get_list->paramInGemo(1));

    script->addConnect(get_list->paramOutGemo(0), get_point->paramInGemo(0));
    script->addConnect(node_for->paramOutGemo(0), get_point->paramInGemo(1));

    script->addConnect(get_point->paramOutGemo(0), point_row->paramInGemo(0));
    script->addConnect(get_point->paramOutGemo(0), point_col->paramInGemo(0));

    script->addConnect(shape_row->paramOutGemo(0), shape_row_add->paramInGemo(0));
    script->addConnect(point_row->paramOutGemo(0), shape_row_add->paramInGemo(1));

    script->addConnect(shape_col->paramOutGemo(0), shape_col_add->paramInGemo(0));
    script->addConnect(point_col->paramOutGemo(0), shape_col_add->paramInGemo(1));

    script->addConnect(shape_row_add->paramOutGemo(0), set_map->paramInGemo(1));
    script->addConnect(shape_col_add->paramOutGemo(0), set_map->paramInGemo(2));
    set_map->setParamInValue(3, "0");

    script->addConnect(node_for->subFlowOutGemo(0), set_map->flowInGemo());
    script->addConnect(node_for->flowOutGemo(0), set_if_rect->flowInGemo());

    JZNodeFunction *node_update = new JZNodeFunction();
    node_update->setFunction(meta->function("update"));
    script->addNode(node_update);

    JZNodeFunction *clear_line = new JZNodeFunction();
    clear_line->setFunction(meta->function("clearLine"));
    script->addNode(clear_line);    

    script->addConnect(set_if_rect->flowOutGemo(0), clear_line->flowInGemo());    
    script->addConnect(clear_line->flowOutGemo(0), node_update->flowInGemo());

    // return
    node_ret->setPinValue(node_ret->paramIn(0), "false");
    script->addConnect(moveDown->flowOutGemo(), node_ret->flowInGemo());
    script->addConnect(node_update->flowOutGemo(), node_ret->flowInGemo());
}

void SampleRussian::addMoveFunction()
{
    auto mainwindow_id = JZNodeObjectManager::instance()->getClassId("MainWindow");
    auto class_file = m_project.getClass("MainWindow");
    QStringList functions = { "canMoveDown","canMoveLeft", "canMoveRight" , "moveDown", "moveLeft", "moveRight" };

    for (int i = 0; i < functions.size(); i++)
    {
        bool isCan = functions[i].startsWith("can");

        JZFunctionDefine define;
        define.name = functions[i];
        define.isFlowFunction = !isCan;
        define.paramIn.push_back(JZParamDefine("this", mainwindow_id));
        if(isCan)
            define.paramOut.push_back(JZParamDefine("ok", Type_bool));
        
        class_file->addMemberFunction(define);
    }   
    auto meta = JZNodeObjectManager::instance()->meta("MainWindow");
    auto func_inst = JZNodeFunctionManager::instance();

    for (int i = 0; i < functions.size(); i++)
    {
        QString function_name = functions[i];
        bool isCan = functions[i].startsWith("can");
        auto script = class_file->memberFunction(functions[i]);
        JZNode *node_start = script->getNode(0);        
                      
        if (isCan)
        {                       
            JZNodeParam *shape_row = new JZNodeParam();
            JZNodeParam *shape_col = new JZNodeParam();
            JZNodeParam *shape_index = new JZNodeParam();
            shape_row->setVariable("this.shape_row");
            shape_col->setVariable("this.shape_col");
            shape_index->setVariable("this.shape_index");

            JZNodeReturn *node_return = new JZNodeReturn();
            node_return->setFunction(meta->function(functions[i]));

            JZNodeFunction *can_place = new JZNodeFunction();
            can_place->setFunction(meta->function("canPlace"));

            script->addNode(shape_index);
            script->addNode(shape_row);
            script->addNode(shape_col);
            script->addNode(node_return);
            script->addNode(can_place);
            
            script->addConnect(shape_index->paramOutGemo(0), can_place->paramInGemo(1));
            if (functions[i] == "canMoveDown")
            {                
                JZNodeAdd *node_add = new JZNodeAdd();
                script->addNode(node_add);

                script->addConnect(shape_row->paramOutGemo(0), node_add->paramInGemo(0));
                node_add->setParamInValue(1, "1");
                script->addConnect(node_add->paramOutGemo(0), can_place->paramInGemo(2));
                script->addConnect(shape_col->paramOutGemo(0), can_place->paramInGemo(3));
            }
            else if (functions[i] == "canMoveRight")
            {
                JZNodeAdd *node_add = new JZNodeAdd();
                script->addNode(node_add);

                script->addConnect(shape_col->paramOutGemo(0), node_add->paramInGemo(0));
                node_add->setParamInValue(1, "1");
                script->addConnect(shape_row->paramOutGemo(0), can_place->paramInGemo(2));
                script->addConnect(node_add->paramOutGemo(0), can_place->paramInGemo(3));
            }
            else
            {
                JZNodeSub *node_sub = new JZNodeSub();
                script->addNode(node_sub);

                script->addConnect(shape_col->paramOutGemo(0), node_sub->paramInGemo(0));
                node_sub->setParamInValue(1, "1");
                script->addConnect(shape_row->paramOutGemo(0), can_place->paramInGemo(2));
                script->addConnect(node_sub->paramOutGemo(0), can_place->paramInGemo(3));
            }

            script->addConnect(node_start->flowOutGemo(), node_return->flowInGemo());                                                                                  
            script->addConnect(can_place->paramOutGemo(0), node_return->paramInGemo(0));
        }
        else
        {
            JZNodeFunction *node_update = new JZNodeFunction();
            node_update->setFunction(meta->function("update"));

            JZNodeParam *shape = new JZNodeParam();
            JZNodeSetParam *set_shape = new JZNodeSetParam();
            JZNode *node_op = nullptr;

            if (functions[i] == "moveDown")
            {
                shape->setVariable("this.shape_row");
                set_shape->setVariable("this.shape_row");
                node_op = new JZNodeAdd();
            }
            else if (functions[i] == "moveRight")
            {
                shape->setVariable("this.shape_col");
                set_shape->setVariable("this.shape_col");
                node_op = new JZNodeAdd();
            }
            else
            {
                shape->setVariable("this.shape_col");
                set_shape->setVariable("this.shape_col");
                node_op = new JZNodeSub();                
            }

            script->addNode(node_update);
            script->addNode(set_shape);
            script->addNode(shape);
            script->addNode(node_op);            

            script->addConnect(node_start->flowOutGemo(), set_shape->flowInGemo());
            script->addConnect(shape->paramOutGemo(0), node_op->paramInGemo(0));
            node_op->setParamInValue(1, "1");
            script->addConnect(node_op->paramOutGemo(0), set_shape->paramInGemo(1));
            script->addConnect(set_shape->flowOutGemo(), node_update->flowInGemo());
        }
    }    
}

void SampleRussian::addGameLoop()
{
    auto timer_meta = JZNodeObjectManager::instance()->meta("Timer");
    auto meta = JZNodeObjectManager::instance()->meta("MainWindow");

    JZNodeSingleEvent *onTimer = new JZNodeSingleEvent();
    onTimer->setSingle(timer_meta->single("timeout"));
    onTimer->setVariable("this.timer");

    m_script->addNode(onTimer);

    JZNodeThis *node_this = new JZNodeThis();

    JZNodeParam *is_rect = new JZNodeParam();
    is_rect->setVariable("this.isRect");

    JZNodeBranch *is_rect_branch = new JZNodeBranch();
    m_script->addNode(is_rect);
    m_script->addNode(is_rect_branch);

    m_script->addConnect(is_rect->paramOutGemo(0), is_rect_branch->paramInGemo(0));
    m_script->addConnect(onTimer->flowOutGemo(), is_rect_branch->flowInGemo());
    
    JZNodeFunction *rect_down = new JZNodeFunction();    
    rect_down->setFunction(meta->function("rectDown"));

    m_script->addNode(node_this);    
    m_script->addNode(rect_down);

    m_script->addConnect(node_this->paramOutGemo(0), rect_down->paramInGemo(0));
    m_script->addConnect(is_rect_branch->flowOutGemo(0), rect_down->flowInGemo());    

    JZNodeFunction *create_rect = new JZNodeFunction();
    create_rect->setFunction(meta->function("createRect"));
    m_script->addNode(create_rect);

    m_script->addConnect(node_this->paramOutGemo(0), create_rect->paramInGemo(0));
    m_script->addConnect(is_rect_branch->flowOutGemo(1), create_rect->flowInGemo());    

    JZNodeFunction *update = new JZNodeFunction();
    update->setFunction(JZNodeFunctionManager::instance()->function("Widget.update"));
    m_script->addNode(update);

    JZNodeParam *timer = new JZNodeParam();
    timer->setVariable("this.timer");
    m_script->addNode(timer);

    JZNodeFunction *stop_timer = new JZNodeFunction();
    stop_timer->setFunction(JZNodeFunctionManager::instance()->function("Timer.stop"));
    m_script->addNode(stop_timer);    

    m_script->addConnect(rect_down->flowOutGemo(0), update->flowInGemo());
    m_script->addConnect(create_rect->flowOutGemo(0), update->flowInGemo());

    //is game end
    JZNodeBranch *is_game_end = new JZNodeBranch();
    m_script->addNode(is_game_end);
    m_script->addConnect(rect_down->paramOutGemo(0), is_game_end->paramInGemo(0));
    m_script->addConnect(create_rect->paramOutGemo(0), is_game_end->paramInGemo(0));

    m_script->addConnect(node_this->paramOutGemo(0), update->paramInGemo(0));
    m_script->addConnect(update->flowOutGemo(0), is_game_end->flowInGemo());

    JZNodeFunction *message = new JZNodeFunction();
    message->setFunction(JZNodeFunctionManager::instance()->function("MessageBox.information"));
    message->setPinValue(message->paramIn(1), "\"Game over\"");
    m_script->addNode(message);

    m_script->addConnect(timer->paramOutGemo(0), stop_timer->paramInGemo(0));
    m_script->addConnect(is_game_end->flowOutGemo(), stop_timer->flowInGemo());
    m_script->addConnect(node_this->paramOutGemo(0), message->paramInGemo(0));
    m_script->addConnect(stop_timer->flowOutGemo(), message->flowInGemo());
    
}

void SampleRussian::addKeyEvent()
{
    auto func_inst = JZNodeFunctionManager::instance();

    auto meta = JZNodeObjectManager::instance()->meta("MainWindow");    
    auto key_meta = JZNodeObjectManager::instance()->meta("KeyEvent");    

    JZNodeQtEvent *node_keyPress = new JZNodeQtEvent();
    node_keyPress->setEvent("MainWindow", meta->event("keyPressEvent"));
    m_script->addNode(node_keyPress);

    
    JZNodeFunction *canRight = new JZNodeFunction();
    canRight->setFunction(meta->function("canMoveRight"));

    JZNodeFunction *moveRight = new JZNodeFunction();
    moveRight->setFunction(meta->function("moveRight"));

    JZNodeFunction *canLeft = new JZNodeFunction();
    canLeft->setFunction(meta->function("canMoveLeft"));

    JZNodeFunction *moveLeft = new JZNodeFunction();
    moveLeft->setFunction(meta->function("moveLeft"));

    JZNodeFunction *canDown = new JZNodeFunction();
    canDown->setFunction(meta->function("canMoveDown"));

    JZNodeFunction *moveDown = new JZNodeFunction();
    moveDown->setFunction(meta->function("moveDown"));
    
    JZNodeFunction *moveDown2 = new JZNodeFunction();
    moveDown2->setFunction(meta->function("moveDown"));

    JZNodeFunction *canRotate = new JZNodeFunction();
    canRotate->setFunction(meta->function("canRotate"));

    JZNodeFunction *rotate = new JZNodeFunction();
    rotate->setFunction(meta->function("rotate"));

    JZNodeIf *key_if = new JZNodeIf();
    key_if->addCondPin();
    key_if->addCondPin();
    key_if->addCondPin();
    key_if->addCondPin();

    JZNodeBranch *branch_right = new JZNodeBranch();
    JZNodeBranch *branch_left = new JZNodeBranch();
    JZNodeBranch *branch_down = new JZNodeBranch();
    JZNodeBranch *branch_rotate = new JZNodeBranch();
    JZNodeWhile *while_space = new JZNodeWhile();

    JZNodeFunction *key_code = new JZNodeFunction();
    key_code->setFunction(key_meta->function("key"));

    JZNodeEQ *key_eqR = new JZNodeEQ();
    JZNodeEQ *key_eqD = new JZNodeEQ();
    JZNodeEQ *key_eqL = new JZNodeEQ();    
    JZNodeEQ *key_eqRotate = new JZNodeEQ();

    JZNodeEQ *key_eqR2 = new JZNodeEQ();
    JZNodeEQ *key_eqD2 = new JZNodeEQ();
    JZNodeEQ *key_eqL2 = new JZNodeEQ();
    JZNodeEQ *key_eqRotate2 = new JZNodeEQ();

    JZNodeOr *key_eqROr = new JZNodeOr();
    JZNodeOr *key_eqDOr = new JZNodeOr();
    JZNodeOr *key_eqLOr = new JZNodeOr();
    JZNodeOr *key_eqRotateOr = new JZNodeOr();

    JZNodeEQ *key_eqSpace = new JZNodeEQ(); 
        
    m_script->addNode(canRight);
    m_script->addNode(moveRight);
    m_script->addNode(canLeft);
    m_script->addNode(moveLeft);
    m_script->addNode(canDown);
    m_script->addNode(moveDown);
    m_script->addNode(canRotate);
    m_script->addNode(rotate);
    m_script->addNode(branch_right);
    m_script->addNode(branch_left);
    m_script->addNode(branch_down);
    m_script->addNode(key_code);
    m_script->addNode(key_eqR);
    m_script->addNode(key_eqD);
    m_script->addNode(key_eqL);
    m_script->addNode(key_eqRotate);
    m_script->addNode(branch_rotate);
    m_script->addNode(key_if);        
    m_script->addNode(key_eqSpace);
    m_script->addNode(while_space);
    m_script->addNode(moveDown2);

    m_script->addNode(key_eqR2);
    m_script->addNode(key_eqD2);
    m_script->addNode(key_eqL2);
    m_script->addNode(key_eqRotate2);

    m_script->addNode(key_eqROr);
    m_script->addNode(key_eqDOr);
    m_script->addNode(key_eqLOr);
    m_script->addNode(key_eqRotateOr);

    m_script->addConnect(node_keyPress->paramOutGemo(0), key_code->paramInGemo(0));        
    
    auto connectKey = [this](JZNodeEQ *eq, int key_code){
        JZNodeEnum *key = new JZNodeEnum();
        key->setEnum("Key");
        key->setValue(key_code);
        m_script->addNode(key);
        m_script->addConnect(key->paramOutGemo(0), eq->paramInGemo(1));
    };

    connectKey(key_eqRotate,Qt::Key_W);
    connectKey(key_eqR, Qt::Key_D);
    connectKey(key_eqD, Qt::Key_S);
    connectKey(key_eqL, Qt::Key_A);
    connectKey(key_eqSpace, Qt::Key_Space);

    connectKey(key_eqRotate2, Qt::Key_Up);
    connectKey(key_eqD2, Qt::Key_Down);
    connectKey(key_eqL2, Qt::Key_Left);
    connectKey(key_eqR2, Qt::Key_Right);    

    m_script->addConnect(node_keyPress->flowOutGemo(0), key_if->flowInGemo());
    m_script->addConnect(key_code->paramOutGemo(0), key_eqL->paramInGemo(0));
    m_script->addConnect(key_code->paramOutGemo(0), key_eqD->paramInGemo(0));
    m_script->addConnect(key_code->paramOutGemo(0), key_eqR->paramInGemo(0));    
    m_script->addConnect(key_code->paramOutGemo(0), key_eqSpace->paramInGemo(0));
    m_script->addConnect(key_code->paramOutGemo(0), key_eqRotate->paramInGemo(0));

    m_script->addConnect(key_code->paramOutGemo(0), key_eqL2->paramInGemo(0));
    m_script->addConnect(key_code->paramOutGemo(0), key_eqD2->paramInGemo(0));
    m_script->addConnect(key_code->paramOutGemo(0), key_eqR2->paramInGemo(0));
    m_script->addConnect(key_code->paramOutGemo(0), key_eqRotate2->paramInGemo(0));

    m_script->addConnect(key_eqL->paramOutGemo(0), key_eqLOr->paramInGemo(0));
    m_script->addConnect(key_eqD->paramOutGemo(0), key_eqDOr->paramInGemo(0));
    m_script->addConnect(key_eqR->paramOutGemo(0), key_eqROr->paramInGemo(0));
    m_script->addConnect(key_eqRotate->paramOutGemo(0), key_eqRotateOr->paramInGemo(0));

    m_script->addConnect(key_eqL2->paramOutGemo(0), key_eqLOr->paramInGemo(1));
    m_script->addConnect(key_eqD2->paramOutGemo(0), key_eqDOr->paramInGemo(1));
    m_script->addConnect(key_eqR2->paramOutGemo(0), key_eqROr->paramInGemo(1));
    m_script->addConnect(key_eqRotate2->paramOutGemo(0), key_eqRotateOr->paramInGemo(1));

    m_script->addConnect(key_eqLOr->paramOutGemo(0), key_if->paramInGemo(0));
    m_script->addConnect(key_eqDOr->paramOutGemo(0), key_if->paramInGemo(1));
    m_script->addConnect(key_eqROr->paramOutGemo(0), key_if->paramInGemo(2));        
    m_script->addConnect(key_eqRotateOr->paramOutGemo(0), key_if->paramInGemo(3));
    m_script->addConnect(key_eqSpace->paramOutGemo(0), key_if->paramInGemo(4));

    m_script->addConnect(key_if->subFlowOutGemo(0), branch_left->flowInGemo());
    m_script->addConnect(key_if->subFlowOutGemo(1), branch_down->flowInGemo());
    m_script->addConnect(key_if->subFlowOutGemo(2), branch_right->flowInGemo());
    m_script->addConnect(key_if->subFlowOutGemo(3), branch_rotate->flowInGemo());
    m_script->addConnect(key_if->subFlowOutGemo(4), while_space->flowInGemo());
    
    m_script->addConnect(canLeft->paramOutGemo(0), branch_left->paramInGemo(0));
    m_script->addConnect(canDown->paramOutGemo(0), branch_down->paramInGemo(0));
    m_script->addConnect(canRight->paramOutGemo(0), branch_right->paramInGemo(0));
    m_script->addConnect(canRotate->paramOutGemo(0), branch_rotate->paramInGemo(0));
    m_script->addConnect(canDown->paramOutGemo(0), while_space->paramInGemo(0));

    m_script->addConnect(branch_down->flowOutGemo(0), moveDown->flowInGemo());    
    m_script->addConnect(branch_right->flowOutGemo(0), moveRight->flowInGemo());    
    m_script->addConnect(branch_left->flowOutGemo(0), moveLeft->flowInGemo());    
    m_script->addConnect(branch_rotate->flowOutGemo(0), rotate->flowInGemo());
    m_script->addConnect(while_space->subFlowOutGemo(0), moveDown2->flowInGemo());
}

void SampleRussian::addRotate()
{
    auto mainwindow_id = JZNodeObjectManager::instance()->getClassId("MainWindow");
    auto class_file = m_project.getClass("MainWindow");

    JZFunctionDefine def_can;    
    def_can.name = "canRotate";
    def_can.isFlowFunction = false;
    def_can.paramIn.push_back(JZParamDefine("this", mainwindow_id));
    def_can.paramOut.push_back(JZParamDefine("ok", Type_bool));
    class_file->addMemberFunction(def_can);
    
    JZFunctionDefine def_rotate;
    def_rotate.name = "rotate";
    def_rotate.isFlowFunction = true;
    def_rotate.paramIn.push_back(JZParamDefine("this", mainwindow_id));
    class_file->addMemberFunction(def_rotate);

    auto meta = JZNodeObjectManager::instance()->meta("MainWindow");
    auto func_inst = JZNodeFunctionManager::instance();

    //can
    {
        auto script = class_file->memberFunction("canRotate");
        JZNode *node_start = script->getNode(0);

        JZNodeParam *shape_row = new JZNodeParam();
        JZNodeParam *shape_col = new JZNodeParam();
        JZNodeParam *shape_index = new JZNodeParam();
        JZNodeParam *shape = new JZNodeParam();

        JZNodeFunction *shape_size = new JZNodeFunction();
        shape_size->setFunction(JZNodeFunctionManager::instance()->function("List.size"));

        shape->setVariable("this.shape");
        shape_row->setVariable("this.shape_row");
        shape_col->setVariable("this.shape_col");
        shape_index->setVariable("this.shape_index");

        JZNodeReturn *node_return = new JZNodeReturn();
        node_return->setFunction(&def_can);

        JZNodeFunction *can_place = new JZNodeFunction();
        can_place->setFunction(meta->function("canPlace"));

        script->addNode(shape);
        script->addNode(shape_size);
        script->addNode(shape_index);
        script->addNode(shape_row);
        script->addNode(shape_col);
        script->addNode(node_return);
        script->addNode(can_place);

        JZNodeAdd *index_add = new JZNodeAdd();
        JZNodeMod *index_mod = new JZNodeMod();
        script->addNode(index_add);
        script->addNode(index_mod);

        script->addConnect(shape_index->paramOutGemo(0), index_add->paramInGemo(0));
        index_add->setParamInValue(1, QString::number(1));

        script->addConnect(shape->paramOutGemo(0), shape_size->paramInGemo(0));

        script->addConnect(index_add->paramOutGemo(0), index_mod->paramInGemo(0));
        script->addConnect(shape_size->paramOutGemo(0), index_mod->paramInGemo(1));

        script->addConnect(index_mod->paramOutGemo(0), can_place->paramInGemo(1));
        script->addConnect(shape_row->paramOutGemo(0), can_place->paramInGemo(2));
        script->addConnect(shape_col->paramOutGemo(0), can_place->paramInGemo(3));


        script->addConnect(node_start->flowOutGemo(), node_return->flowInGemo());
        script->addConnect(can_place->paramOutGemo(0), node_return->paramInGemo(0));
    }

    //rotate       
    {
        auto script = class_file->memberFunction("rotate");
        JZNode *node_start = script->getNode(0);
        JZNodeFunction *node_update = new JZNodeFunction();
        node_update->setFunction(meta->function("update"));

        JZNodeParam *shape_index = new JZNodeParam();
        JZNodeSetParam *set_shape_index = new JZNodeSetParam();

        JZNodeParam *shape = new JZNodeParam();
        JZNodeFunction *shape_size = new JZNodeFunction();
        shape_size->setFunction(JZNodeFunctionManager::instance()->function("List.size"));

        shape->setVariable("this.shape");
        shape_index->setVariable("this.shape_index");
        set_shape_index->setVariable("this.shape_index");

        JZNodeAdd *index_add = new JZNodeAdd();
        JZNodeMod *index_mod = new JZNodeMod();
        script->addNode(index_add);
        script->addNode(index_mod);
        script->addNode(node_update);
        script->addNode(shape_index);        
        script->addNode(set_shape_index);
        script->addNode(shape);
        script->addNode(shape_size);

        script->addConnect(shape_index->paramOutGemo(0), index_add->paramInGemo(0));
        index_add->setParamInValue(1, QString::number(1));

        script->addConnect(shape->paramOutGemo(0), shape_size->paramInGemo(0));

        script->addConnect(index_add->paramOutGemo(0), index_mod->paramInGemo(0));
        script->addConnect(shape_size->paramOutGemo(0), index_mod->paramInGemo(1));
                
        script->addConnect(index_mod->paramOutGemo(0), set_shape_index->paramInGemo(1));
        script->addConnect(node_start->flowOutGemo(), set_shape_index->flowInGemo());
        script->addConnect(set_shape_index->flowOutGemo(), node_update->flowInGemo());
    }
}

QVector<QVector<QVector<QPoint>>> SampleRussian::shapeGroup()
{
    QVector<QVector<QVector<QPoint>>> shape_group;

    QVector<QPoint> pt_box;
    pt_box << QPoint(0, 0) << QPoint(0, 1) << QPoint(1, 0) << QPoint(1, 1);

    QVector<QVector<QPoint>> box_group;
    box_group << pt_box;
    shape_group << box_group;

    QVector<QPoint> pt_line1,pt_line2;
    pt_line1 << QPoint(0, 0) << QPoint(1, 0) << QPoint(2, 0) << QPoint(3, 0);
    pt_line2 << QPoint(1, -2) << QPoint(1, -1) << QPoint(1, 0) << QPoint(1, 1);

    QVector<QVector<QPoint>> line_group;
    line_group << pt_line1 << pt_line2;
    shape_group << line_group;

    QVector<QPoint> pt_tri1, pt_tri2, pt_tri3, pt_tri4;
    pt_tri1 << QPoint(0, 1) << QPoint(1, 0) << QPoint(1, 1) << QPoint(1, 2);
    pt_tri2 << QPoint(0, 1) << QPoint(1, 1) << QPoint(2, 1) << QPoint(1, 2);
    pt_tri3 << QPoint(1, 0) << QPoint(1, 1) << QPoint(1, 2) << QPoint(2, 1);
    pt_tri4 << QPoint(1, 0) << QPoint(0, 1) << QPoint(1, 1) << QPoint(2, 1);

    QVector<QVector<QPoint>> tri_group;
    tri_group << pt_tri1 << pt_tri2 << pt_tri3 << pt_tri4;
    shape_group << tri_group;

    QVector<QPoint> pt_left1, pt_left2, pt_left3, pt_left4;
    pt_left1 << QPoint(0, 0) << QPoint(0, 1) << QPoint(0, 2) << QPoint(1, 0);
    pt_left2 << QPoint(0, 1) << QPoint(0, 2) << QPoint(1, 2) << QPoint(2, 2);
    pt_left3 << QPoint(1, 0) << QPoint(1, 1) << QPoint(1, 2) << QPoint(0, 2);
    pt_left4 << QPoint(0,0) << QPoint(1, 0) << QPoint(2, 0) << QPoint(2, 1);

    QVector<QVector<QPoint>> left_group;
    left_group << pt_left1 << pt_left2 << pt_left3 << pt_left4;
    shape_group << left_group;

    QVector<QPoint> pt_right1, pt_right2, pt_right3, pt_right4;
    pt_right1 << QPoint(0, 0) << QPoint(0, 1) << QPoint(0, 2) << QPoint(1, 2);
    pt_right2 << QPoint(2, 1) << QPoint(0, 2) << QPoint(1, 2) << QPoint(2, 2);
    pt_right3 << QPoint(1, 0) << QPoint(2, 0) << QPoint(2, 1) << QPoint(2, 2);
    pt_right4 << QPoint(0, 0) << QPoint(1, 0) << QPoint(2, 0) << QPoint(0, 1);

    QVector<QVector<QPoint>> right_group;
    right_group << pt_right1 << pt_right2 << pt_right3 << pt_right4;
    shape_group << right_group;

    return shape_group;
}

void SampleRussian::addCreateShape()
{
    auto mainwindow_id = JZNodeObjectManager::instance()->getClassId("MainWindow");

    JZFunctionDefine define;
    define.name = "createShape";
    define.isFlowFunction = false;
    define.paramIn.push_back(JZParamDefine("this", mainwindow_id));
    define.paramIn.push_back(JZParamDefine("type", Type_int));    
    define.paramOut.push_back(JZParamDefine("shape", Type_list));

    auto func_inst = JZNodeFunctionManager::instance();
    auto class_file = m_project.getClass("MainWindow");
    class_file->addMemberFunction(define);
    
    auto script = class_file->memberFunction("createShape");
    script->addLocalVariable(JZParamDefine("shape", Type_list));

    JZNodeParam *shape = new JZNodeParam();
    shape->setVariable("shape");

    JZNodeParam *type = new JZNodeParam();
    type->setVariable("type");
    
    JZNodeReturn *node_return = new JZNodeReturn();
    node_return->setFunction(&define);    

    JZNode *node_start = script->getNode(0);
    script->addNode(type);
    script->addNode(shape);    
    script->addNode(node_return);           

    JZNodeIf *node_if = new JZNodeIf();
    node_if->addCondPin();  //2
    node_if->addCondPin();  //3
    node_if->addCondPin();  //4
    node_if->addCondPin();  //5
    script->addNode(node_if);
    script->addConnect(node_start->flowOutGemo(0), node_if->flowInGemo());
            
    QVector<QVector<QVector<QPoint>>> ptGroup = shapeGroup();
    for (int shape_type = 0; shape_type < ptGroup.size(); shape_type++)
    {
        JZNodeEQ *eq = new JZNodeEQ();
        script->addNode(eq);
        script->addConnect(type->paramOutGemo(0), eq->paramInGemo(0));
        eq->setParamInValue(1, QString::number(shape_type));

        script->addConnect(eq->paramOutGemo(0), node_if->paramInGemo(shape_type));

        JZNodeCreateFromString *create_from_string = new JZNodeCreateFromString();
        script->addNode(create_from_string);        
        create_from_string->setClassName("List");

        QString context;
        for (int type_idx = 0; type_idx < ptGroup[shape_type].size(); type_idx++)
        {
            context += "[";
            auto ptList = ptGroup[shape_type][type_idx];
            for (int i = 0; i < ptList.size(); i++)
            {
                context += QString::asprintf("Point{%d,%d}", ptList[i].x(), ptList[i].y());
                if (i != ptList.size() - 1)
                    context += ",";
            }
            context += "]";
            if (type_idx != ptGroup[shape_type].size() - 1)
                context += ",";
            context += "\n";
        }
        create_from_string->setContext("\"" + context + "\"");

        JZNodeSetParam *set_shape = new JZNodeSetParam();
        set_shape->setVariable("shape");
        script->addNode(set_shape);

        script->addConnect(node_if->subFlowOutGemo(shape_type), set_shape->flowInGemo());
        script->addConnect(create_from_string->paramOutGemo(0), set_shape->paramInGemo(1));
    }
        
    
    script->addConnect(shape->paramOutGemo(0), node_return->paramInGemo(0));
    script->addConnect(node_if->flowOutGemo(0), node_return->flowInGemo());
}

void SampleRussian::addCanPlaceShape()
{
    auto mainwindow_id = JZNodeObjectManager::instance()->getClassId("MainWindow");

    JZFunctionDefine define;
    define.name = "canPlace";
    define.isFlowFunction = false;
    define.paramIn.push_back(JZParamDefine("this", mainwindow_id));
    define.paramIn.push_back(JZParamDefine("index", Type_int));
    define.paramIn.push_back(JZParamDefine("row", Type_int));
    define.paramIn.push_back(JZParamDefine("col", Type_int));
    define.paramOut.push_back(JZParamDefine("result", Type_bool));

    auto class_file = m_project.getClass("MainWindow");
    class_file->addMemberFunction(define);

    auto func_inst = JZNodeFunctionManager::instance();
    auto meta = JZNodeObjectManager::instance()->meta("MainWindow");

    auto script = class_file->memberFunction("canPlace");
    script->addLocalVariable(JZParamDefine("pt_row", "int"));
    script->addLocalVariable(JZParamDefine("pt_col", "int"));

    JZNode *node_start = script->getNode(0);

    JZNodeReturn *ret_true = new JZNodeReturn();
    ret_true->setFunction(&define);
    ret_true->setParamInValue(0, "true");

    JZNodeReturn *ret_false = new JZNodeReturn();
    ret_false->setFunction(&define);
    ret_false->setParamInValue(0, "false");

    JZNodeFor *node_for = new JZNodeFor();

    JZNodeFunction *get_list = new JZNodeFunction();
    get_list->setFunction(func_inst->function("List.get"));

    JZNodeFunction *get_point = new JZNodeFunction();
    get_point->setFunction(func_inst->function("List.get"));
    
    JZNodeFunction *get_list_size = new JZNodeFunction();
    get_list_size->setFunction(func_inst->function("List.size"));

    JZNodeFunction *point_row = new JZNodeFunction();
    point_row->setFunction(func_inst->function("Point.x"));

    JZNodeFunction *point_col = new JZNodeFunction();
    point_col->setFunction(func_inst->function("Point.y"));

    JZNodeFunction *get_map = new JZNodeFunction();
    get_map->setFunction(meta->function("getMap"));

    JZNodeBranch *branch = new JZNodeBranch();

    JZNodeParam *in_row = new JZNodeParam();
    in_row->setVariable("row");

    JZNodeParam *in_col = new JZNodeParam();
    in_col->setVariable("col");

    JZNodeSetParam *set_row = new JZNodeSetParam();
    set_row->setVariable("pt_row");

    JZNodeSetParam *set_col = new JZNodeSetParam();
    set_col->setVariable("pt_col");

    JZNodeParam *shape = new JZNodeParam();
    shape->setVariable("this.shape");

    JZNodeParam *index = new JZNodeParam();
    index->setVariable("index");

    JZNodeParam *row = new JZNodeParam();
    row->setVariable("pt_row");

    JZNodeParam *col = new JZNodeParam();
    col->setVariable("pt_col");

    JZNodeOr *or = new JZNodeOr();
    or->addInput();

    JZNodeOr *or_row = new JZNodeOr();

    JZNodeOr *or_col = new JZNodeOr();

    JZNodeNE *eq_map = new JZNodeNE();    

    JZNodeLT *row_lt = new JZNodeLT();
    JZNodeGE *row_ge = new JZNodeGE();
    JZNodeLT *col_lt = new JZNodeLT();
    JZNodeGE *col_ge = new JZNodeGE();

    JZNodeAdd *add_row = new JZNodeAdd();
    JZNodeAdd *add_col = new JZNodeAdd();    
    script->addNode(ret_true);
    script->addNode(ret_false);
    script->addNode(node_for);
    script->addNode(get_list);
    script->addNode(get_point);
    script->addNode(get_list_size);
    script->addNode(point_row);
    script->addNode(point_col);
    script->addNode(get_map);
    script->addNode(branch);
    script->addNode(in_row);
    script->addNode(in_col);
    script->addNode(set_row);
    script->addNode(set_col);
    script->addNode(shape);
    script->addNode(index);
    script->addNode(row);
    script->addNode(col);
    script->addNode(or);
    script->addNode(or_row);
    script->addNode(or_col);
    script->addNode(eq_map);
    script->addNode(row_lt);
    script->addNode(row_ge);
    script->addNode(col_lt);
    script->addNode(col_ge);
    script->addNode(add_row);
    script->addNode(add_col);        

    script->addConnect(row->paramOutGemo(0), row_lt->paramInGemo(0));
    row_lt->setParamInValue(1, "0");

    script->addConnect(row->paramOutGemo(0), row_ge->paramInGemo(0));
    row_ge->setParamInValue(1, QString::number(m_row));

    script->addConnect(col->paramOutGemo(0), col_lt->paramInGemo(0));
    col_lt->setParamInValue(1, "0");

    script->addConnect(col->paramOutGemo(0), col_ge->paramInGemo(0));
    col_ge->setParamInValue(1, QString::number(m_col));

    script->addConnect(row_lt->paramOutGemo(0), or_row->paramInGemo(0));
    script->addConnect(row_ge->paramOutGemo(0), or_row->paramInGemo(1));

    script->addConnect(col_lt->paramOutGemo(0), or_col->paramInGemo(0));
    script->addConnect(col_ge->paramOutGemo(0), or_col->paramInGemo(1));

    script->addConnect(row->paramOutGemo(0), get_map->paramInGemo(1));
    script->addConnect(col->paramOutGemo(0), get_map->paramInGemo(2));    
    script->addConnect(get_map->paramOutGemo(0), eq_map->paramInGemo(0));
    eq_map->setParamInValue(1, QString::number(-1));

    // row or col or getMap
    script->addConnect(or_row->paramOutGemo(0), or->paramInGemo(0));
    script->addConnect(or_col->paramOutGemo(0), or->paramInGemo(1));
    script->addConnect(eq_map->paramOutGemo(0), or->paramInGemo(2));

    node_for->setParamInValue(0, QString::number(0));
    node_for->setParamInValue(1, QString::number(1));

    script->addConnect(get_list->paramOutGemo(0), get_list_size->paramInGemo(0));
    script->addConnect(get_list_size->paramOutGemo(0), node_for->paramInGemo(2));
    script->addConnect(node_start->flowOutGemo(), node_for->flowInGemo());

    // for
    script->addConnect(shape->paramOutGemo(0), get_list->paramInGemo(0));
    script->addConnect(index->paramOutGemo(0), get_list->paramInGemo(1));

    script->addConnect(get_list->paramOutGemo(0), get_point->paramInGemo(0));
    script->addConnect(node_for->paramOutGemo(0), get_point->paramInGemo(1));

    script->addConnect(get_point->paramOutGemo(0), point_row->paramInGemo(0));
    script->addConnect(get_point->paramOutGemo(0), point_col->paramInGemo(0));

    script->addConnect(in_row->paramOutGemo(0), add_row->paramInGemo(0));
    script->addConnect(point_row->paramOutGemo(0), add_row->paramInGemo(1));

    script->addConnect(in_col->paramOutGemo(0), add_col->paramInGemo(0));
    script->addConnect(point_col->paramOutGemo(0), add_col->paramInGemo(1));

    script->addConnect(add_row->paramOutGemo(0), set_row->paramInGemo(1));
    script->addConnect(add_col->paramOutGemo(0), set_col->paramInGemo(1));

    script->addConnect(node_for->subFlowOutGemo(0), set_row->flowInGemo());
    script->addConnect(set_row->flowOutGemo(0), set_col->flowInGemo());        
    script->addConnect(set_col->flowOutGemo(0), branch->flowInGemo());

    script->addConnect(or->paramOutGemo(0), branch->paramInGemo(0));
    script->addConnect(branch->flowOutGemo(0), ret_false->flowInGemo());

    //return true;
    script->addConnect(node_for->flowOutGemo(), ret_true->flowInGemo());
}

void SampleRussian::addClearLine()
{
    auto mainwindow_id = JZNodeObjectManager::instance()->getClassId("MainWindow");

    JZFunctionDefine define;
    define.name = "clearLine";
    define.isFlowFunction = true;
    define.paramIn.push_back(JZParamDefine("this", mainwindow_id));    

    auto class_file = m_project.getClass("MainWindow");
    class_file->addMemberFunction(define);

    auto func_inst = JZNodeFunctionManager::instance();
    auto meta = JZNodeObjectManager::instance()->meta("MainWindow");

    auto script = class_file->memberFunction("clearLine");
    script->addLocalVariable(JZParamDefine("line_count", "int"));
    script->addLocalVariable(JZParamDefine("row", "int", QString::number(m_row - 1)));

    JZNode *node_start = script->getNode(0);

    JZNodeWhile *while_row = new JZNodeWhile();
    JZNodeFor *for_col = new JZNodeFor();    
    for_col->setRange(0, m_col);

    JZNodeParam *node_row = new JZNodeParam();
    node_row->setVariable("row");

    JZNodeSetParam *node_set_row = new JZNodeSetParam();
    node_set_row->setVariable("row");

    JZNodeAdd *node_row_add = new JZNodeAdd();
    node_row_add->setParamInValue(1, QString::number(-1));

    JZNodeParam *node_line_count = new JZNodeParam();
    node_line_count->setVariable("line_count");

    JZNodeEQ *map_eq_0 = new JZNodeEQ();
    map_eq_0->setParamInValue(1, "0");

    JZNodeSetParam *node_param_set0 = new JZNodeSetParam();
    node_param_set0->setVariable("line_count");
    node_param_set0->setParamInValue(1,"0");

    JZNodeSetParam *node_param_set_add = new JZNodeSetParam();
    node_param_set_add->setVariable("line_count");

    JZNodeAdd *node_add = new JZNodeAdd();
    node_add->setParamInValue(1, QString::number(1));

    JZNodeBranch *line_count_branch = new JZNodeBranch();

    JZNodeBranch *line_count_eq_col_branch = new JZNodeBranch();

    JZNodeFunction *get_map = new JZNodeFunction();
    get_map->setFunction(meta->function("getMap"));

    JZNodeFunction *get_map2 = new JZNodeFunction();
    get_map2->setFunction(meta->function("getMap"));

    JZNodeFunction *set_map = new JZNodeFunction();
    set_map->setFunction(meta->function("setMap"));      

    script->addNode(node_row);
    script->addNode(node_set_row);
    script->addNode(node_row_add);    
    script->addNode(node_line_count);
    script->addNode(map_eq_0);

    script->addNode(node_param_set0);
    script->addNode(node_param_set_add);
    script->addNode(node_add);
    script->addNode(line_count_branch);
    script->addNode(line_count_eq_col_branch);
    script->addNode(get_map);
    script->addNode(get_map2);
    script->addNode(set_map);

    script->addNode(while_row);
    script->addNode(for_col);

    JZNodeGE *node_ge = new JZNodeGE();
    node_ge->setParamInValue(1, "0");
    script->addNode(node_ge);

    script->addConnect(node_row->paramOutGemo(0), node_row_add->paramInGemo(0));
    script->addConnect(node_row->paramOutGemo(0), node_ge->paramInGemo(0));
    script->addConnect(node_row_add->paramOutGemo(0), node_set_row->paramInGemo(1));
    script->addConnect(node_ge->paramOutGemo(0), while_row->paramInGemo(0));

    // while(row >= 0)
    script->addConnect(node_start->flowOutGemo(), while_row->flowInGemo());
    script->addConnect(while_row->subFlowOutGemo(0), node_param_set0->flowInGemo());
    script->addConnect(node_param_set0->flowOutGemo(), for_col->flowInGemo());

    // for col
    script->addConnect(node_row->paramOutGemo(0), get_map->paramInGemo(1));
    script->addConnect(for_col->paramOutGemo(0), get_map->paramInGemo(2));

    script->addConnect(get_map->paramOutGemo(0), map_eq_0->paramInGemo(0));
    script->addConnect(map_eq_0->paramOutGemo(0), line_count_branch->paramInGemo(0));
    script->addConnect(for_col->subFlowOutGemo(0), line_count_branch->flowInGemo());

    // if(get_map == 0)
    script->addConnect(node_line_count->paramOutGemo(0), node_add->paramInGemo(0));
    script->addConnect(node_add->paramOutGemo(0), node_param_set_add->paramInGemo(1));
    node_add->setParamInValue(1, QString::number(1));

    // line_count += 1
    script->addConnect(line_count_branch->flowOutGemo(), node_param_set_add->flowInGemo());

    // if( == 10)
    JZNodeEQ *line_count_eq = new JZNodeEQ();
    line_count_eq->setParamInValue(1, QString::number(m_col));
    script->addNode(line_count_eq);

    script->addConnect(node_line_count->paramOutGemo(0), line_count_eq->paramInGemo(0));
    script->addConnect(line_count_eq->paramOutGemo(0), line_count_eq_col_branch->paramInGemo(0));

    script->addConnect(for_col->flowOutGemo(0), line_count_eq_col_branch->flowInGemo());
    
    //����
    JZNodeFor *for_row_down = new JZNodeFor();
    JZNodeFor *for_col_down = new JZNodeFor();
    for_row_down->setStep(-1);
    for_row_down->setEnd(0);
    for_row_down->setOp(OP_gt);
    for_col_down->setRange(0, m_col);
    
    JZNodeAdd *row_add = new JZNodeAdd();
    row_add->setParamInValue(1, QString::number(-1));

    JZNodeFunction *set_map2 = new JZNodeFunction();
    set_map2->setFunction(meta->function("setMap"));

    JZNodeFor *for_col_top = new JZNodeFor();
    for_col_top->setRange(0, m_col);
    script->addNode(for_row_down);
    script->addNode(for_col_down);
    script->addNode(row_add);
    script->addNode(set_map2);
    script->addNode(for_col_top);

    JZNodeFor *for_col_clear = new JZNodeFor();
    for_col_clear->setRange(0, m_col);

    JZNodeFunction *set_map_clear = new JZNodeFunction();
    set_map_clear->setFunction(meta->function("setMap"));
    script->addNode(for_col_clear);
    script->addNode(set_map_clear);

    script->addConnect(node_row->paramOutGemo(0), set_map_clear->paramInGemo(1));
    script->addConnect(for_col_clear->paramOutGemo(0), set_map_clear->paramInGemo(2));
    set_map_clear->setParamInValue(3, QString::number(-1));

    script->addConnect(node_row->paramOutGemo(0), for_row_down->paramInGemo(0));    
    
    JZNodeAdd *score_add = new JZNodeAdd();
    score_add->setParamInValue(1, QString::number(10));

    JZNodeParam *score_get = new JZNodeParam();
    score_get->setVariable("this.score");

    JZNodeSetParam *score_set = new JZNodeSetParam();
    score_set->setVariable("this.score");

    script->addNode(score_get);
    script->addNode(score_add);
    script->addNode(score_set);

    script->addConnect(score_get->paramOutGemo(0), score_add->paramInGemo(0));
    script->addConnect(score_add->paramOutGemo(0), score_set->paramInGemo(1));

    script->addConnect(line_count_eq_col_branch->flowOutGemo(0), score_set->flowInGemo());
    script->addConnect(score_set->flowOutGemo(), for_col_clear->flowInGemo());

    script->addConnect(for_col_clear->subFlowOutGemo(0), set_map_clear->flowInGemo());

    script->addConnect(for_col_clear->flowOutGemo(0), for_row_down->flowInGemo());
    script->addConnect(for_row_down->subFlowOutGemo(0), for_col_down->flowInGemo());
    
    script->addConnect(for_row_down->paramOutGemo(0), row_add->paramInGemo(0));
    script->addConnect(row_add->paramOutGemo(0), get_map2->paramInGemo(1));
    script->addConnect(for_col_down->paramOutGemo(0), get_map2->paramInGemo(2));
    
    script->addConnect(for_row_down->paramOutGemo(0), set_map->paramInGemo(1));
    script->addConnect(for_col_down->paramOutGemo(0), set_map->paramInGemo(2));
    script->addConnect(get_map2->paramOutGemo(0), set_map->paramInGemo(3));
    script->addConnect(for_col_down->subFlowOutGemo(0), set_map->flowInGemo());    

    set_map2->setParamInValue(1, "0");
    set_map2->setParamInValue(3, QString::number(-1));
    script->addConnect(for_col_top->paramOutGemo(0), set_map2->paramInGemo(2));

    script->addConnect(for_row_down->flowOutGemo(), for_col_top->flowInGemo());
    script->addConnect(for_col_top->subFlowOutGemo(0), set_map2->flowInGemo());

    script->addConnect(line_count_eq_col_branch->flowOutGemo(1), node_set_row->flowInGemo());
}