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
        item->setPos(oldPos);
    }
    else if(command == MoveNode)
    {        
        m_view->getNodeItem(itemId)->setPos(oldPos);
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
        item->setPos(newPos);
    }
    else if(command == RemoveNode)
    {
        m_view->removeNode(itemId);
    }
    else if(command == MoveNode)
    {        
        m_view->getNodeItem(itemId)->setPos(newPos);
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
bool JZNodeView::CopyData::isEmpty()
{
    return nodes.isEmpty();
}

//JZNodeView
JZNodeView::JZNodeView(QWidget *widget)
    : QGraphicsView(widget)
{
    m_selLine = nullptr;
    m_propEditor = nullptr;
    m_loadFlag = false;
    m_scene = new JZNodeScene();    
    m_file = nullptr;
    m_isMove = false;

    setScene(m_scene);
    setAcceptDrops(true);

    QTimer *timer = new QTimer();
    connect(timer,&QTimer::timeout,this,&JZNodeView::onTimer);
    //timer->start(2000);

    setWindowFlags(Qt::BypassGraphicsProxyWidget);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &JZNodeView::customContextMenuRequested, this, &JZNodeView::onContextMenu);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    auto cutCopy = new QShortcut(QKeySequence("Ctrl+c"),this);
    auto cutPaste = new QShortcut(QKeySequence("Ctrl+v"),this);
    auto cutRedo = new QShortcut(QKeySequence("Ctrl+y"),this);
    auto cutUndo = new QShortcut(QKeySequence("Ctrl+z"),this);

    connect(cutCopy,&QShortcut::activated,this,&JZNodeView::onCopy);
    connect(cutPaste,&QShortcut::activated,this,&JZNodeView::onPaste);
    connect(cutRedo,&QShortcut::activated,this,&JZNodeView::onRedo);
    connect(cutUndo,&QShortcut::activated,this,&JZNodeView::onUndo);
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
    connect(m_propEditor, &JZNodePropertyEditor::sigPropUpdate, this, &JZNodeView::onPropUpdate);
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

    Q_ASSERT(m_file->getConnectId(id).size() == 0);
    m_scene->removeItem(item);
    delete item;
    m_file->removeNode(id);
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

JZNodeLineItem *JZNodeView::createLine(JZNodeGemo from, JZNodeGemo to)
{
    int id = m_file->addConnect(from, to);
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

            JZNodeViewCommand *cmd = new JZNodeViewCommand(this,JZNodeViewCommand::MoveNode);
            cmd->itemId = node_id;
            cmd->oldPos = m_file->getNodePos(node_id);
            cmd->newPos = item->pos();
            m_commandStack.push(cmd);
        }
    }
    else if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        auto list = m_scene->selectedItems();
        if(list.size() > 0)
        {
            auto node = ((JZNodeGraphItem *)list[0])->node();
            m_propEditor->setNode(node);
        }
        else
        {
            m_propEditor->setNode(nullptr);
        }
    }
    return value;
}

void JZNodeView::initGraph()
{
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

    foreachLine([](JZNodeLineItem *item)
                { item->updateNode(); });
    m_loadFlag = false;
    m_scene->update();
    m_commandStack.clear();
}

void JZNodeView::clear()
{
    m_scene->clear();
    m_file->clear();
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

void JZNodeView::copy()
{

}

void JZNodeView::paste()
{

}

int JZNodeView::paramId(int nodeId,int propId)
{
    return 0;//return m_file->paramId(nodeId,propId);
}

void JZNodeView::updateNodeLayout()
{
/*
    int y = 0;
    auto graph_list = compilper.graphs();
    for(int i = 0; i < graph_list.size(); i++)
    {
        auto graph = graph_list[i];
        auto list = graph->topolist;

        int time = 0;
        int x_offset = 0;
        while(list.size() > 0)
        {
            QVector<int> nodes;
            for(int i = list.size() - 1; i >=0; i--){
                if(list[i]->paramIn.size() == 0)
                {
                    nodes.push_back(list[i]->node->id());
                    list.removeAt(i);
                }
            }
            std::sort(nodes.begin(),nodes.end());
            if(nodes.size() == 0)
                break;

            int max_w = 0;
            for(int i = 0; i < nodes.size(); i++){
                auto item = getNodeItem(nodes[i]);
                item->setPos(x_offset,y);
                y += item->boundingRect().height() + 20;
                max_w = qMax(max_w,(int)item->boundingRect().width());

                GraphNode *node = graph->m_nodes[nodes[i]].get();
                auto it = node->paramOut.begin();
                while(it != node->paramOut.end())
                {
                    JZNodeGemo out_src(node->node->id(),it.key());

                    auto &out_list = it.value();
                    for(int j = 0; j < out_list.size(); j++)
                    {
                        GraphNode *next = graph->m_nodes[out_list[j].nodeId].get();
                        auto it_in = next->paramIn.begin();
                        while(it_in != next->paramIn.end())
                        {
                            auto &in_list = it_in.value();
                            for(int in_idx = 0; in_idx < in_list.size(); in_idx++)
                            {
                            }
                        }
                    }
                    it++;
                }
            }
            x_offset += max_w + 20;
            time++;
        }
    }
    m_scene->setSceneRect(m_scene->itemsBoundingRect());
    fitInView(rect());
*/        
}

void JZNodeView::removeItem(QGraphicsItem *item)
{
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

bool JZNodeView::canConnect(JZNodeGemo from,JZNodeGemo to)
{
    JZNode *node_from = getNode(from.nodeId);
    JZNode *node_to = getNode(to.nodeId);
    if(node_from == node_to)
        return true;    

    auto lines = m_file->getConnectId(to.nodeId, to.propId, Prop_in);
    if(lines.size() != 0)
        return false;

    QList<int> form_type = node_from->propType(from.nodeId);
    QList<int> in_type = node_to->propType(to.propId);
    bool ok = JZNodeType::canConvert(form_type,in_type);
    if(!ok)
        return false;

    return true;
}

void JZNodeView::onContextMenu(const QPoint &pos)
{
    QMenu menu(this);

    auto item = itemAt(pos);
    QAction *actAdd = menu.addAction("添加节点");
    QAction *actCpy = nullptr;
    QAction *actDel = nullptr;
    if (item)
    {
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
        scale*=1.1;//每次放大10%
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
        m_isMove = true;
    }
    QGraphicsView::mousePressEvent(event);
}

void JZNodeView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_selLine)
    {
        JZNodeGemo gemo;
        auto item = this->itemAt(event->pos());
        if (item && item->type() == Item_node)
        {
            JZNodeGraphItem *node_item = (JZNodeGraphItem *)item;     
            auto pos = node_item->mapFromScene(mapToScene(event->pos()));
            auto prop = node_item->propAt(pos);
            if(prop)
            {
                JZNodeGemo to(node_item->id(), prop->id());
                if(canConnect(m_selLine->startTraget(),to))
                    gemo = to;            
            }
        }
        if (gemo.nodeId != INVALID_ID)
            endLine(gemo);
        else
            cancelLine();
    }
    QGraphicsView::mouseReleaseEvent(event);

    if(m_isMove){
        m_isMove = false;
    }
}

void JZNodeView::onTimer()
{
}

void JZNodeView::onPropUpdate(int id)
{
    getNodeItem(id)->updateNode();
}

void JZNodeView::copyItem(QList<QGraphicsItem*> items)
{
    m_copyData = CopyData();
    for(int i = 0; i < items.size(); i++)
    {
        auto item = items[i];
        if(item->type() == Item_node)
        {
            JZNodeGraphItem *node = (JZNodeGraphItem*)item;
            QByteArray data = formatNode(node->node());
            m_copyData.nodes.append(data);
            m_copyData.nodesPos.append(item->pos());
        }
        else if(item->type() == Item_line)
        {
            JZNodeLineItem *line = (JZNodeLineItem*)item;
            JZNodeConnect connect = *m_file->getConnect(line->id());
            m_copyData.lines << connect;
        }
    }
}

void JZNodeView::onCopy()
{
    auto items = m_scene->selectedItems();
    copyItem(items);
}

void JZNodeView::onPaste()
{
    if(m_copyData.nodes.size() == 0)
        return;

    m_commandStack.beginMacro("paste");
    QMap<int,int> nodeIdMap;
    for(int i = 0; i < m_copyData.nodesPos.size(); i++)
    {
        JZNode *node = parseNode(m_copyData.nodes[i]);
        int old_id = node->id();
        node->setId(-1);

        JZNodeViewCommand *cmd = new JZNodeViewCommand(this,JZNodeViewCommand::CreateNode);
        cmd->itemId << -1;
        cmd->newValue = formatNode(node);
        cmd->newPos = m_copyData.nodesPos[i];
        m_commandStack.push(cmd);

        nodeIdMap[old_id] = cmd->itemId;
    }

    //保留原有节点间线段关系
    for(int i = 0; i < m_copyData.lines.size(); i++)
    {
        JZNodeConnect line = m_copyData.lines[i];
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

void JZNodeView::onRedo()
{
    redo();
}

void JZNodeView::onUndo()
{
    undo();
}
