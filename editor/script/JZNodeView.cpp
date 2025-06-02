#include <QMouseEvent>
#include <QDebug>
#include <QMimeData>
#include <QMenu>
#include <QLabel>
#include <QFormLayout>
#include <QLineEdit>
#include <QGraphicsProxyWidget>
#include <QMessageBox>
#include <QTimer>
#include <QShowEvent>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QPolygon>
#include <QComboBox>
#include <QInputDialog>
#include <QToolTip>
#include <QScrollBar>
#include <QCursor>

#include "JZNodeFactory.h"
#include "JZNodeValue.h"
#include "JZNodeCompiler.h"
#include "JZProject.h"
#include "JZNodeEngine.h"
#include "JZNodeLayout.h"
#include "JZNodeFunction.h"
#include "JZNodeGroupEditDialog.h"
#include "JZNodeBuilder.h"
#include "LogManager.h"
#include "JZNodeExprEditDialog.h"
#include "JZNodePanel.h"
#include "JZNodeFlowPanel.h"
#include "JZNodeUtils.h"
#include "JZNodeGraphItem.h"
#include "JZEditorGlobal.h"
#include "JZNodeView.h"
#include "JZNodeDisplayItem.h"
#include "JZScriptItemVisitor.h"
#include "JZNodeParamEditWidget.h"
#include "JZNodeUtils.h"

//JZNodeView
JZNodeView::JZNodeView(QWidget *widget)
    : JZNodeAbstractView(widget)
{         
    m_propEditor = nullptr;
    m_runningMode = Process_none;
    m_runNode = -1;        
    m_isUpdateFlowPanel = false;
    m_editProxy = nullptr;    
    
    m_mouseMoveTimer = new QTimer(this);
    connect(m_mouseMoveTimer, &QTimer::timeout, this, &JZNodeView::onMouseMoveTimer);

    m_editWidgetTimer = new QTimer(this);
    connect(m_editWidgetTimer, &QTimer::timeout, this, &JZNodeView::onEditWidgetTimer);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &JZNodeView::customContextMenuRequested, this, &JZNodeView::onContextMenu);
}

JZNodeView::~JZNodeView()
{
    this->disconnect();
}

JZAbstractNodeItem *JZNodeView::createNodeItem(JZNode *node)
{
    return JZNodeEditorManager::instance()->createNodeItem(node);
}

JZAbstractLineItem *JZNodeView::createLineItem(JZNodeGemo gemo)
{
    return new JZNodeLineItem(gemo);
}

void JZNodeView::setPropertyEditor(JZNodePropertyEditor *propEditor)
{
    m_propEditor = propEditor;
    connect(m_propEditor, &JZNodePropertyEditor::sigNodePropChanged, this, &JZNodeView::onNodePinValueChanged);    
}

void JZNodeView::setRunEditor(JZNodeAutoRunWidget *runEditor)
{
    m_runEditor = runEditor;    
}

void JZNodeView::setPanel(JZNodePanel *panel)
{
    m_panel = panel;
}

void JZNodeView::setFlowPanel(JZNodeFlowPanel *panel)
{
    m_flowPanel = panel;
}

void JZNodeView::udpateFlowPanel()
{
    if (m_isUpdateFlowPanel)
        return;

    m_isUpdateFlowPanel = true;
    QTimer::singleShot(0, this, [this]{
        m_flowPanel->updateFlow(m_file);
        m_isUpdateFlowPanel = false;
    });
}

void JZNodeView::editPinValue(int node_id, int pin_id)
{
    if (m_editProxy)
        editFinish();

    auto node_item = getNodeItem(node_id);
    node_item->setBaseZValue(5);

    auto item = dynamic_cast<JZNodeGraphItem*>(node_item);
    auto block = item->block(pin_id);
    QRectF rect = item->mapToScene(block->valueRect).boundingRect();

    JZNodeParamValueWidget *widget = new JZNodeParamValueWidget();
    connect(widget, &JZNodeParamValueWidget::sigEditFinish, this, &JZNodeView::onEditFinish);
    widget->setProperty("nodeId", node_id);
    widget->setProperty("pinId", pin_id);    
    widget->setProperty("JZNodeViewEdit", true);    

    widget->init(block->edit);
    widget->setValue(item->pinValue(pin_id));    

    m_editProxy = new QGraphicsProxyWidget();
    m_editProxy->setZValue(10);
    m_editProxy->setWidget(widget);
    m_editProxy->setGeometry(rect);
    m_scene->addItem(m_editProxy);

    QTimer::singleShot(0, [this] {
        m_editProxy->widget()->setFocus();
    });
    m_editWidgetTimer->start(100);
}

void JZNodeView::editFinish()
{    
    if (m_editProxy)
    {
        JZNodeParamValueWidget *w = qobject_cast<JZNodeParamValueWidget*>(m_editProxy->widget());
        int node_id = w->property("nodeId").toInt();
        int pin_id = w->property("pinId").toInt();
        QString value = w->value();

        auto item = dynamic_cast<JZNodeGraphItem*>(getNodeItem(node_id));
        item->setBaseZValue(0);
        m_scene->removeItem(m_editProxy);
        m_editProxy = nullptr;

        if (pin_id < MAX_PIN_ID)
            onNodePinValueChanged(node_id, pin_id, value);
        else
            item->setPinValue(pin_id, value);
    }
    m_editWidgetTimer->stop();
}

void JZNodeView::onEditWidgetTimer()
{
    bool has_focus = false;

    auto item = m_scene->focusItem();
    while (item)
    {
        if (item == m_editProxy)
        {
            has_focus = true;
            break;
        }
        item = item->parentItem();
    }
    
    if(!has_focus)
        editFinish();    
}

void JZNodeView::onEditFinish()
{
    editFinish();
}

void JZNodeView::updateNodeLayout()
{    
    m_recordMove = false;
    
    m_commandStack.beginMacro("layout");

    JZNodeCompiler compiler;
    QVector<GraphPtr> graph_list;
    if (!compiler.genGraphs(m_file, graph_list))
    {
        QMessageBox::information(this, "", "编译失败");
        return;
    }

    JZNodeLayoutTree tree;
    tree.m_view = this;

    int gap = 20;
    int y_offset = 0;
    int max_y = 0;
    for (int graph_idx = 0; graph_idx < graph_list.size(); graph_idx++)
    {
        auto graph = graph_list[graph_idx].data();
        tree.make(graph);

        QVector<int> col_start;
        QVector<int> row_start;
        QVector<int> row_max;
        QVector<int> col_max;
        row_max.resize(tree.max_row + 1);
        col_max.resize(tree.max_col + 1);
        row_start.resize(tree.max_row + 1);
        col_start.resize(tree.max_col + 1);

        auto it = tree.m_nodeMap.begin();
        while (it != tree.m_nodeMap.end())
        {
            int node_id = it.key();
            auto item = getNodeItem(node_id);
            auto tree_node = it->data();
            row_max[tree_node->row] = qMax(item->size().height(), row_max[tree_node->row]);
            col_max[tree_node->col] = qMax(item->size().width(), col_max[tree_node->col]);
            it++;
        }        
        for (int i = 0; i < row_max.size(); i++)
        {
            if (row_max[i] != 0)
                row_max[i] += gap;
        }
        for (int i = 0; i < col_max.size(); i++)
        {
            if (col_max[i] != 0)
                col_max[i] += gap;
        }        
        for (int i = 1; i < col_start.size(); i++)
        {
            col_start[i] = col_start[i - 1] + col_max[i - 1];
        }
        for (int i = 1; i < row_start.size(); i++)
        {
            row_start[i] = row_start[i - 1] + row_max[i - 1];
        }

        it = tree.m_nodeMap.begin();
        while (it != tree.m_nodeMap.end())
        {
            int node_id = it.key();
            auto item = getNodeItem(node_id);
            auto tree_node = it->data();            

            QPointF new_pos(col_start[tree_node->col], y_offset + row_start[tree_node->row]);
            addMoveNodeCommand(node_id, new_pos);
            max_y = qMax(max_y,(int)item->pos().y() + item->size().width());            

            it++;
        }
        y_offset = max_y + 50;
    }
    m_commandStack.endMacro();
    m_recordMove = true;
    fitNodeView();
}

void JZNodeView::breakPointTrigger()
{    
    auto items = m_scene->selectedItems();
    for(int i = 0; i < items.size(); i++)
    {
        if(items[i]->type() == Item_node)
        {                        
            auto node_item = (JZNodeGraphItem*)items[i];
            int node_id = node_item->id();
            auto project = m_file->project();
            QString filepath = m_file->itemPath();            
            if(project->hasBreakPoint(filepath,node_id))
            {                
                project->removeBreakPoint(filepath,node_id);
                node_item->update();
            }
            else
            {
                BreakPoint pt;
                pt.scriptItemPath = filepath;
                pt.nodeId = node_item->id();
                pt.type = BreakPoint::nodeEnter;
                project->addBreakPoint(pt);
                node_item->update();
            }
            break;
        }
    }
}

ProcessStatus JZNodeView::runningMode()
{
    return m_runningMode;
}

void JZNodeView::setRunningMode(ProcessStatus status)
{
    m_runningMode = status;
        
    bool flag = (status == Process_none);    
    //setInteractive(flag);
    foreachNode([flag](JZAbstractNodeItem *node) {
        node->setFlag(QGraphicsItem::ItemIsMovable, flag);
    });    

    if (m_runningMode == Process_none)
    {
        m_runNode = -1;    
    }
}

int JZNodeView::runtimeNode()
{
    return m_runNode;
}

void JZNodeView::setRuntimeNode(int nodeId)
{
    auto preItem = getNodeItem(m_runNode);
    if (preItem)
    {        
        preItem->update();
    }
    m_runNode = nodeId;
    if (nodeId == -1)
        return;
        
    auto item = getNodeItem(m_runNode);
    item->update();    
    selectNode(nodeId);
}

void JZNodeView::clearRuntimeValue()
{
    foreachNode([](JZAbstractNodeItem *node){
        auto item = dynamic_cast<JZNodeGraphItem*>(node);
        item->clearRuntimeValue();
    });
}

void JZNodeView::setRuntimeValue(int node_id,int pin_id,const JZNodeDebugParamValue &value)
{
    auto item = getNodeItem(node_id);
    if(!item)
        return;

    //item->setPinRuntimeValue(pin_id, value);
}

void JZNodeView::clearDisplayValue()
{
    foreachNode(Node_display, [](JZAbstractNodeItem *node) {
        auto display = dynamic_cast<JZNodeDisplayItem*>(node);
        display->clearValues();
    });
}

void JZNodeView::displayValue(int node_id,int pin_id,QVariantPtr *ptr)
{
    auto item = getNodeItem(node_id);
    if(!item)
        return;

    auto lines = m_file->getConnectOut(node_id,pin_id);
    for(int i = 0; i < lines.size(); i++)
    {
        auto line = m_file->getConnect(lines[i]);
        if (getNode(line->to.nodeId)->type() != Node_display)
            continue;
        
        JZNodeDisplayItem *item = dynamic_cast<JZNodeDisplayItem*>(getNodeItem(line->to.nodeId));        
        item->setValue(line->to.pinId, ptr);
    }
}

bool JZNodeView::isBreakPoint(int nodeId)
{
    auto project = m_file->project();
    return project->hasBreakPoint(m_file->itemPath(), nodeId);
}

void JZNodeView::onNodePinValueChanged(int id, int pinId, const QString &value)
{
    if (m_runningMode != Process_none)
    {
        int param_id = JZNodeCompiler::paramId(id, pinId);
        emit sigRuntimeValueChanged(param_id, value);
        return;
    }

    QString oldValue = getNode(id)->pinValue(pinId);
    if (oldValue == value)
        return;

    addPinValueChangedCommand(id, pinId, value);
}

void JZNodeView::onContextMenu(const QPoint &pos)
{
    if (m_runningMode != Process_none)
        return;

    auto func_inst = editorFunctionManager();
    QMenu menu(this);    

    auto item = itemAt(pos);
    QList<QAction*> actAddList;
    QList<QTreeWidgetItem*> actAddItemList;
    QAction *actCpy = nullptr;
    QAction *actDel = nullptr;
    QAction *actPaste = nullptr;
    QAction *actFuncGoto = nullptr;
    QAction *actFuncRename = nullptr;
    QAction *actEditGroup = nullptr;
    QAction *actSetExpr = nullptr;
    int pin_id = -1;
    QList<QAction*> pin_actions;    
    QList<QAction*> node_actions;
    JZNode *node = nullptr;
    if (!item)
    {
        QMenu* addMenu = menu.addMenu("添加节点");
        QMenu* menu_op = addMenu->addMenu("操作");
        
        auto item_op = m_panel->itemOp();
        for (int i = 0; i < item_op->childCount(); i++)
        {
            auto child = item_op->child(i);
            actAddItemList << child;
            actAddList << menu_op->addAction(child->text(0));
        }

        QMenu* menu_stat = addMenu->addMenu("过程");
        auto item_process = m_panel->itemProcess();
        for (int i = 0; i < item_process->childCount(); i++)
        {
            auto child = item_process->child(i);
            actAddItemList << child;
            actAddList << menu_stat->addAction(child->text(0));
        }
        
        actPaste = menu.addAction("粘贴");                
    }
    else
    {
        if(!item->isSelected())
        {
            m_scene->clearSelection();
            item->setSelected(true);
        }
        if (item->type() == Item_node)
        {
            JZNodeGraphItem* node_item = (JZNodeGraphItem*)item;
            node = ((JZNodeGraphItem*)item)->node();

            auto scene_pos = mapToScene(pos);
            auto item_pos = node_item->mapFromScene(scene_pos);            
            pin_id = node_item->pinAtInName(item_pos);

            actCpy = menu.addAction("复制节点");
            actDel = menu.addAction("删除节点");            
        }
        else if (item->type() == Item_line)
            actDel = menu.addAction("删除连线");
        else if (item->type() == Item_group)
        {
            actEditGroup = menu.addAction("编辑分组");
            actDel = menu.addAction("删除分组");
        }
    }

    auto node_list = selectNodeItems();    
    int same_group_id = -1;
    QAction *actAddNodeGroup = nullptr;
    QAction *actRemoveNodeGroup = nullptr;
    QAction *actMergeNodeGroup = nullptr;
    if (node_list.size() > 0 && pin_actions.size() == 0)
    {          
        QSet<int> group_ids;        
        int vaild_group_num = 0;
        for (int i = 0; i < node_list.size(); i++)
        {
            if (node_list[i]->node()->group() != -1)
            {             
                group_ids.insert(node_list[i]->node()->group());
                vaild_group_num++;
            }            
        }    
        if (group_ids.size() == 0 || group_ids.size() == 1)
            menu.addSeparator();

        if (group_ids.size() == 0)
            actAddNodeGroup = menu.addAction("创建分组");
        if (group_ids.size() == 1)
        {
            same_group_id = *group_ids.begin();
            if(vaild_group_num == node_list.size())
                actRemoveNodeGroup = menu.addAction("取消分组");
            else
                actMergeNodeGroup = menu.addAction("合并分组");
        }                
    }

    QAction *ret = menu.exec(this->mapToGlobal(pos));
    if(!ret)
        return;

    if (actAddList.contains(ret))
    {       
        int idx = actAddList.indexOf(ret);
        QByteArray buffer = actAddItemList[idx]->data(0,TreeItem_value).toByteArray();
        QPointF node_pos = mapToScene(pos);
        addCreateNodeCommand(buffer, node_pos);
    }
    else if(ret == actCpy)
    {
        copy();
    }
    else if(ret == actPaste)
    {
        paste();
    }
    else if(ret == actDel)
    {
        removeItem(item);
    }
    else if (pin_actions.contains(ret))
    {
        int index = pin_actions.indexOf(ret);
        auto node = dynamic_cast<JZNodeGraphItem*>(item)->node();        
        auto old = getNodeData(node->id());
        addNodeChangedCommand(node->id(), old);        
    }
    else if(node_actions.contains(ret))
    {
        int index = node_actions.indexOf(ret);
        auto node = dynamic_cast<JZNodeGraphItem*>(item)->node(); 
        auto old = getNodeData(node->id());
        addNodeChangedCommand(node->id(), old);
    }
    else if(ret == actFuncGoto)
    {
        QString filePath = actFuncGoto->data().toString();
        emit sigFunctionOpen(filePath);
    }
    else if (ret == actFuncRename)
    {
        QString function = actFuncRename->data().toString();
        bool ok;
        auto text = QInputDialog::getText(this, "", "函数名:", QLineEdit::Normal, function, &ok);
        if (!ok || text == function)
            return;
        if (!func_inst->function(text))
        {
            QMessageBox::information(this, "", "函数不存在");
            return;
        }

        auto node_func = dynamic_cast<JZNodeFunction*>(node);
        auto old = getNodeData(node_func->id());        
        node_func->setFunction(func_inst->function(text));
        addNodeChangedCommand(node_func->id(), old);
    }
    else if (ret == actEditGroup)
    {
        int group_id = dynamic_cast<JZNodeGroupItem*>(item)->id();
        JZNodeGroupEditDialog dlg(this);
        dlg.setGroup(*m_file->getGroup(group_id));
        if (dlg.exec() != QDialog::Accepted)
            return;
    
        addSetGroupCommand(group_id, dlg.group());
    }
    else if (ret == actAddNodeGroup)
    {
        JZNodeGroupEditDialog dlg(this);
        if (dlg.exec() != QDialog::Accepted)
            return;

        m_commandStack.beginMacro("add group");
        JZNodeGroup group = dlg.group();
        int id = addCreateGroupCommand(group);
        for (int i = 0; i < node_list.size(); i++)
        {
            QByteArray oldValue = getNodeData(node_list[i]->id());
            node_list[i]->node()->setGroup(id);
            addNodeChangedCommand(node_list[i]->id(), oldValue);
        }
        m_commandStack.endMacro();
    }
    else if (ret == actRemoveNodeGroup)
    {        
        m_commandStack.beginMacro("remove group");
        for (int i = 0; i < node_list.size(); i++)
        {
            QByteArray oldValue = getNodeData(node_list[i]->id());
            node_list[i]->node()->setGroup(-1);
            addNodeChangedCommand(node_list[i]->id(), oldValue);
        }
        auto list = m_file->groupNodeList(same_group_id);
        if(list.size() == 0)
            addRemoveGroupCommand(same_group_id);
        m_commandStack.endMacro();
    }
    else if (ret == actMergeNodeGroup)
    {
        m_commandStack.beginMacro("add group");                
        for (int i = 0; i < node_list.size(); i++)
        {            
            if (node_list[i]->node()->group() == -1)
            {
                QByteArray oldValue = getNodeData(node_list[i]->id());
                node_list[i]->node()->setGroup(same_group_id);
                addNodeChangedCommand(node_list[i]->id(), oldValue);
            }
        }
        m_commandStack.endMacro();
    }
}

void JZNodeView::dragEnterEvent(QDragEnterEvent *event)
{    
    if (event->mimeData()->hasFormat("node_data")
        || event->mimeData()->hasFormat("node_param")
        || event->mimeData()->hasFormat("node_class"))
        event->acceptProposedAction();
}

void JZNodeView::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void JZNodeView::dropEvent(QDropEvent *event)
{    
    if (m_runningMode != Process_none)
        return;
    
    auto env = editorEnvironment();
    auto obj_inst = env->objectManager();
    auto func_inst = env->functionManager();
    auto factory = editorNodeFactory();
    if(event->mimeData()->hasFormat("node_data"))
    {
        QByteArray node_data = event->mimeData()->data("node_data");                       
        addCreateNodeCommand(node_data,mapToScene(event->pos()));
        event->accept();
    }
    else if(event->mimeData()->hasFormat("node_param"))       
    {
        QMenu menu(this);        
        QAction *actSet = nullptr;        
        
        QString param_name = QString::fromUtf8(event->mimeData()->data("node_param"));            
        QAction *actGet = menu.addAction("Get");
        if (param_name != "this")
            actSet = menu.addAction("Set");        

        auto def = JZNodeCompiler::getVariableInfo(m_file,param_name);
        Q_ASSERT(def);
        int data_type = env->nameToType(def->type);
        if(data_type >= Type_class || data_type == Type_string)
        {
            auto meta = obj_inst->meta(data_type);
            if(meta)
            {
                QMenu *menuCall = nullptr;
                auto func_list = meta->functionList();
                std::sort(func_list.begin(), func_list.end());
                for(int i = 0; i < func_list.size(); i++)
                {
                    auto func = meta->function(func_list[i]);
                    if(func->isMemberFunction())
                    {
                        if(!menuCall)
                            menuCall = menu.addMenu("Call");
                            
                        auto tmp = menuCall->addAction(func_list[i]);
                        tmp->setData(func->fullName());
                    }
                }
            }
        }

        auto act = menu.exec(QCursor::pos());
        if (!act)
            return;
            
        if (act == actGet)
        {
            JZNodeParam node_param;
            node_param.setVariable(param_name);
            addCreateNodeCommand(factory->saveNode(&node_param), mapToScene(event->pos()));
        }
        else if(act == actSet)
        {
            JZNodeSetParam set_param;
            set_param.setVariable(param_name);
            addCreateNodeCommand(factory->saveNode(&set_param), mapToScene(event->pos()));
        }
        else
        {
            JZNodeFunction function;
            function.setFunction(func_inst->function(act->data().toString()));
            function.setVariable(param_name);
            addCreateNodeCommand(factory->saveNode(&function), mapToScene(event->pos()));
        }  

        event->accept();
    }
    else if (event->mimeData()->hasFormat("node_class"))
    {
        QMenu menu(this);
        QList<QAction*> call_list;

        QString class_name = QString::fromUtf8(event->mimeData()->data("node_class"));
        auto meta = obj_inst->meta(class_name);
        if (meta)
        {
            QMenu *menuCall = nullptr;
            auto func_list = meta->functionList();
            std::sort(func_list.begin(), func_list.end());
            for (int i = 0; i < func_list.size(); i++)
            {
                auto func = meta->function(func_list[i]);
                if (func->isMemberFunction())
                {
                    if (!menuCall)
                        menuCall = menu.addMenu("Call");

                    auto tmp = menuCall->addAction(func_list[i]);
                    tmp->setData(func->fullName());
                    call_list << tmp;
                }
            }
        }

        auto act = menu.exec(QCursor::pos());
        if (!act)
            return;

        if (call_list.contains(act))
        {
            JZNodeFunction function;
            function.setFunction(func_inst->function(act->data().toString()));
            addCreateNodeCommand(factory->saveNode(&function), mapToScene(event->pos()));
        }

        event->accept();
    }
}

int JZNodeView::propEditorNodeId()
{
    auto node = m_propEditor->node();
    if (!node)
        return -1;
    else
        return node->id();
}

QList<int> JZNodeView::watchList()
{    
    QList<int> watchList;

    auto node_list = m_file->nodeList();
    for (int i = 0; i < node_list.size(); i++)
    {
        auto node = m_file->getNode(node_list[i]);
        if (node->type() != Node_display)
            continue;

        auto inputs = m_file->getConnectInput(node->id());
        for (int i = 0; i < inputs.size(); i++)
        {
            auto line = m_file->getConnect(inputs[i]);
            if (!watchList.contains(line->from.paramId()))
                watchList << line->from.paramId();
        }
    }

    return watchList;
}