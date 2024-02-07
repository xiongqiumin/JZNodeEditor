#include "JZNodeView.h"
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
#include <QWheelEvent>
#include <algorithm>
#include <cmath>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QPolygon>
#include <QComboBox>
#include <QInputDialog>
#include "JZNodeFactory.h"
#include "JZNodeValue.h"
#include "JZNodeCompiler.h"
#include "JZProject.h"
#include "JZNodeEngine.h"
#include "JZNodeLayout.h"
#include "JZNodeFunction.h"

JZNodeViewCommand::JZNodeViewCommand(JZNodeView *view,int type)
{
    itemId = -1;
    command = type;
    m_view = view;
}

int JZNodeViewCommand::id() const
{
    if(command == MoveNode)
        return command;

    return -1;
}

void JZNodeViewCommand::setItemPos(JZNodeGraphItem *item,QPointF pos)
{
    m_view->setRecordMove(false);
    item->setPos(pos);
    m_view->setRecordMove(true);
}

QVariant JZNodeViewCommand::saveItem(JZNodeGraphItem *item)
{
    QByteArray buffer;
    QDataStream s(&buffer,QIODevice::WriteOnly);
    m_view->getNode(itemId)->saveToStream(s);
    return buffer;
}

void JZNodeViewCommand::loadItem(JZNodeGraphItem *item,const QVariant &value)
{
    QByteArray buffer = value.toByteArray();
    QDataStream s(&buffer,QIODevice::ReadOnly);
    item->node()->loadFromStream(s);
    item->updateNode();
}

void JZNodeViewCommand::undo()
{
    if(command == CreateNode)
    {        
        m_view->removeNode(itemId);
    }
    else if(command == RemoveNode)
    {
        auto node = loadNode(oldValue.toByteArray());
        node->setId(itemId);
        auto item = m_view->insertNode(JZNodePtr(node));
        setItemPos(item,oldPos);
    }
    else if(command == MoveNode)
    {        
        auto item = m_view->getNodeItem(itemId);
        setItemPos(item,oldPos);
    }
    else if(command == CreateLine)
    {        
        m_view->removeLine(itemId);
    }
    else if(command == RemoveLine)
    {
        auto line = parseLine(oldValue.toByteArray());
        line.id = itemId;
        m_view->insertLine(line);
    }
    else if(command == PropertyChange)
    {
        m_view->setNodeData(itemId,oldValue.toByteArray());
    }
}

void JZNodeViewCommand::redo()
{
    if(command == CreateNode)
    {        
        auto node = loadNode(newValue.toByteArray());        
        JZNodeGraphItem *item = nullptr;
        if(itemId == -1)
        {
            item = m_view->createNode(JZNodePtr(node));
            itemId = item->id();
        }
        else
        {
            node->setId(itemId);
            item = m_view->insertNode(JZNodePtr(node));
        }
        setItemPos(item,newPos);
    }
    else if(command == RemoveNode)
    {
        m_view->removeNode(itemId);
    }
    else if(command == MoveNode)
    {        
        auto item = m_view->getNodeItem(itemId);
        setItemPos(item,newPos);
    }
    else if(command == CreateLine)
    {
        auto line = parseLine(newValue.toByteArray());
        if(itemId == -1)
        {
            auto item = m_view->createLine(line.from,line.to);
            itemId = item->id();
        }
        else
        {
            line.id = itemId;
            m_view->insertLine(line);
        }
    }
    else if(command == RemoveLine)
    {
        m_view->removeLine(itemId);
    }
    else if(command == PropertyChange)
    {
        if(newValue.isNull())
        {
            newValue = m_view->getNodeData(itemId);
            m_view->updateNode(itemId);
        }
        else
        {
            m_view->setNodeData(itemId,newValue.toByteArray());
        }
    }
}

bool JZNodeViewCommand::mergeWith(const QUndoCommand *command)
{
    JZNodeViewCommand *other = (JZNodeViewCommand*)command;
    if(itemId != other->itemId)
        return false;

    newPos = other->newPos;
    return true;
}


//CopyData
struct CopyData
{
    CopyData();
    bool isEmpty();

    QList<QByteArray> nodes;
    QList<QPointF> nodesPos;
    QList<JZNodeConnect> lines;
};

CopyData::CopyData()
{

}

bool CopyData::isEmpty()
{
    return nodes.isEmpty();
}

QByteArray pack(const CopyData &param)
{
    QByteArray buffer;
    QDataStream s(&buffer,QIODevice::WriteOnly);
    s << param.nodes;
    s << param.nodesPos;
    s << param.lines;  
    return buffer;
}

CopyData unpack(const QByteArray &buffer)
{
    CopyData param;
    QDataStream s(buffer);
    s >> param.nodes;
    s >> param.nodesPos;
    s >> param.lines;  
    return param;
}

//BreakPointTriggerResult
BreakPointTriggerResult::BreakPointTriggerResult()
{
    type = none;
    nodeId = 0;
}

JZNodeView *g_nodeView = nullptr;
//JZNodeView
JZNodeView::JZNodeView(QWidget *widget)
    : QGraphicsView(widget)
{
    m_selLine = nullptr;    
    m_tip = nullptr;    
    m_propEditor = nullptr;
    m_loadFlag = false;    
    m_file = nullptr;    
    m_recordMove = true;
    m_propEditFlag = false;
    m_runningMode = false;
    m_runNode = -1;    
    g_nodeView = this;

    connect(&m_commandStack,&QUndoStack::cleanChanged, this, &JZNodeView::onCleanChanged);
    connect(&m_commandStack,&QUndoStack::canRedoChanged,this,&JZNodeView::redoAvailable);
    connect(&m_commandStack,&QUndoStack::canUndoChanged,this,&JZNodeView::undoAvailable);
    connect(&m_commandStack,&QUndoStack::indexChanged, this,&JZNodeView::onUndoStackChanged);

    m_scene = new JZNodeScene();    
    setScene(m_scene);
    setAcceptDrops(true);

    m_compilerTimer = new QTimer();
    connect(m_compilerTimer,&QTimer::timeout,this,&JZNodeView::onAutoCompiler);
    m_compilerTimer->setSingleShot(true);        

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &JZNodeView::customContextMenuRequested, this, &JZNodeView::onContextMenu);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);        

    setAlignment(Qt::AlignTop | Qt::AlignLeft);

    setRenderHint(QPainter::Antialiasing);    
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}

JZNodeView::~JZNodeView()
{
    this->disconnect();
}

void JZNodeView::drawBackground(QPainter* painter, const QRectF& r)
{
  QGraphicsView::drawBackground(painter, r);

  auto drawGrid =
    [&](double gridStep)
    {
      QRect   windowRect = rect();
      QPointF tl = mapToScene(windowRect.topLeft());
      QPointF br = mapToScene(windowRect.bottomRight());

      double left   = std::floor(tl.x() / gridStep - 0.5);
      double right  = std::floor(br.x() / gridStep + 1.0);
      double bottom = std::floor(tl.y() / gridStep - 0.5);
      double top    = std::floor (br.y() / gridStep + 1.0);

      // vertical lines
      for (int xi = int(left); xi <= int(right); ++xi)
      {
        QLineF line(xi * gridStep, bottom * gridStep,
                    xi * gridStep, top * gridStep );

        painter->drawLine(line);
      }

      // horizontal lines
      for (int yi = int(bottom); yi <= int(top); ++yi)
      {
        QLineF line(left * gridStep, yi * gridStep,
                    right * gridStep, yi * gridStep );
        painter->drawLine(line);
      }
    };

    QColor BackgroundColor(255, 255, 240);
    QColor FineGridColor(245, 245, 230);
    QColor CoarseGridColor(235, 235, 220);

  QBrush bBrush = backgroundBrush();

  QPen pfine(FineGridColor, 1.0);

  painter->setPen(pfine);
  drawGrid(15);

  QPen p(CoarseGridColor, 1.0);

  painter->setPen(p);
  drawGrid(150);
}

void JZNodeView::setPropertyEditor(JZNodePropertyEditor *propEditor)
{
    m_propEditor = propEditor;
    connect(m_propEditor, &JZNodePropertyEditor::sigNodePropChanged, this, &JZNodeView::onPropUpdate);
    connect(m_propEditor, &JZNodePropertyEditor::sigNodePropNameChanged, this, &JZNodeView::onPropNameUpdate);
}

void JZNodeView::setFile(JZScriptFile *file)
{
    m_file = file;
    initGraph();
    autoCompiler();
}

bool JZNodeView::isModified()
{
    return !m_commandStack.isClean();
}

int JZNodeView::getVariableType(const QString &name)
{
    auto info = JZNodeCompiler::getVariableInfo(m_file, name);
    return info? info->dataType : Type_none;    
}

void JZNodeView::saveNodePos()
{
    foreachNode([this](JZNodeGraphItem *node) {
        m_file->setNodePos(node->id(), node->pos());
    });
}

JZNode *JZNodeView::getNode(int id)
{
    return m_file->getNode(id);
}

JZNodePin *JZNodeView::getPin(JZNodeGemo gemo)
{
    return m_file->getPin(gemo);
}

JZNodeGraphItem *JZNodeView::createNode(JZNodePtr node)
{        
    int id = m_file->addNode(node);
    return createNodeItem(id);
}

JZNodeGraphItem *JZNodeView::insertNode(JZNodePtr node)
{
    m_file->insertNode(node);
    return createNodeItem(node->id());
}

void JZNodeView::moveNode(int id,QPointF pos)
{    
    getNodeItem(id)->setPos(pos);
}

void JZNodeView::removeNode(int id)
{
    auto item = getNodeItem(id);
    if (!item)
        return;

    if(m_propEditor->node() == item->node())
        setSelectNode(-1);

    Q_ASSERT(m_file->getConnectId(id).size() == 0);
    m_scene->removeItem(item);
    delete item;
    m_file->removeNode(id);        
}

QByteArray JZNodeView::getNodeData(int id)
{
    auto node = getNode(id);

    QByteArray buffer;
    QDataStream s(&buffer,QIODevice::WriteOnly);
    node->saveToStream(s);

    return buffer;
}

void JZNodeView::setNodeData(int id,const QByteArray &buffer)
{
    auto node = getNode(id);
    QDataStream s(buffer);
    node->loadFromStream(s);

    updateNode(id);
}

void JZNodeView::pinClicked(int nodeId,int pinId)
{
    auto node = getNode(nodeId);
    QByteArray oldValue = getNodeData(nodeId);
    if(node->pinClicked(pinId))
        addPropChangedCommand(nodeId,oldValue);
}


bool JZNodeView::isPropEditable(int id,int prop_id)
{
    if(!getNode(id)->prop(prop_id)->isEditValue())
        return false;

    QList<int> lines = m_file->getConnectInput(id,prop_id);
    return lines.size() == 0;
}

JZNodeGraphItem *JZNodeView::createNodeItem(int id)
{
    JZNodeGraphItem *item = new JZNodeGraphItem(m_file->getNode(id));
    m_scene->addItem(item);
    item->updateNode();
    item->update();    
    return item;
}

JZNodeGraphItem *JZNodeView::getNodeItem(int id)
{
    auto items = m_scene->items();
    for (int i = 0; i < items.size(); i++)
    {
        if (items[i]->type() == Item_node && ((JZNodeGraphItem *)items[i])->id() == id)
            return (JZNodeGraphItem *)items[i];
    }
    return nullptr;
}

void JZNodeView::setNodePropValue(int nodeId, int prop_id, QString value)
{
    auto item = getNodeItem(nodeId);    
    item->setPropValue(prop_id, value);
}

QString JZNodeView::getNodePropValue(int nodeId, int prop_id)
{
    auto item = getNodeItem(nodeId);
    return item->propValue(prop_id);    
}

void JZNodeView::longPressCheck(int nodeId)
{
    QTimer::singleShot(200, this, [this,nodeId] {
        auto node = getNodeItem(nodeId);
        if (node)
            node->updateLongPress();
    });
}

void JZNodeView::updateNode(int id)
{
    getNodeItem(id)->updateNode();
    auto lineId = m_file->getConnectId(id);
    for (int i = 0; i < lineId.size(); i++)
        getLineItem(lineId[i])->updateNode();

    if(getNode(id) == m_propEditor->node())
    {
        if(!m_propEditFlag)
            m_propEditor->updateNode();
    }
}

void JZNodeView::updatePropEditable(const JZNodeGemo &gemo)
{
    getNodeItem(gemo.nodeId)->updateNode();

    auto node = m_propEditor->node();    
    if(node && gemo.nodeId == node->id())
        m_propEditor->setPropEditable(gemo.propId,isPropEditable(gemo.nodeId,gemo.propId));
}

JZNodeLineItem *JZNodeView::createLine(JZNodeGemo from, JZNodeGemo to)
{
    int id = m_file->addConnect(from, to);    
    updatePropEditable(to);    
    return createLineItem(id);
}

JZNodeLineItem *JZNodeView::insertLine(const JZNodeConnect &connect)
{
    m_file->insertConnect(connect);
    updatePropEditable(connect.to);
    return createLineItem(connect.id);
}

void JZNodeView::removeLine(int id)
{
    auto item = getLineItem(id);
    if (!item)
        return;

    m_file->removeConnect(id);
    JZNodeGemo to = item->endTraget();    
    updatePropEditable(to);

    m_scene->removeItem(item);
    delete item;
}

JZNodeLineItem *JZNodeView::createLineItem(int id)
{
    auto info = m_file->getConnect(id);
    JZNodeLineItem *line = new JZNodeLineItem(info->from);
    line->setEndTraget(info->to);
    line->setId(info->id);
    m_scene->addItem(line);
    line->updateNode();

    return line;
}

JZNodeLineItem *JZNodeView::getLineItem(int id)
{
    auto items = m_scene->items();
    for (int i = 0; i < items.size(); i++)
    {
        if (items[i]->type() == Item_line && ((JZNodeLineItem *)items[i])->id() == id)
            return (JZNodeLineItem *)items[i];
    }
    return nullptr;
}

void JZNodeView::startLine(JZNodeGemo from)
{
    if (m_selLine)
        return;

    JZNodeGraphItem *node_from = getNodeItem(from.nodeId);
    auto pt = node_from->mapToScene(node_from->propRect(from.propId).center());

    m_selLine = new JZNodeLineItem(from);
    m_selLine->setZValue(1);
    m_selLine->setEndPoint(pt);
    m_scene->addItem(m_selLine);
    m_selLine->grabMouse();
    m_selLine->updateNode();
}

void JZNodeView::endLine(JZNodeGemo to)
{
    if (!m_selLine)
        return;

    JZNodeConnect line;
    line.from = m_selLine->startTraget();
    line.to = to;

    JZNodeViewCommand *cmd = new JZNodeViewCommand(this,JZNodeViewCommand::CreateLine);
    cmd->itemId = -1;
    cmd->newValue = formatLine(line);
    m_commandStack.push(cmd);

    delete m_selLine;
    m_selLine = nullptr;    
}

void JZNodeView::cancelLine()
{
    if (!m_selLine)
        return;

    m_selLine->ungrabMouse();
    if (m_selLine->endTraget().nodeId != INVALID_ID)
    {
        m_selLine->setDrag(false);
    }
    else
    {
        m_scene->removeItem(m_selLine);
        delete m_selLine;
        m_scene->update();
    }
    m_selLine = nullptr;
}

void JZNodeView::addTip(QRectF tipArea,QString text)
{
    if(!m_tip)
        m_tip = m_scene->addText("");

    m_tipArea = tipArea;
    m_tip->setHtml("<div style='background-color:#FFFFFF;'>" + text + "</div>");
    m_tip->setPos(tipArea.topRight());
    m_tip->setZValue(10);
}

void JZNodeView::clearTip()
{
    if(m_tip)
    {
        m_scene->removeItem(m_tip);
        delete m_tip;
        m_tip = nullptr;
    }
}

JZNodeGraphItem *JZNodeView::nodeItemAt(QPoint pos)
{
    QList<QGraphicsItem *> list = m_scene->items(mapToScene(pos));
    for(int i = 0; i < list.size(); i++)
    {
        if(list[i]->type() == Item_node)
            return dynamic_cast<JZNodeGraphItem*>(list[i]);
    }
    return nullptr;
}

void JZNodeView::foreachNode(std::function<void(JZNodeGraphItem *)> func, int nodeType)
{
    auto items = m_scene->items();
    for (int i = 0; i < items.size(); i++)
    {
        if (items[i]->type() == Item_node)
        {
            JZNodeGraphItem *node_item = (JZNodeGraphItem *)items[i];
            if (nodeType == -1 || node_item->node()->type() == nodeType)
                func(node_item);
        }
    }
}

void JZNodeView::foreachLine(std::function<void(JZNodeLineItem *)> func)
{
    auto items = m_scene->items();
    for (int i = 0; i < items.size(); i++)
    {
        if (items[i]->type() == Item_line)
            func((JZNodeLineItem *)items[i]);
    }
}

QVariant JZNodeView::onItemChange(JZNodeBaseItem *item, QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (m_loadFlag)
        return value;

    if (change == QGraphicsItem::ItemPositionHasChanged)
    {
        if (item->type() == Item_node)
        {
            int node_id = ((JZNodeGraphItem *)item)->node()->id();
            auto lineId = m_file->getConnectId(node_id);
            for (int i = 0; i < lineId.size(); i++)
                getLineItem(lineId[i])->updateNode();

            if(m_recordMove)
            {
                JZNodeViewCommand *cmd = new JZNodeViewCommand(this,JZNodeViewCommand::MoveNode);
                cmd->itemId = node_id;
                cmd->oldPos = m_file->getNodePos(node_id);
                cmd->newPos = item->pos();
                m_commandStack.push(cmd);
            }
        }
    }
    else if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        auto list = m_scene->selectedItems();
        if(list.size() > 0)
        {
            for(int i = 0; i < list.size(); i++)
            {
                if(list[i]->type() == Item_node)
                {
                    auto node = ((JZNodeGraphItem *)list[i])->id();
                    setSelectNode(node);
                    break;
                }
            }
        }
        else
        {
            setSelectNode(-1);
        }
    }
    return value;
}

void JZNodeView::initGraph()
{
    m_loadFlag = true;
    m_scene->clear();

    QList<int> node_list = m_file->nodeList();
    for (int i = 0; i < node_list.size(); i++)
    {
        auto item = createNodeItem(node_list[i]);
        item->setPos(m_file->getNodePos(item->id()));
        item->updateNode();
    }
    auto lines = m_file->connectList();
    for (int i = 0; i < lines.size(); i++)
        createLineItem(lines[i].id);

    m_loadFlag = false;
    m_scene->update();
    m_commandStack.clear();
        
    setSceneRect(sceneRect().translated(-100, -100));    
}

void JZNodeView::clear()
{
    m_scene->clear();
    m_file->clear();
    m_commandStack.clear();    
}

void JZNodeView::save()
{
    auto project = m_file->project();
    saveNodePos();
    project->saveItem(m_file);
    m_commandStack.setClean();
}

void JZNodeView::redo()
{
    m_commandStack.redo();    
}

void JZNodeView::undo()
{
    m_commandStack.undo();    
}

void JZNodeView::remove()
{
    auto items = m_scene->selectedItems();
    removeItems(items);
}

void JZNodeView::cut()
{
    auto items = m_scene->selectedItems();
    copyItems(items);
    removeItems(items);
}

void JZNodeView::copy()
{
    auto items = m_scene->selectedItems();
    copyItems(items);
}

void JZNodeView::paste()
{
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    if(!mimeData->hasFormat("jznode_copy_data"))
        return;

    CopyData copyData = unpack(mimeData->data("jznode_copy_data"));    
    m_commandStack.beginMacro("paste");
    QMap<int,int> nodeIdMap;
    QPolygonF ploy;
    for(int i = 0; i < copyData.nodesPos.size(); i++)
        ploy << copyData.nodesPos[i];
    QPointF topLeft = ploy.boundingRect().topLeft();
    for(int i = 0; i < copyData.nodesPos.size(); i++)
        copyData.nodesPos[i] -= topLeft;

    QPoint cur_pos = mapFromGlobal(QCursor::pos());
    QPointF offset;
    if(this->rect().contains(cur_pos))
        offset = mapToScene(cur_pos);
    else
        offset = mapToScene(0,0);

    for(int i = 0; i < copyData.nodesPos.size(); i++)
    {
        JZNode *node = loadNode(copyData.nodes[i]);
        int old_id = node->id();
        node->setId(-1);

        JZNodeViewCommand *cmd = new JZNodeViewCommand(this,JZNodeViewCommand::CreateNode);
        cmd->itemId = -1;
        cmd->newValue = saveNode(node);
        cmd->newPos = copyData.nodesPos[i] + offset;
        m_commandStack.push(cmd);

        nodeIdMap[old_id] = cmd->itemId;
    }

    //保留原有节点间线段关系
    for(int i = 0; i < copyData.lines.size(); i++)
    {
        JZNodeConnect line = copyData.lines[i];
        line.id = -1;
        line.from.nodeId = nodeIdMap[line.from.nodeId];
        line.to.nodeId = nodeIdMap[line.to.nodeId];

        JZNodeViewCommand *cmd = new JZNodeViewCommand(this,JZNodeViewCommand::CreateLine);
        cmd->itemId = -1;
        cmd->newValue = formatLine(line);
        m_commandStack.push(cmd);
    }
    m_commandStack.endMacro();
}

void JZNodeView::selectAll()
{
    setFocus();
    m_scene->clearSelection();
    auto items = m_scene->items();
    for (int i = 0; i < items.size(); i++)
    {
        if (items[i]->type() == Item_node || items[i]->type() == Item_line)
            items[i]->setSelected(true);
    }
}

void JZNodeView::onCleanChanged(bool clean)
{
    emit modifyChanged(!clean);
}

void JZNodeView::setRecordMove(bool flag)
{
    m_recordMove = flag;
}

void JZNodeView::setSelectNode(int id)
{
    if(id != -1)
    {
        auto node = getNode(id);
        m_propEditor->setNode(node);

        auto list = node->paramInList();
        for(int i = 0; i < list.size(); i++)
            m_propEditor->setPropEditable(list[i],isPropEditable(id,list[i]));
    }
    else
    {
        m_propEditor->setNode(nullptr);
    }

}

void JZNodeView::updateNodeLayout()
{    
    m_recordMove = false;
    
    m_commandStack.beginMacro("layout");

    JZNodeCompiler compiler;
    QVector<GraphPtr> graph_list;
    if (!compiler.genGraphs(m_file, graph_list))
    {
        QMessageBox::information(this, "", compiler.error());
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
            item->setPos(col_start[tree_node->col], y_offset + row_start[tree_node->row]);            
            max_y = qMax(max_y,(int)item->pos().y() + item->size().width());

            //add command
            JZNodeViewCommand *cmd = new JZNodeViewCommand(this, JZNodeViewCommand::MoveNode);
            cmd->itemId = node_id;
            cmd->oldPos = m_file->getNodePos(node_id);
            cmd->newPos = item->pos();
            m_commandStack.push(cmd);

            it++;
        }
        y_offset = max_y + 50;
    }
    m_commandStack.endMacro();
    m_recordMove = true;

    fitNodeView();
}

void JZNodeView::fitNodeView()
{        
    this->setSceneRect(QRectF());
    this->fitInView(m_scene->itemsBoundingRect(), Qt::KeepAspectRatio);    
}

void JZNodeView::ensureNodeVisible(int id)
{
    auto item = getNodeItem(id);

    QRectF view_rc = mapToScene(rect()).boundingRect();    
    auto item_rc = item->mapRectToScene(item->boundingRect());
    if (!view_rc.contains(item_rc))
    {        
        setSceneRect(QRectF());
        this->centerOn(item);
    }
}

void JZNodeView::selectNode(int id)
{
    ensureNodeVisible(id);
    m_scene->clearSelection();
    getNodeItem(id)->setSelected(true);
}

BreakPointTriggerResult JZNodeView::breakPointTrigger()
{
    BreakPointTriggerResult ret;

    auto items = m_scene->selectedItems();
    for(int i = 0; i < items.size(); i++)
    {
        if(items[i]->type() == Item_node)
        {                        
            auto node_item = (JZNodeGraphItem*)items[i];
            int node_id = node_item->id();
            auto project = m_file->project();
            QString filepath = m_file->itemPath();

            ret.filename = filepath;
            ret.nodeId = node_item->id();
            if(project->hasBreakPoint(filepath,node_id))
            {
                ret.type = BreakPointTriggerResult::remove;
                project->removeBreakPoint(filepath,node_id);
                node_item->update();
            }
            else
            {
                ret.type = BreakPointTriggerResult::add;
                project->addBreakPoint(filepath,node_id);
                node_item->update();
            }
            break;
        }
    }

    return ret;
}

void JZNodeView::setRunning(bool flag)
{
    m_runningMode = flag;
    setInteractive(!m_runningMode);
    foreachNode([flag](JZNodeGraphItem *node) {
        node->setFlag(QGraphicsItem::ItemIsMovable, !flag);
    });    

    if (!m_runningMode)
    {
        m_runNode = -1;
        foreachNode([flag](JZNodeGraphItem *node) {
            node->resetPropValue();
        });
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
        preItem->update();
    m_runNode = nodeId;
    if (nodeId == -1)
        return;
        
    auto item = getNodeItem(m_runNode);
    item->update();    
    selectNode(nodeId);
}

bool JZNodeView::isBreakPoint(int nodeId)
{
    auto project = m_file->project();
    return project->hasBreakPoint(m_file->itemPath(), nodeId);
}

void JZNodeView::removeItem(QGraphicsItem *item)
{        
    Q_ASSERT(item->type() > Item_none);
        
    auto item_id = ((JZNodeBaseItem *)item)->id();
    if (item->type() == Item_node)
    {
        if(!getNode(item_id)->canRemove())
            return;

        m_commandStack.beginMacro("remove");
        auto lines = m_file->getConnectId(item_id);
        for (int i = 0; i < lines.size(); i++)
        {
            auto line = m_file->getConnect(lines[i]);

            JZNodeViewCommand *cmd = new JZNodeViewCommand(this,JZNodeViewCommand::RemoveLine);
            cmd->itemId = line->id;
            cmd->oldValue = formatLine(*line);
            m_commandStack.push(cmd);
        }

        JZNodeViewCommand *cmd = new JZNodeViewCommand(this,JZNodeViewCommand::RemoveNode);
        auto node = m_file->getNode(item_id);
        cmd->itemId = node->id(); 
        cmd->oldValue = saveNode(node);
        cmd->oldPos = item->pos();
        m_commandStack.push(cmd);

        m_commandStack.endMacro();
    }
    else if (item->type() == Item_line)
    {
        auto line = m_file->getConnect(item_id);
        JZNodeViewCommand *cmd = new JZNodeViewCommand(this,JZNodeViewCommand::RemoveLine);
        cmd->itemId = line->id;
        cmd->oldValue = formatLine(*line);
        m_commandStack.push(cmd);
    }
}

QStringList JZNodeView::matchParmas(JZNodeObjectDefine *meta, int match_type, QString pre)
{
    QStringList list;
    auto params = meta->paramList(true);
    for (int i = 0; i < params.size(); i++)
    {
        auto p = meta->param(params[i]);
        if (JZNodeType::canConvert(p->dataType,match_type))
            list << pre + p->name;
        if (JZNodeType::isObject(p->dataType))
        {
            auto sub_meta = JZNodeObjectManager::instance()->meta(p->dataType);
            list << matchParmas(sub_meta, match_type, pre + p->name + ".");
        }
    }
    return list;
}

void JZNodeView::onContextMenu(const QPoint &pos)
{
    if (m_runningMode)
        return;

    QMenu menu(this);

    auto item = itemAt(pos);
    QAction *actAdd = nullptr;
    QAction *actCpy = nullptr;
    QAction *actDel = nullptr;
    QAction *actPaste = nullptr;
    QAction *actGoto = nullptr;
    int pin_id = -1;
    QList<QAction*> pin_actions;
    QList<QAction*> param_actions;
    if (!item)
    {
        actAdd = menu.addAction("添加节点");
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
            auto node = ((JZNodeGraphItem*)item)->node();

            auto scene_pos = mapToScene(pos);
            auto item_pos = node_item->mapFromScene(scene_pos);
            pin_id = node_item->propAt(item_pos);

            QStringList actions_list;
            if(pin_id >= 0)
                actions_list = node_item->node()->pinActionList(pin_id);

            if (actions_list.size() > 0)
            {
                for (int i = 0; i < actions_list.size(); i++)
                    pin_actions << menu.addAction(actions_list[i]);
            }
            else
            {
                if (node->canDragVariable())
                {
                    auto classFile = m_file->getClassFile();
                    if (classFile)
                    {
                        auto meta = JZNodeObjectManager::instance()->meta(classFile->className());
                        QStringList param_list = matchParmas(meta, node->variableType(), "this.");
                        if (param_list.size() > 0)
                        {
                            auto tmp_menu = menu.addMenu("设置参数");
                            for (int i = 0; i < param_list.size(); i++)
                                param_actions << tmp_menu->addAction(param_list[i]);
                        }
                    }                    
                }
                if (node->type() == Node_function)
                {
                    auto func = (JZNodeFunction*)node;
                    auto func_def = JZNodeFunctionManager::instance()->function(func->function());
                    if (func_def && !func_def->isCFunction)
                    {
                        QString fullName = func->function();
                        actGoto = menu.addAction("跳转到");
                        actGoto->setProperty("filePath",fullName);
                    }
                }

                actCpy = menu.addAction("复制节点");
                actDel = menu.addAction("删除节点");
            }
        }
        else
            actDel = menu.addAction("删除连线");
    }

    QAction *ret = menu.exec(this->mapToGlobal(pos));
    if(!ret)
        return;

    if (ret == actAdd)
    {       
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
        auto node = ((JZNodeGraphItem*)item)->node();
        auto pre_list = node->propList();

        auto old = getNodeData(node->id());
        if (node->pinActionTriggered(pin_id, index))
        {
            auto cur_data = getNodeData(node->id());
            setNodeData(node->id(), old);

            m_commandStack.beginMacro(ret->text());
            auto new_list = node->propList();
            auto remove_set = pre_list.toList().toSet() - new_list.toList().toSet();
            for (auto remove_prop : remove_set)
            {
                auto lines = m_file->getConnectId(node->id(),remove_prop);                                
                for (int i = 0; i < lines.size(); i++)                
                    addRemoveLineCommand(lines[i]);                
            }            
            setNodeData(node->id(), cur_data);
            addPropChangedCommand(node->id(), old);
            m_commandStack.endMacro();
        }
    }
    else if (param_actions.contains(ret))
    {
        auto node = ((JZNodeGraphItem*)item)->node();
        auto old = getNodeData(node->id());
        node->setVariable(ret->text());        
        addPropChangedCommand(node->id(), old);
    }
    else if(ret == actGoto)
    {
        QString filePath = actGoto->property("filePath").toString();
        emit sigFunctionOpen(filePath);
    }
}

void JZNodeView::addCreateNodeCommand(const QByteArray &buffer,QPointF pt)
{
    JZNodeViewCommand *cmd = new JZNodeViewCommand(this,JZNodeViewCommand::CreateNode);
    cmd->itemId = -1;
    cmd->newValue = buffer;
    cmd->newPos = pt;
    m_commandStack.push(cmd);
}

void JZNodeView::addRemoveLineCommand(int line_id)
{
    auto line = m_file->getConnect(line_id);

    JZNodeViewCommand *cmd = new JZNodeViewCommand(this, JZNodeViewCommand::RemoveLine);
    cmd->itemId = line->id;
    cmd->oldValue = formatLine(*line);
    m_commandStack.push(cmd);
}

void JZNodeView::addPropChangedCommand(int id,const QByteArray &oldValue)
{
    JZNodeViewCommand *cmd = new JZNodeViewCommand(this,JZNodeViewCommand::PropertyChange);
    cmd->itemId = id;
    cmd->oldValue = oldValue;
    m_commandStack.push(cmd);
}

void JZNodeView::dragEnterEvent(QDragEnterEvent *event)
{    
    if (event->mimeData()->hasFormat("node_data")
        || event->mimeData()->hasFormat("node_param"))
        event->acceptProposedAction();
}

void JZNodeView::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void JZNodeView::dropEvent(QDropEvent *event)
{
    if (m_runningMode)
        return;

    if(event->mimeData()->hasFormat("node_data"))
    {
        QByteArray data = event->mimeData()->data("node_data");
        QDataStream s(&data,QIODevice::ReadOnly);
        int node_type;
        s >> node_type;
        if(node_type == Node_expr)
        {
            QString text = getExpr();
            if(text.isEmpty())
                return;

            JZNodePtr node = JZNodePtr(loadNode(data));
            QString error;
            JZNodeExpression *expr = dynamic_cast<JZNodeExpression *>(node.data());
            if(!expr->setExpr(text,error))
                return;

            data = saveNode(node.data());
        }
        addCreateNodeCommand(data,mapToScene(event->pos()));
        event->accept();
    }
    else if(event->mimeData()->hasFormat("node_param"))
    {
        QString param_name = QString::fromUtf8(event->mimeData()->data("node_param"));
        JZNodeGraphItem *node_item = nodeItemAt(event->pos());
        if(!node_item)
        {
            int sel = getSelect({ "Get","Set" });
            if (sel == 0)
            {
                JZNodeParam node_param;
                node_param.setVariable(param_name);
                addCreateNodeCommand(saveNode(&node_param), mapToScene(event->pos()));
            }
            else if(sel == 1)
            {
                JZNodeSetParam set_param;
                set_param.setVariable(param_name);
                addCreateNodeCommand(saveNode(&set_param), mapToScene(event->pos()));
            }
        }
        else if(node_item->node()->canDragVariable())
        {
            QByteArray old = getNodeData(node_item->node()->id());
            node_item->node()->drag(param_name);
            addPropChangedCommand(node_item->id(),old);
        }
        event->accept();
    }
}

void JZNodeView::wheelEvent(QWheelEvent *event)
{
    double scale = this->transform().m11();
    if(event->angleDelta().y() > 0)//鼠标滚轮向前滚动
    {
        if(scale >= 4)
            return;

        scale*=1.1;//每次放大10%
        scale = qMin(scale,4.0);
    }
    else
    {
        scale*=0.9;//每次缩小10%
    }

    auto targetViewportPos = event->pos();
    auto targetScenePos = mapToScene(event->pos());

    resetTransform();
    this->scale(scale, scale);
    centerOn(targetScenePos);

    QPointF deltaViewportPos = targetViewportPos - QPointF(viewport()->width() / 2.0, viewport()->height() / 2.0);
    QPointF viewportCenter = mapFromScene(targetScenePos) - deltaViewportPos;
    centerOn(mapToScene(viewportCenter.toPoint()));

    event->accept();
}

void JZNodeView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
    viewport()->setCursor(QCursor());
    if (event->button() == Qt::LeftButton)
        m_downPoint = mapToScene(event->pos());
    if (m_runningMode)
    {        
        auto item = nodeItemAt(event->pos());
        if (item)
        {
            m_scene->clearSelection();
            item->setSelected(true);
        }        
    }
}

void JZNodeView::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    viewport()->setCursor(QCursor());

    if (m_selLine)
    {
        m_selLine->setEndPoint(mapToScene(event->pos()));

        JZNodeGraphItem *node_item = nodeItemAt(event->pos());
        if(node_item && m_selLine->startTraget().nodeId != node_item->id())
        {
            auto scene_pos = mapToScene(event->pos());
            auto item_pos = node_item->mapFromScene(scene_pos);
            auto pin_id = node_item->propAt(item_pos);
            if(pin_id >= 0)
            {
                JZNodeGemo to(node_item->id(), pin_id);
                QString error;
                if(!m_file->canConnect(m_selLine->startTraget(),to,error))
                {
                    auto rc = node_item->propRect(pin_id);
                    rc = node_item->mapToScene(rc).boundingRect();
                    addTip(rc,"无法连接: " + error);
                }
            }
            else
                clearTip();
        }
    }
    else if(m_tip)
    {
        if(!m_tipArea.contains(mapToScene(event->pos())))
            clearTip();
    }

    if (scene()->mouseGrabberItem() == nullptr && event->buttons() == Qt::LeftButton)
    {
      // Make sure shift is not being pressed
      if ((event->modifiers() & Qt::ShiftModifier) == 0)
      {
        QPointF difference = m_downPoint - mapToScene(event->pos());
        setSceneRect(sceneRect().translated(difference.x(), difference.y()));
      }
    }
}

void JZNodeView::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    viewport()->setCursor(QCursor());

    if (m_selLine)
    {
        JZNodeGemo gemo;
        JZNodeGraphItem *node_item = nodeItemAt(event->pos());
        if (node_item && m_selLine->startTraget().nodeId != node_item->id())
        {            
            auto pos = node_item->mapFromScene(mapToScene(event->pos()));
            auto pin_id = node_item->propAt(pos);
            if(pin_id >= 0)
            {                
                QString error;
                JZNodeGemo to(node_item->id(), pin_id);
                if(m_file->canConnect(m_selLine->startTraget(),to, error))
                    gemo = to;
            }
        }
        if (gemo.nodeId != INVALID_ID)
            endLine(gemo);
        else
            cancelLine();
        clearTip();
    }        
}

void JZNodeView::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Shift)
        setDragMode(QGraphicsView::RubberBandDrag);

    QGraphicsView::keyReleaseEvent(event);
}

void JZNodeView::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Shift)
        setDragMode(QGraphicsView::NoDrag);
    else if(event->key() == Qt::Key_Delete)
        remove();
    else if(event->key() == Qt::Key_Escape){
        cancelLine();        
    }

    QGraphicsView::keyReleaseEvent(event);
}

void JZNodeView::onItemPropChanged()
{
    QObject *obj = sender();
    int node_id = obj->property("node_id").toInt();
    int prop_id = obj->property("prop_id").toInt();
    
    QVariant value;
    if (obj->inherits("QComboBox"))
    {
        QComboBox *box = qobject_cast<QComboBox*>(obj);
        value = box->currentData();
    }
    else if (obj->inherits("QLineEdit"))
    {
        QLineEdit *line = qobject_cast<QLineEdit*>(obj);
        value = line->text();
    }    
    auto node = getNode(node_id);
    if (value == node->propValue(prop_id))
        return;
    
    m_propEditFlag = true;    
    auto old = getNodeData(node_id);
    node->setPropValue(prop_id, value);
    addPropChangedCommand(node->id(), old);
    m_propEditFlag = false;
}

void JZNodeView::onPropNameUpdate(int id,int propId,const QString &value)
{
    QVariant oldValue = getNode(id)->propName(propId);
    if(oldValue == value)
        return;

    m_propEditFlag = true;
    auto node = getNode(id);
    auto old = getNodeData(id);
    node->setPropName(propId,value);    
    addPropChangedCommand(node->id(),old);
    m_propEditFlag = false;
}

void JZNodeView::onPropUpdate(int id,int propId,const QVariant &value)
{
    QVariant oldValue = getNode(id)->propValue(propId);
    if(oldValue == value)
        return;

    m_propEditFlag = true;
    auto node = getNode(id);
    auto old = getNodeData(id);
    node->setPropValue(propId,value);    
    addPropChangedCommand(node->id(),old);
    m_propEditFlag = false;
}

void JZNodeView::copyItems(QList<QGraphicsItem*> items)
{
    CopyData copydata;

    QVector<int> node_ids;
    for (int i = 0; i < items.size(); i++)
    {
        auto item = items[i];
        if (item->type() == Item_node)
        {
            JZNodeGraphItem *node = (JZNodeGraphItem*)item;
            if (node->node()->canRemove())
            {
                node_ids << ((JZNodeGraphItem*)item)->id();
                QByteArray data = saveNode(node->node());
                copydata.nodes.append(data);
                copydata.nodesPos.append(item->pos());
            }
        }
    }
    
    for(int i = 0; i < items.size(); i++)
    {
        auto item = items[i];
        if(item->type() == Item_line)
        {
            JZNodeLineItem *line = (JZNodeLineItem*)item;
            JZNodeConnect connect = *m_file->getConnect(line->id());
            if(node_ids.contains(connect.from.nodeId) && node_ids.contains(connect.to.nodeId))
                copydata.lines << connect;
        }
    }
    
    QMimeData *mimeData = new QMimeData();
    mimeData->setData("jznode_copy_data",pack(copydata));
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setMimeData(mimeData);
}

void JZNodeView::removeItems(QList<QGraphicsItem*> items)
{
    m_commandStack.beginMacro("remove");
    std::sort(items.begin(),items.end(),[](const QGraphicsItem *i1,const QGraphicsItem *i2)->bool{
            return i1->type() < i2->type();
        });

    for(int i = 0; i < items.size(); i++)
    {        
        removeItem(items[i]);
    }
    m_commandStack.endMacro();
}

void JZNodeView::autoCompiler()
{    
    m_compilerTimer->start(1000);
}

void JZNodeView::onAutoCompiler()
{
    if (m_runningMode)
        return;
    
    foreachNode([](JZNodeGraphItem *node){
        node->clearError();
    });

    JZNodeCompiler compiler;
    JZNodeScript result;
    if(!compiler.build(m_file,&result))
    {
        auto compilerInfo = compiler.compilerInfo();
        auto &nodeInfo = compilerInfo.nodeInfo;
        auto it = nodeInfo.begin();
        while(it != nodeInfo.end())
        {
            if(!it->error.isEmpty())
                getNodeItem(it.key())->setError(it->error);
            it++;
        }
    }
}

QString JZNodeView::getExpr()
{
    QString text = QInputDialog::getText(this,"请输入表达式","");
    return text;
}

int JZNodeView::getSelect(QStringList list)
{
    QMenu menu;
    for (int i = 0; i < list.size(); i++)
        menu.addAction(list[i]);
    auto act = menu.exec(QCursor::pos());
    if (!act)
        return -1;
    int sel = menu.actions().indexOf(act);
    return sel;
}

void JZNodeView::onUndoStackChanged()
{
    autoCompiler();
}