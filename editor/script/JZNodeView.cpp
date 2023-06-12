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
#include <QInputDialog>
#include "JZNodeFactory.h"
#include "JZNodeValue.h"
#include "JZNodeCompiler.h"

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
    m_view->setMoveUndo(false);
    item->setPos(pos);
    m_view->setMoveUndo(true);
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
        auto node = parseNode(oldValue.toByteArray());
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
    else if(command == PropertyNameChange)
    {
        m_view->setPinName(itemId,propId,oldValue.toString());
    }
    else if(command == PropertyValueChange)
    {
        m_view->setPinValue(itemId,propId,oldValue);
    }
}

void JZNodeViewCommand::redo()
{
    if(command == CreateNode)
    {        
        auto node = parseNode(newValue.toByteArray());        
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
    else if(command == PropertyNameChange)
    {
        m_view->setPinName(itemId,propId,newValue.toString());
    }
    else if(command == PropertyValueChange)
    {
        m_view->setPinValue(itemId,propId,newValue);
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

//JZNodeView
JZNodeView::JZNodeView(QWidget *widget)
    : QGraphicsView(widget)
{
    m_selLine = nullptr;
    m_selArea = nullptr;
    m_tip = nullptr;    
    m_propEditor = nullptr;
    m_loadFlag = false;    
    m_file = nullptr;
    m_isMove = false;    
    m_moveUndo = false;

    connect(&m_commandStack,&QUndoStack::canRedoChanged,this,&JZNodeView::redoAvailable);
    connect(&m_commandStack,&QUndoStack::canUndoChanged,this,&JZNodeView::undoAvailable);

    m_scene = new JZNodeScene();    
    setScene(m_scene);
    setAcceptDrops(true);

    QTimer *timer = new QTimer();
    connect(timer,&QTimer::timeout,this,&JZNodeView::onTimer);
    //timer->start(2000);

    setWindowFlags(Qt::BypassGraphicsProxyWidget);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &JZNodeView::customContextMenuRequested, this, &JZNodeView::onContextMenu);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);    

    m_scene->setSceneRect(QRect(0,0,1000,1000));
}

JZNodeView::~JZNodeView()
{
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
    connect(m_propEditor, &JZNodePropertyEditor::sigPropChanged, this, &JZNodeView::onPropUpdate);
    connect(m_propEditor, &JZNodePropertyEditor::sigPropNameChanged, this, &JZNodeView::onPropNameUpdate);
}

void JZNodeView::setFile(JZScriptFile *file)
{
    m_file = file;
    initGraph();
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
    m_file->setNodePos(id,pos);
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

void JZNodeView::addPin(int id,JZNodePin pin)
{

}

void JZNodeView::removePin(int id,int prop_id)
{

}

void JZNodeView::setPinName(int id,int prop_id,const QString &value)
{
    auto node = getNode(id);
    node->setPropName(prop_id,value);
    getNodeItem(id)->updateNode();

    auto lineId = m_file->getConnectId(id);
    for (int i = 0; i < lineId.size(); i++)
        getLineItem(lineId[i])->updateNode();

    if(node == m_propEditor->node())
        m_propEditor->setPropName(prop_id,value);
}

void JZNodeView::setPinValue(int id,int prop_id,const QVariant &value)
{
    auto node = getNode(id);
    node->setPropValue(prop_id,value);
    getNodeItem(id)->updateNode();

    auto lineId = m_file->getConnectId(id);
    for (int i = 0; i < lineId.size(); i++)
        getLineItem(lineId[i])->updateNode();

    if(node == m_propEditor->node())
        m_propEditor->setPropValue(prop_id,value);
}

bool JZNodeView::isPropEditable(int id,int prop_id)
{
    if(!getNode(id)->prop(prop_id)->isDispValue())
        return false;

    QList<int> lines = m_file->getConnectId(id,prop_id);
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

void JZNodeView::updatePropEditable(const JZNodeGemo &gemo)
{
    auto node = m_propEditor->node();
    if(!node)
        return;

    if(gemo.nodeId == node->id())
        m_propEditor->setPropEditable(gemo.propId,isPropEditable(gemo.nodeId,gemo.propId));
}

JZNodeLineItem *JZNodeView::createLine(JZNodeGemo from, JZNodeGemo to)
{
    int id = m_file->addConnect(from, to);
    updatePropEditable(from);
    updatePropEditable(to);
    return createLineItem(id);
}

JZNodeLineItem *JZNodeView::insertLine(const JZNodeConnect &connect)
{
    m_file->insertConnect(connect);
    return createLineItem(connect.id);
}

void JZNodeView::removeLine(int id)
{
    auto item = getLineItem(id);
    if (!item)
        return;

    JZNodeGemo from = item->startTraget();
    JZNodeGemo to = item->endTraget();
    updatePropEditable(from);
    updatePropEditable(to);

    m_scene->removeItem(item);
    delete item;
    m_file->removeConnect(id);       
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
    m_selLine->setEndPoint(pt);
    m_scene->addItem(m_selLine);
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

void JZNodeView::createTip(QPointF pos,QString text)
{
    if(!m_tip)
        m_tip = m_scene->addText("");

    m_tip->setHtml("<div style='background-color:#FFFFFF;'>" + text + "</div>");
    m_tip->setPos(pos);
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

void JZNodeView::cancelSelect()
{
    if(!m_selArea)
        return;

    m_scene->clearSelection();
    QPainterPath path;
    path.addRect(m_selArea->rect());
    m_scene->setSelectionArea(path);
    m_scene->removeItem(m_selArea);
    delete m_selArea;
    m_selArea = nullptr;
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

            if(m_moveUndo)
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
}

void JZNodeView::clear()
{
    m_scene->clear();
    m_file->clear();
    m_commandStack.clear();
    m_scene->setSceneRect(QRect(0,0,1000,1000));
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
        JZNode *node = parseNode(copyData.nodes[i]);
        int old_id = node->id();
        node->setId(-1);

        JZNodeViewCommand *cmd = new JZNodeViewCommand(this,JZNodeViewCommand::CreateNode);
        cmd->itemId = -1;
        cmd->newValue = formatNode(node);
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

void JZNodeView::setMoveUndo(bool flag)
{
    m_moveUndo = flag;
}

void JZNodeView::setSelectNode(int id)
{
    if(id != -1)
    {
        auto node = getNode(id);
        m_propEditor->setNode(node);

        auto list = node->propListByType(Prop_param);
        for(int i = 0; i < list.size(); i++)
            m_propEditor->setPropEditable(list[i],isPropEditable(id,list[i]));
    }
    else
    {
        m_propEditor->setNode(nullptr);
    }

}

void JZNodeView::setDepth(GraphNode *node,int depth,Graph *graph,QMap<GraphNode*,int> &result)
{
    result[node] = depth;
    for(int i = 0; i < node->next.size(); i++)
    {
        auto next = graph->graphNode(node->next[i].nodeId);
        if(!result.contains(next))
            setDepth(next,depth-1,graph,result);
    }
    auto it = node->paramIn.begin();
    while(it != node->paramIn.end())
    {
        auto &in_list = it.value();
        for(int i = 0; i < in_list.size(); i++)
        {
            auto prev = graph->graphNode(in_list[i].nodeId);
            if(!result.contains(prev))
                setDepth(prev,depth+1,graph,result);
        }
        it++;
    }
}

int JZNodeView::nodeDepth(GraphNode *node,Graph *graph)
{
    int depth = 0;
    for(int i = 0; i < node->next.size(); i++)
    {
        depth = qMax(depth,nodeDepth(graph->graphNode(node->next[i].nodeId),graph) + 1);
    }
    return depth;
}

void JZNodeView::updateNodeLayout()
{
    JZNodeCompiler compiler;
    JZNodeScript result;
    if(!compiler.build(m_file,&result))
        return;   

    int y = 0;
    auto graph_list = result.graphs;
    for(int graph_idx = 0; graph_idx < graph_list.size(); graph_idx++)
    {
        auto graph = graph_list[graph_idx].data();
        auto list = graph->topolist;               

        GraphNode *start = nullptr;
        //查找最长链
        int max_depth = 0;
        for(int i = 0; i < list.size(); i++)
        {
            if(list[i]->paramIn.size() == 0)
            {
                int depth = nodeDepth(list[i],graph);
                if(depth > max_depth)
                {
                    max_depth = depth;
                    start = list[i];
                }
            }
        }
        //计算深度
        QMap<GraphNode*,int> depthMap;
        setDepth(start,max_depth,graph,depthMap);

        //设置坐标
        QVector<QVector<int>> nodeLevel;
        nodeLevel.resize(max_depth + 1);

        auto it = depthMap.begin();
        while (it != depthMap.end())
        {
            nodeLevel[max_depth - it.value()].push_back(it.key()->node->id());
            it++;
        }

        //设置
        int x = 0;
        int max_h = 0;
        for(int col = 0; col < nodeLevel.size(); col++)
        {
            auto &row_list = nodeLevel[col];
            int cur_y = y;
            int max_w = 0;
            for(int row = 0; row < row_list.size(); row++)
            {
                auto item = getNodeItem(row_list[row]);
                item->setPos(x,cur_y);
                max_w = qMax(max_w,(int)item->boundingRect().width());
                cur_y += item->boundingRect().height();
            }
            x += max_w + 20;
            max_h = qMax(max_h,cur_y);
        }
        y = max_h + 20;
    }
    QRectF scene_rc = m_scene->itemsBoundingRect();
    if(scene_rc.width() < 1000)
    {
        int w = (1000 - scene_rc.width())/2;
        scene_rc.adjust(-w,0,w,0);
    }
    if(scene_rc.height() < 600)
    {
        int h = (600 - scene_rc.height())/2;
        scene_rc.adjust(0,-1,0,h);
    }
    m_scene->setSceneRect(scene_rc);
    fitInView(rect(),Qt::KeepAspectRatio);
}

void JZNodeView::removeItem(QGraphicsItem *item)
{        
    Q_ASSERT(item->type() > Item_none);
        
    auto item_id = ((JZNodeBaseItem *)item)->id();
    if (item->type() == Item_node)
    {
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
        cmd->oldValue = formatNode(node);
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

void JZNodeView::onContextMenu(const QPoint &pos)
{
    QMenu menu(this);

    auto item = itemAt(pos);
    QAction *actAdd = nullptr;
    QAction *actCpy = nullptr;
    QAction *actDel = nullptr;
    QAction *actPaste = nullptr;
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
            actCpy = menu.addAction("复制节点");
            actDel = menu.addAction("删除节点");
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
}

void JZNodeView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("node_data"))
        event->acceptProposedAction();
}

void JZNodeView::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void JZNodeView::dropEvent(QDropEvent *event)
{
    QByteArray data = event->mimeData()->data("node_data");
    QDataStream s(&data,QIODevice::ReadOnly);
    int node_type;
    s >> node_type;
    if(node_type == Node_expr)
    {
        QString text = QInputDialog::getText(this,"请输入表达式","");
        if(text.isEmpty())
            return;

        JZNodePtr node = JZNodePtr(parseNode(data));
        QString error;
        JZNodeExpression *expr = dynamic_cast<JZNodeExpression *>(node.data());
        if(!expr->setExpr(text,error))
            return;

        data = formatNode(node.data());
    }

    JZNodeViewCommand *cmd = new JZNodeViewCommand(this,JZNodeViewCommand::CreateNode);
    cmd->itemId = -1;
    cmd->newValue = data;
    cmd->newPos = mapToScene(event->pos());
    m_commandStack.push(cmd);

    event->accept();
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

void JZNodeView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_selLine)
    {
        m_selLine->setEndPoint(mapToScene(event->pos()));
    }
    QGraphicsView::mouseMoveEvent(event);

    if(m_isMove)
    {
        QPointF gap = mapToScene(event->pos()) - mapToScene(m_downPoint);
        QPointF old_center = mapToScene(viewport()->rect().width()/2,viewport()->rect().height()/2);
        centerOn(old_center - gap);

        m_downPoint = event->pos();
    }
    else if(m_selArea)
    {        
        m_selArea->setRect(QRectF(mapToScene(m_downPoint),mapToScene(event->pos())));
    }
    else if (m_selLine)
    {
        JZNodeGraphItem *node_item = nodeItemAt(event->pos());
        if(node_item && m_selLine->startTraget().nodeId != node_item->id())
        {
            auto scene_pos = mapToScene(event->pos());
            auto item_pos = node_item->mapFromScene(scene_pos);
            auto prop = node_item->propAt(item_pos);
            if(prop)
            {
                JZNodeGemo to(node_item->id(), prop->id());
                QString error;
                if(!m_file->canConnect(m_selLine->startTraget(),to,error))
                    createTip(mapToScene(event->pos() + QPoint(10,0)),"无法连接: " + error);
            }
            else
                clearTip();
        }
    }
}

void JZNodeView::mousePressEvent(QMouseEvent *event)
{
    if (m_selLine)
    {
        event->ignore();
        return;
    }
    if(!itemAt(event->pos())){
        m_downPoint = event->pos();
        if(event->modifiers() & Qt::SHIFT)
        {
            Q_ASSERT(!m_selArea);
            m_selArea = m_scene->addRect(QRectF(),QPen(),QBrush());
            m_selArea->setZValue(100);
        }
        else
            m_isMove = true;
    }
    QGraphicsView::mousePressEvent(event);
}

void JZNodeView::mouseReleaseEvent(QMouseEvent *event)
{
    if(m_isMove)
        m_isMove = false;    
    if (m_selLine)
    {
        JZNodeGemo gemo;
        JZNodeGraphItem *node_item = nodeItemAt(event->pos());
        if (node_item && m_selLine->startTraget().nodeId != node_item->id())
        {            
            auto pos = node_item->mapFromScene(mapToScene(event->pos()));
            auto prop = node_item->propAt(pos);
            if(prop)
            {
                QString error;
                JZNodeGemo to(node_item->id(), prop->id());
                if(m_file->canConnect(m_selLine->startTraget(),to,error))
                    gemo = to;
            }
        }
        if (gemo.nodeId != INVALID_ID)
            endLine(gemo);
        else
            cancelLine();
        clearTip();
    }        
    if(m_selArea)
        cancelSelect();
    QGraphicsView::mouseReleaseEvent(event);
}

void JZNodeView::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Delete)
        remove();
    else if(event->key() == Qt::Key_Escape){
        cancelLine();
        cancelSelect();
    }

    QGraphicsView::keyReleaseEvent(event);
}

void JZNodeView::onTimer()
{
}

void JZNodeView::onPropNameUpdate(int id,int propId,const QString &value)
{
    QVariant oldValue = getNode(id)->propName(propId);
    if(oldValue == value)
        return;

    JZNodeViewCommand *cmd = new JZNodeViewCommand(this,JZNodeViewCommand::PropertyNameChange);
    cmd->itemId = id;
    cmd->propId = propId;
    cmd->newValue = value;
    cmd->oldValue = value;
    m_commandStack.push(cmd);
}

void JZNodeView::onPropUpdate(int id,int propId,const QVariant &value)
{
    QVariant oldValue = getNode(id)->propValue(propId);
    if(oldValue == value)
        return;

    JZNodeViewCommand *cmd = new JZNodeViewCommand(this,JZNodeViewCommand::PropertyValueChange);
    cmd->itemId = id;
    cmd->propId = propId;
    cmd->newValue = value;
    cmd->oldValue = value;
    m_commandStack.push(cmd);
}

void JZNodeView::copyItems(QList<QGraphicsItem*> items)
{
    CopyData copydata;
    for(int i = 0; i < items.size(); i++)
    {
        auto item = items[i];
        if(item->type() == Item_node)
        {
            JZNodeGraphItem *node = (JZNodeGraphItem*)item;
            QByteArray data = formatNode(node->node());
            copydata.nodes.append(data);
            copydata.nodesPos.append(item->pos());
        }
        else if(item->type() == Item_line)
        {
            JZNodeLineItem *line = (JZNodeLineItem*)item;
            JZNodeConnect connect = *m_file->getConnect(line->id());
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
