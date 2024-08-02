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
#include "JZNodeView.h"
#include "JZNodeExprEditDialog.h"
#include "JZNodePanel.h"
#include "JZNodeViewPanel.h"
#include "3rd/jzprofiler/JZTx.h"

enum ViewCommand {
    CreateNode,
    RemoveNode,
    MoveNode,
    NodePropertyChange,    
    CreateLine,
    RemoveLine,
    CreateGroup,
    RemoveGroup,
    SetGroup,
};

JZNodeConnect parseLine(const QByteArray &buffer)
{
    JZNodeConnect line;
    QDataStream s(buffer);
    s >> line;
    return line;
}

QByteArray formatLine(const JZNodeConnect &line)
{
    QByteArray buffer;
    QDataStream s(&buffer, QIODevice::WriteOnly);
    s << line;
    return buffer;
}

JZNodeGroup parseGroup(const QByteArray &buffer)
{
    JZNodeGroup group;
    QDataStream s(buffer);
    s >> group;
    return group;
}

QByteArray formatGroup(const JZNodeGroup &group)
{
    QByteArray buffer;
    QDataStream s(&buffer, QIODevice::WriteOnly);
    s << group;
    return buffer;
}

//JZNodeViewCommand
JZNodeViewCommand::JZNodeViewCommand(JZNodeView *view,int type)
{
    itemId = -1;
    pinId = -1;
    command = type;
    m_view = view;
}

int JZNodeViewCommand::id() const
{
    if (command == NodePropertyChange && pinId != -1)
    {
        return 10000 + JZNodeCompiler::paramId(itemId, pinId);
    }

    return -1;
}

bool JZNodeViewCommand::mergeWith(const QUndoCommand *cmd)
{
    auto *other = dynamic_cast<const JZNodeViewCommand*>(cmd);
    Q_ASSERT(other);

    this->newValue = other->newValue;
    return true;
}

void JZNodeViewCommand::undo()
{
    if(command == CreateNode)
    {        
        m_view->removeNode(itemId);
    }
    else if(command == RemoveNode)
    {
        auto node = JZNodeFactory::instance()->loadNode(oldValue.toByteArray());
        node->setId(itemId);
        auto item = m_view->insertNode(node);
        m_view->setNodePos(itemId,oldPos);
    }    
    else if (command == NodePropertyChange)
    {
        m_view->setNodeData(itemId, oldValue.toByteArray());
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
    else if (command == CreateGroup)
    {
        m_view->removeGroup(itemId);        
    }
    else if (command == RemoveGroup)
    {
        auto group = parseGroup(oldValue.toByteArray());
        m_view->insertGroup(group);
    }    
}

void JZNodeViewCommand::redo()
{
    if(command == CreateNode)
    {        
        auto node = JZNodeFactory::instance()->loadNode(newValue.toByteArray());
        JZNodeGraphItem *item = nullptr;
        if(itemId == -1)
        {
            item = m_view->createNode(node);
            itemId = item->id();
        }
        else
        {
            node->setId(itemId);
            item = m_view->insertNode(node);
        }
        m_view->setNodePos(itemId,newPos);
    }
    else if(command == RemoveNode)
    {
        m_view->removeNode(itemId);
    }
    else if (command == NodePropertyChange)
    {
        if (newValue.isNull())
        {
            newValue = m_view->getNodeData(itemId);
            m_view->updateNode(itemId);
        }
        else
        {
            m_view->setNodeData(itemId, newValue.toByteArray());
        }
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
    else if (command == CreateGroup)
    {
        auto group = parseGroup(newValue.toByteArray());
        if (itemId == -1)
        {
            auto item = m_view->createGroup(group);
            itemId = item->id();
        }
        else
        {
            group.id = itemId;
            m_view->insertGroup(group);
        }
    }
    else if (command == RemoveGroup)
    {
        m_view->removeGroup(itemId);
    }
    else if (command == SetGroup)
    {
        if (newValue.isNull())
        {   
            newValue = m_view->getGroupData(itemId);
            m_view->updateGroup(itemId);
        }
        else
        {
            m_view->setGroupData(itemId,newValue.toByteArray());
        }
    }    
}

//JZNodeMoveCommand
JZNodeMoveCommand::JZNodeMoveCommand(JZNodeView *view, int type)
{
    m_view = view;
    command = type;
}

int JZNodeMoveCommand::id() const
{
    return command;
}

bool JZNodeMoveCommand::mergeWith(const QUndoCommand *cmd)
{    
    auto *other = dynamic_cast<const JZNodeMoveCommand*>(cmd);
    Q_ASSERT(other);
    for (int i = 0; i < other->nodeList.size(); i++)
    {
        int index = -1;
        for (int j = 0; j < nodeList.size(); j++)
        {
            if (nodeList[j].itemId == other->nodeList[i].itemId)
            {
                index = j;
                break;
            }
        }

        if (index != -1)
            nodeList[index].newPos = other->nodeList[i].newPos;
        else
            nodeList.push_back(other->nodeList[i]);
    }

    return true;
}

void JZNodeMoveCommand::redo()
{
    for (int i = 0; i < nodeList.size(); i++)
    {
        auto &info = nodeList[i];
        m_view->setNodePos(info.itemId, info.newPos);
    }
}

void JZNodeMoveCommand::undo()
{
    for (int i = 0; i < nodeList.size(); i++)
    {
        auto &info = nodeList[i];
        m_view->setNodePos(info.itemId, info.oldPos);
    }
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
    m_panel = nullptr;
    m_selLine = nullptr;        
    m_propEditor = nullptr;
    m_loadFlag = false;    
    m_file = nullptr;    
    m_recordMove = true;    
    m_runningMode = Process_none;
    m_runNode = -1;        
    m_groupIsMoving = false;
    m_autoRunning = false;
    m_isUpdateFlowPanel = false;
    
    connect(&m_commandStack,&QUndoStack::cleanChanged, this, &JZNodeView::onCleanChanged);
    connect(&m_commandStack,&QUndoStack::canRedoChanged,this,&JZNodeView::redoAvailable);
    connect(&m_commandStack,&QUndoStack::canUndoChanged,this,&JZNodeView::undoAvailable);
    connect(&m_commandStack,&QUndoStack::indexChanged, this,&JZNodeView::onUndoStackChanged);

    m_map = new JZNodeViewMap(this);
    m_map->setFixedSize(160, 120);
    m_map->setView(this);
    connect(m_map, &JZNodeViewMap::mapSceneChanged, this, &JZNodeView::onMapSceneChanged);
    connect(m_map, &JZNodeViewMap::mapSceneScaled, this, &JZNodeView::onMapSceneScaled);

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->addStretch();
    hbox->addWidget(m_map);
    hbox->setAlignment(Qt::AlignTop);
    this->setLayout(hbox);

    m_scene = new JZNodeScene();    
    setScene(m_scene);

    setAcceptDrops(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); 

    m_nodeTimer = new QTimer(this);
    connect(m_nodeTimer, &QTimer::timeout, this, &JZNodeView::onNodeTimer);
    m_nodeTimer->setSingleShot(true);

    m_mouseMoveTimer = new QTimer(this);
    connect(m_mouseMoveTimer, &QTimer::timeout, this, &JZNodeView::onMouseMoveTimer);

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
    connect(m_propEditor, &JZNodePropertyEditor::sigNodePropChanged, this, &JZNodeView::onPropChanged);    
}

void JZNodeView::setRunEditor(JZNodeAutoRunWidget *runEditor)
{
    m_runEditor = runEditor;
    connect(m_runEditor, &JZNodeAutoRunWidget::sigDependChanged, this, &JZNodeView::onDependChanged);
}

JZScriptItem *JZNodeView::file()
{
    return m_file;
}

void JZNodeView::setPanel(JZNodePanel *panel)
{
    m_panel = panel;
}

void JZNodeView::setFlowPanel(JZNodeViewPanel *panel)
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

void JZNodeView::setFile(JZScriptItem *file)
{
    m_file = file;    
    connect(m_file->project(), &JZProject::sigScriptNodeChanged, this, &JZNodeView::onScriptNodeChanged);

    initGraph();
    autoCompiler();    
}

bool JZNodeView::isModified()
{
    return !m_commandStack.isClean();
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

JZNodeGraphItem *JZNodeView::createNode(JZNode *node)
{        
    int id = m_file->addNode(node);
    return createNodeItem(id);
}

JZNodeGraphItem *JZNodeView::insertNode(JZNode *node)
{
    m_file->insertNode(node);
    return createNodeItem(node->id());
}

void JZNodeView::removeNode(int id)
{
    auto item = getNodeItem(id);
    if (!item)
        return;

    if(m_propEditor->node() == item->node())
        setSelectNode(-1);
            
    Q_ASSERT(m_file->getConnectPin(id).size() == 0);
    item->clear();
    m_scene->removeItem(item);
    delete item;
    m_file->removeNode(id);
    m_map->updateMap();
}

QByteArray JZNodeView::getNodeData(int id)
{
    auto node = getNode(id);
    return node->toBuffer();
}

void JZNodeView::setNodeData(int id,const QByteArray &buffer)
{    
    auto node = getNode(id);
    int old_group = node->group();

    node->fromBuffer(buffer);
    if (old_group != -1 && old_group != node->group())
        getGroupItem(old_group)->updateNode();

    updateNode(id);
}

void JZNodeView::setNodePos(int node_id, QPointF pos)
{        
    m_recordMove = false;    
    auto node = getNode(node_id);
    getNodeItem(node_id)->setPos(pos);
    m_file->setNodePos(node_id, pos);

    auto lineId = m_file->getConnectPin(node_id);
    for (int i = 0; i < lineId.size(); i++)
        getLineItem(lineId[i])->updateNode();
    
    if (node->group() != -1 && !m_groupIsMoving)
        getGroupItem(node->group())->updateNode();

    m_map->updateMap();
    m_recordMove = true;
}

bool JZNodeView::isPropEditable(int id,int prop_id)
{
    if(!getNode(id)->pin(prop_id)->isEditValue())
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
    m_map->updateMap();
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
    item->setPinValue(prop_id, value);
    m_map->updateMap();
}

QString JZNodeView::getNodePropValue(int nodeId, int prop_id)
{
    auto item = getNodeItem(nodeId);
    return item->pinValue(prop_id);    
}

void JZNodeView::setNodeTimer(int ms,int nodeId,int event)
{
    m_nodeTimer->stop();
    m_nodeTimer->start(ms);

    m_nodeTimeInfo.nodeId = nodeId;
    m_nodeTimeInfo.event = event;
}

void JZNodeView::onNodeTimer()
{        
    auto node = getNodeItem(m_nodeTimeInfo.nodeId);
    if (node)
        node->onTimerEvent(m_nodeTimeInfo.event);
}

void JZNodeView::onMouseMoveTimer()
{
    QPoint cursor_pt = mapFromGlobal(QCursor::pos());
    if(rect().contains(cursor_pt))
    {
        m_mouseMoveTimer->stop();
        return;
    }

    QPoint pt = cursor_pt - rect().center();
    double scale = 20.0 / pt.manhattanLength();
    int x = qRound(pt.x() * scale);
    int y = qRound(pt.y() * scale);
    sceneTranslate(x,y);

    if(m_selLine)
        m_selLine->setEndPoint(mapToScene(cursor_pt));
}

void JZNodeView::updateNode(int id)
{
    getNodeItem(id)->updateNode();
    auto lineId = m_file->getConnectPin(id);
    for (int i = 0; i < lineId.size(); i++)
        getLineItem(lineId[i])->updateNode();

    int group = getNode(id)->group();
    if (group != -1)
        getGroupItem(group)->updateNode();

    if(getNode(id) == m_propEditor->node())    
        m_propEditor->updateNode();
}

void JZNodeView::updatePropEditable(const JZNodeGemo &gemo)
{
    getNodeItem(gemo.nodeId)->updateNode();

    auto node = m_propEditor->node();    
    if(node && gemo.nodeId == node->id())
        m_propEditor->setPropEditable(gemo.pinId,isPropEditable(gemo.nodeId,gemo.pinId));
}

JZNodeLineItem *JZNodeView::createLine(JZNodeGemo from, JZNodeGemo to)
{
    int id = m_file->addConnect(from, to);    
    updatePropEditable(to);    
    udpateFlowPanel();
    return createLineItem(id);
}

JZNodeLineItem *JZNodeView::insertLine(const JZNodeConnect &connect)
{
    m_file->insertConnect(connect);
    updatePropEditable(connect.to);
    udpateFlowPanel();
    return createLineItem(connect.id);
}

void JZNodeView::removeLine(int id)
{
    auto item = getLineItem(id);
    if (!item)
        return;

    m_file->removeConnect(id);
    JZNodeGemo to = item->endTraget();
    if(m_file->getPin(to))       //节点内部事件处理先删除pin, 外面再删除线条，会存在to 不存在的情况
        updatePropEditable(to);
    udpateFlowPanel();
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
    if (m_selLine || m_runningMode != Process_none)
        return;

    JZNodeGraphItem *node_from = getNodeItem(from.nodeId);
    auto pt = node_from->mapToScene(node_from->pinRect(from.pinId).center());

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

    m_commandStack.beginMacro("create line");

    JZNodeViewCommand *cmd = new JZNodeViewCommand(this, ViewCommand::CreateLine);
    cmd->itemId = -1;
    cmd->newValue = formatLine(line);
    m_commandStack.push(cmd);

    auto node = getNode(line.to.nodeId);
    if(node->pin(line.to.pinId)->isParam())
    {
        auto old = getNodeData(line.to.nodeId);
        node->setPinValue(line.to.pinId,QString());
        addPinValueChangedCommand(line.to.nodeId,line.to.pinId,old);
    }
    m_commandStack.endMacro();

    m_selLine->ungrabMouse();
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

JZNodeGroupItem *JZNodeView::createGroupItem(int id)
{
    JZNodeGroupItem *item = new JZNodeGroupItem(id);
    m_scene->addItem(item);
    item->updateNode();
    return item;
}

JZNodeGroupItem *JZNodeView::createGroup(const JZNodeGroup &group)
{
    int id = m_file->addGroup(group);
    return createGroupItem(id);
}

JZNodeGroupItem *JZNodeView::insertGroup(const JZNodeGroup &group)
{
    m_file->insertGroup(group);
    return createGroupItem(group.id);
}
void JZNodeView::removeGroup(int id)
{
    auto items = m_scene->items();
    for (int i = 0; i < items.size(); i++)
    {
        if (items[i]->type() == Item_group && ((JZNodeGroupItem *)items[i])->id() == id)
        {
            m_scene->removeItem(items[i]);
            break;
        }
    }
    m_file->removeGroup(id);
}

JZNodeGroupItem *JZNodeView::getGroupItem(int id)
{
    auto items = m_scene->items();
    for (int i = 0; i < items.size(); i++)
    {
        if (items[i]->type() == Item_group && ((JZNodeGroupItem *)items[i])->id() == id)
            return (JZNodeGroupItem *)items[i];
    }
    return nullptr;
}

QByteArray JZNodeView::getGroupData(int id)
{
    auto group = m_file->getGroup(id);
    return formatGroup(*group);
}

void JZNodeView::setGroupData(int id, QByteArray buffer)
{
    auto group = m_file->getGroup(id);
    *group = parseGroup(buffer);
}

void JZNodeView::updateGroup(int id)
{
    getGroupItem(id)->updateNode();
}

void JZNodeView::showTip(QPointF pt,QString text)
{            
    auto tip = this->mapFromScene(pt);
    if (QToolTip::isVisible() && (tip - m_tipPoint).manhattanLength() < 15)
        return;

    m_tipPoint = tip;
    tip = mapToGlobal(tip);    
    QToolTip::showText(tip , text, nullptr, QRect(), 15 * 1000);    
}

void JZNodeView::clearTip()
{
    QToolTip::hideText();
    m_tipPoint = QPoint();
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

JZNodeGemo JZNodeView::pinAt(QPoint pos)
{
    auto node_item = nodeItemAt(pos);
    if(!node_item)
        return JZNodeGemo();

    auto scene_pos = mapToScene(pos);
    auto item_pos = node_item->mapFromScene(scene_pos);            
    int pin_id = node_item->pinAt(item_pos);
    if(pin_id == -1)
        return JZNodeGemo();
    
    return JZNodeGemo(node_item->id(),pin_id);
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

    if (change == QGraphicsItem::ItemPositionChange)
    {
        if (item->type() == Item_node)
        {
            if (m_recordMove)
            {
                auto node = ((JZNodeGraphItem *)item)->node();
                addMoveNodeCommand(node->id(), value.toPointF());
            }
        }
        else if (item->type() == Item_group)
        {            
            JZNodeGroupItem *group_item = dynamic_cast<JZNodeGroupItem*>(item);
            auto group_id = group_item->id();
            QPointF offset = value.toPointF() - group_item->pos();

            m_groupIsMoving = true;
            auto node_list = m_file->groupNodeList(group_id);
            for (int i = 0; i < node_list.size(); i++)
            {
                int node_id = node_list[i];
                QPointF old_pos = getNodeItem(node_id)->pos();
                addMoveNodeCommand(node_id, old_pos + offset);
            }            
            m_groupIsMoving = false;
        }
    }
    else if(change == QGraphicsItem::ItemSelectedChange)
    {
        QPoint pt = mapFromGlobal(QCursor::pos());
        if(!pinAt(pt).isNull())
            return false;

        return value;
    }
    else if(change == QGraphicsItem::ItemSelectedHasChanged)
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
    auto groups = m_file->groupList();
    for (int i = 0; i < groups.size(); i++)
        createGroupItem(groups[i].id);

    m_loadFlag = false;
    m_scene->update();
    m_commandStack.clear();

    sceneTranslate(-20,-20);
    m_map->updateMap();
    udpateFlowPanel();
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
        JZNode *node = JZNodeFactory::instance()->loadNode(copyData.nodes[i]);
        int old_id = node->id();
        node->setId(-1);
        node->setGroup(-1);

        JZNodeViewCommand *cmd = new JZNodeViewCommand(this, ViewCommand::CreateNode);
        cmd->itemId = -1;
        cmd->newValue = JZNodeFactory::instance()->saveNode(node);
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

        JZNodeViewCommand *cmd = new JZNodeViewCommand(this, ViewCommand::CreateLine);
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

void JZNodeView::fitNodeView()
{        
    setSceneRect(QRectF());

    QRectF scene_rc = scene()->itemsBoundingRect();        
    if (scene_rc.width() > width() || scene_rc.height() > height())
    {
        //两次处理，保留边距
        this->fitInView(scene_rc, Qt::KeepAspectRatio);

        //缩小scene
        QPointF pt = mapToScene(QPoint(20, 20));
        scene_rc.adjust(-pt.x(), -pt.y(), pt.x(), pt.y());
        setSceneRect(scene_rc);  //要先设置scene大小， fitInView 是在scene大小中处理
        this->fitInView(scene_rc, Qt::KeepAspectRatio);        
    }
    else
    {
        resetTransform();
        sceneTranslate(-20, -20);
    }
}

void JZNodeView::ensureNodeVisible(int id)
{
    auto item = getNodeItem(id);

    QRectF view_rc = mapToScene(rect()).boundingRect();    
    auto item_rc = item->mapRectToScene(item->boundingRect());
    if (!view_rc.contains(item_rc))
    {        
        QPointF pt = item_rc.center();
        sceneCenter(pt);
    }
}

void JZNodeView::selectNode(int id)
{
    ensureNodeVisible(id);
    m_scene->clearSelection();
    getNodeItem(id)->setSelected(true);
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
                pt.file = filepath;
                pt.nodeId = node_item->id();
                pt.type = BreakPoint::nodeEnter;
                project->addBreakPoint(pt);
                node_item->update();
            }
            break;
        }
    }
}

void JZNodeView::setAutoRunning(bool flag)
{
    m_autoRunning = flag;
    autoRunning();
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
    foreachNode([flag](JZNodeGraphItem *node) {
        node->setFlag(QGraphicsItem::ItemIsMovable, flag);
    });    

    if (m_runningMode == Process_none)
    {
        m_runNode = -1;    
    }

    foreachNode([status](JZNodeGraphItem *node) {
        node->setRunningMode(status);
    });
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
        preItem->updateRuntimeStatus();
        preItem->update();
    }
    m_runNode = nodeId;
    if (nodeId == -1)
        return;
        
    auto item = getNodeItem(m_runNode);
    item->updateRuntimeStatus();
    item->update();    
    selectNode(nodeId);
}

void JZNodeView::resetPropValue()
{
    foreachNode([](JZNodeGraphItem *node){
        node->resetPropValue();
    });
}

void JZNodeView::setRuntimeValue(int node_id,int pin_id,const JZNodeDebugParamValue &value)
{
    auto item = getNodeItem(node_id);
    if(!item)
        return;

    item->setPinRuntimeValue(pin_id,value);
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
        auto lines = m_file->getConnectPin(item_id);
        for (int i = 0; i < lines.size(); i++)
        {
            auto line = m_file->getConnect(lines[i]);

            JZNodeViewCommand *cmd = new JZNodeViewCommand(this, ViewCommand::RemoveLine);
            cmd->itemId = line->id;
            cmd->oldValue = formatLine(*line);
            m_commandStack.push(cmd);
        }

        auto node = m_file->getNode(item_id);
        int group = node->group();

        JZNodeViewCommand *cmd = new JZNodeViewCommand(this, ViewCommand::RemoveNode);
        cmd->itemId = node->id(); 
        cmd->oldValue = JZNodeFactory::instance()->saveNode(node);
        cmd->oldPos = item->pos();
        m_commandStack.push(cmd);
        
        if (group != -1 && m_file->groupNodeList(group).size() == 0)
            addRemoveGroupCommand(group);

        m_commandStack.endMacro();
    }
    else if (item->type() == Item_line)
    {
        auto line = m_file->getConnect(item_id);
        JZNodeViewCommand *cmd = new JZNodeViewCommand(this, ViewCommand::RemoveLine);
        cmd->itemId = line->id;
        cmd->oldValue = formatLine(*line);
        m_commandStack.push(cmd);
    }
    else if (item->type() == Item_group)
    {
        m_commandStack.beginMacro("remove");

        auto list = m_file->groupNodeList(item_id);
        for (int i = 0; i < list.size(); i++)
        {
            QByteArray oldValue = getNodeData(list[i]);
            getNode(list[i])->setGroup(-1);
            addPropChangedCommand(list[i], oldValue);
        }

        addRemoveGroupCommand(item_id);
        m_commandStack.endMacro();
    }
}

QStringList JZNodeView::matchParmas(JZNodeObjectDefine *meta, int match_type, QString pre)
{
    QStringList list;
    auto params = meta->paramList(true);
    for (int i = 0; i < params.size(); i++)
    {
        auto p = meta->param(params[i]);
        if (JZNodeType::canConvert(p->dataType(),match_type))
            list << pre + p->name;
        if (JZNodeType::isObject(p->dataType()))
        {
            auto sub_meta = JZNodeObjectManager::instance()->meta(p->dataType());
            list << matchParmas(sub_meta, match_type, pre + p->name + ".");
        }
    }
    return list;
}

QList<JZNodeGraphItem*> JZNodeView::selectNodeItems()
{
    QList<JZNodeGraphItem*> result;
    auto list = m_scene->selectedItems();
    for (int i = 0; i < list.size(); i++)
    {
        if (list[i]->type() == Item_node)
            result << dynamic_cast<JZNodeGraphItem *>(list[i]);
    }
    return result;
}

void JZNodeView::onContextMenu(const QPoint &pos)
{
    if (m_runningMode != Process_none)
        return;

    QMenu menu(this);    

    auto item = itemAt(pos);
    QList<QAction*> actAddList;
    QList<QTreeWidgetItem*> actAddItemList;
    QAction *actCpy = nullptr;
    QAction *actDel = nullptr;
    QAction *actPaste = nullptr;
    QAction *actGoto = nullptr;
    QAction *actEditGroup = nullptr;
    QAction *actSetExpr = nullptr;
    int pin_id = -1;
    QList<QAction*> pin_actions;    
    QList<QAction*> node_actions;
    if (!item)
    {
        QMenu* addMenu = menu.addMenu("添加节点");
        QMenu* menu_op = addMenu->addMenu("操作符");
        auto item_op = m_panel->itemOp();
        for (int i = 0; i < item_op->childCount(); i++)
        {
            auto child = item_op->child(i);
            actAddItemList << child;
            actAddList << menu_op->addAction(child->text(0));
        }

        QMenu* menu_stat = addMenu->addMenu("流程");
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
            auto node = ((JZNodeGraphItem*)item)->node();

            auto scene_pos = mapToScene(pos);
            auto item_pos = node_item->mapFromScene(scene_pos);            
            pin_id = node_item->pinAtInName(item_pos);

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
                actions_list = node_item->node()->actionList();
                for (int i = 0; i < actions_list.size(); i++)
                    node_actions << menu.addAction(actions_list[i]);

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
                else if (node->type() == Node_expr)
                {
                    actSetExpr = menu.addAction("设置表达式");
                }

                actCpy = menu.addAction("复制节点");
                actDel = menu.addAction("删除节点");
            }
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
        if (node->pinActionTriggered(pin_id, index))
            onScriptNodeChanged(m_file, node->id(), old);        
    }
    else if(node_actions.contains(ret))
    {
        int index = node_actions.indexOf(ret);
        auto node = dynamic_cast<JZNodeGraphItem*>(item)->node(); 
        auto old = getNodeData(node->id());
        if (node->actionTriggered(index))
            onScriptNodeChanged(m_file, node->id(), old);  
    }
    else if(ret == actGoto)
    {
        QString filePath = actGoto->property("filePath").toString();
        emit sigFunctionOpen(filePath);
    }
    else if(ret == actSetExpr)
    {
        auto node_item = dynamic_cast<JZNodeGraphItem*>(item); 
        auto node_expr = dynamic_cast<JZNodeExpression*>(node_item->node());
        QString expr = getExpr(node_expr->expr());
        if(expr.isEmpty() || expr == node_expr->expr())
            return;

        auto old = getNodeData(node_expr->id());      
        QString error;
        node_expr->setExpr(expr,error);
        onScriptNodeChanged(m_file,node_expr->id(),old);
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
            addPropChangedCommand(node_list[i]->id(), oldValue);
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
            addPropChangedCommand(node_list[i]->id(), oldValue);
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
                addPropChangedCommand(node_list[i]->id(), oldValue);
            }
        }
        m_commandStack.endMacro();
    }
}

void JZNodeView::addCreateNodeCommand(const QByteArray &buffer,QPointF pt)
{
    JZNodeViewCommand *cmd = new JZNodeViewCommand(this, ViewCommand::CreateNode);
    cmd->itemId = -1;
    cmd->newValue = buffer;
    cmd->newPos = pt;
    m_commandStack.push(cmd);
}

void JZNodeView::addRemoveLineCommand(int line_id)
{
    auto line = m_file->getConnect(line_id);

    JZNodeViewCommand *cmd = new JZNodeViewCommand(this, ViewCommand::RemoveLine);
    cmd->itemId = line->id;
    cmd->oldValue = formatLine(*line);
    m_commandStack.push(cmd);
}

void JZNodeView::addPropChangedCommand(int id,const QByteArray &oldValue)
{
    JZNodeViewCommand *cmd = new JZNodeViewCommand(this, ViewCommand::NodePropertyChange);
    cmd->itemId = id;
    cmd->oldValue = oldValue;
    m_commandStack.push(cmd);
}

void JZNodeView::addPinValueChangedCommand(int id, int pin_id, const QByteArray &oldValue)
{
    JZNodeViewCommand *cmd = new JZNodeViewCommand(this, ViewCommand::NodePropertyChange);
    cmd->itemId = id;
    cmd->pinId = id;
    cmd->oldValue = oldValue;
    m_commandStack.push(cmd);
}

void JZNodeView::addMoveNodeCommand(int id, QPointF pt)
{
    JZNodeMoveCommand *cmd = new JZNodeMoveCommand(this, ViewCommand::MoveNode);
    JZNodeMoveCommand::NodePosInfo info;
    info.itemId = id;
    info.oldPos = getNodeItem(id)->pos();
    info.newPos = pt;
    cmd->nodeList.push_back(info);
    m_commandStack.push(cmd);
}

int JZNodeView::addCreateGroupCommand(const JZNodeGroup &group)
{
    int id = m_file->nextId();

    JZNodeViewCommand *cmd = new JZNodeViewCommand(this, ViewCommand::CreateGroup);
    cmd->newValue = formatGroup(group);
    m_commandStack.push(cmd);    
    return id;
}

void JZNodeView::addRemoveGroupCommand(int id)
{
    auto group = m_file->getGroup(id);

    JZNodeViewCommand *cmd = new JZNodeViewCommand(this, ViewCommand::RemoveGroup);
    cmd->itemId = id;
    cmd->oldValue = formatGroup(*group);
    m_commandStack.push(cmd);
}

void JZNodeView::addSetGroupCommand(int id, const JZNodeGroup &new_group)
{
    auto group = m_file->getGroup(id);    

    JZNodeViewCommand *cmd = new JZNodeViewCommand(this, ViewCommand::SetGroup);
    cmd->itemId = id;
    cmd->oldValue = formatGroup(*group);
    *group = new_group;
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
    if (m_runningMode != Process_none)
        return;

    auto factory = JZNodeFactory::instance();
    if(event->mimeData()->hasFormat("node_data"))
    {
        QByteArray node_data = event->mimeData()->data("node_data");
        QDataStream s(node_data);

        int node_type;
        s >> node_type;
        if(node_type == Node_expr)
        {
            QString expr = getExpr();
            if(expr.isEmpty())
                return;

            QString error;
            JZNodeExpression node_expr;
            node_expr.setExpr(expr,error);

            node_data = factory->saveNode(&node_expr);
        }
        
        addCreateNodeCommand(node_data,mapToScene(event->pos()));
        event->accept();
    }
    else if(event->mimeData()->hasFormat("node_param"))
    {
        QString param_name = QString::fromUtf8(event->mimeData()->data("node_param"));
        JZNodeGraphItem *node_item = nodeItemAt(event->pos());
        if(!node_item)
        {
            QMenu menu;
            auto actGet = menu.addAction("Get");
            auto actSet = menu.addAction("Set");

            auto def = JZNodeCompiler::getVariableInfo(m_file,param_name);
            if(def->dataType() >= Type_class || def->dataType() == Type_string)
            {
                auto meta = JZNodeObjectManager::instance()->meta(def->type);
                if(meta)
                {
                    QMenu *menuCall = nullptr;
                    auto func_list = meta->functionList();
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
            if(act)
            {
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
                    function.setFunction(act->data().toString());
                    function.setVariable(param_name);
                    addCreateNodeCommand(factory->saveNode(&function), mapToScene(event->pos()));
                }
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

void JZNodeView::showEvent(QShowEvent *event)
{
    QGraphicsView::showEvent(event);
}

void JZNodeView::resizeEvent(QResizeEvent *event)
{
    m_map->updateMap();
    QGraphicsView::resizeEvent(event);
}

void JZNodeView::sceneScale(QPoint center,bool up)
{
    double scale = this->transform().m11();
    if (up) //鼠标滚轮向前滚动
    {
        if (scale >= 4)
            return;

        scale *= 1.1;//每次放大10%
        scale = qMin(scale, 4.0);
    }
    else
    {
        scale *= 0.9;//每次缩小10%
    }

    auto targetViewportPos = center;
    auto targetScenePos = mapToScene(center);

    resetTransform();
    this->scale(scale, scale);
    sceneCenter(targetScenePos);

    QPointF deltaViewportPos = targetViewportPos - QPointF(viewport()->width() / 2.0, viewport()->height() / 2.0);
    QPointF viewportCenter = mapFromScene(targetScenePos) - deltaViewportPos;
    sceneCenter(mapToScene(viewportCenter.toPoint()));
}

void JZNodeView::sceneTranslate(int x,int y)
{
    QPoint center_pt = rect().center();
    center_pt += QPoint(x,y);   
    sceneCenter(mapToScene(center_pt));
}

void JZNodeView::sceneCenter(QPointF pt)
{
    QRectF view_rc = mapToScene(rect()).boundingRect();
    int w = view_rc.width();
    int h = view_rc.height();
    QRectF rc(pt.x() - w/2,pt.y() - h/2,w,h);
    setSceneRect(rc);
}

void JZNodeView::wheelEvent(QWheelEvent *event)
{
    if(scene()->focusItem())
    {
        QGraphicsView::wheelEvent(event);
        return;
    }

    sceneScale(event->pos(),event->angleDelta().y() > 0);
    event->accept();
}

void JZNodeView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
    viewport()->setCursor(QCursor());
    if (event->button() == Qt::LeftButton)
    {
        m_downPoint = event->pos();
        m_downCenter = mapToScene(rect().center());
    }
/*
    auto item = nodeItemAt(event->pos());
    if (item && !item->isSelected())
    {
        if((event->modifiers() & Qt::ShiftModifier) == 0)
            m_scene->clearSelection();
        item->setSelected(true);
    }
*/        
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
            auto pin_id = node_item->pinAt(item_pos);
            if(pin_id >= 0)
            {
                JZNodeGemo to(node_item->id(), pin_id);
                QString error;
                if(!m_file->canConnect(m_selLine->startTraget(),to,error))                                    
                    showTip(scene_pos,"无法连接: " + error);                
            }
        }
    }

    if(event->buttons() == Qt::LeftButton)
    {
        auto grabber_item = scene()->mouseGrabberItem();
        if (grabber_item == nullptr)
        {
            // Make sure shift is not being pressed
            if ((event->modifiers() & Qt::ShiftModifier) == 0)
            {
                QPointF pt_diff = m_downPoint - event->pos();
                double scale = this->transform().m11();
                pt_diff /= scale;

                QPointF pt = m_downCenter + pt_diff;
                sceneCenter(pt);

                m_map->update();
            }
        }
        else if(!rect().contains(event->pos()) && grabber_item->type() > Item_none)
        {
            if(!m_mouseMoveTimer->isActive())
                m_mouseMoveTimer->start(50);
        }
    }


    if (QToolTip::isVisible() && !m_tipPoint.isNull())
    {        
        if((event->pos() - m_tipPoint).manhattanLength() > 15)
            clearTip();
    }
    else
    {
        m_tipPoint = QPoint();
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
            auto pin_id = node_item->pinAt(pos);
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
    }        

    m_downPoint = QPoint();
    m_downCenter = QPointF();
    
    if(m_mouseMoveTimer->isActive())
        m_mouseMoveTimer->stop();
}

void JZNodeView::keyPressEvent(QKeyEvent *event)
{
    if(scene()->focusItem())
    {
        QGraphicsView::keyPressEvent(event);
        return;
    }

    if(event->key() == Qt::Key_Shift)
    {
        setDragMode(QGraphicsView::RubberBandDrag);
        event->accept();
    }
    else if(event->key() == Qt::Key_Delete || event->key() == Qt::Key_Escape)
    {
        event->accept();
    }
    else if(event->key() == Qt::Key_Up || event->key() == Qt::Key_Down 
        || event->key() == Qt::Key_Left || event->key() == Qt::Key_Right)
    {
        if(event->key() == Qt::Key_Up)
            sceneTranslate(0,-10);
        else if(event->key() == Qt::Key_Down)
            sceneTranslate(0,10);
        else if(event->key() == Qt::Key_Left)
            sceneTranslate(-10,0);
        else if(event->key() == Qt::Key_Right)
            sceneTranslate(10,0);
        
        event->accept();
    }
    else
    {
        QGraphicsView::keyPressEvent(event);
    }
}

void JZNodeView::keyReleaseEvent(QKeyEvent *event)
{
    if(scene()->focusItem())
    {
        QGraphicsView::keyReleaseEvent(event);
        return;
    }

    if(event->key() == Qt::Key_Shift)
    {
        setDragMode(QGraphicsView::NoDrag);
        event->accept();
    }
    else if(event->key() == Qt::Key_Delete)
    {
        remove();
        event->accept();
    }
    else if(event->key() == Qt::Key_Escape)
    {
        m_scene->clearSelection();
        cancelLine();
        event->accept();
    } 
    else if(event->key() == Qt::Key_Up || event->key() == Qt::Key_Down 
        || event->key() == Qt::Key_Left || event->key() == Qt::Key_Right)
    {
        event->accept();
    }
    else
    {
        QGraphicsView::keyReleaseEvent(event);
    }
}

bool JZNodeView::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {      
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        auto node = nodeItemAt(helpEvent->pos());
        if (node)
        {            
            QPointF scene_pt = mapToScene(helpEvent->pos());
            QPointF item_pt = node->mapFromScene(scene_pt);
            QString tips = node->getTip(item_pt);
            if (!tips.isEmpty())
            {
                showTip(scene_pt, tips);
                event->ignore();
                return true;
            }
        }
    }

    return QGraphicsView::event(event);
}

void JZNodeView::onItemPropChanged()
{
    QObject *obj = sender();
    int node_id = obj->property("node_id").toInt();
    int prop_id = obj->property("prop_id").toInt();    
    
    QString value = getNodeItem(node_id)->pinValue(prop_id);
    onPropChanged(node_id,prop_id,value);    
}

void JZNodeView::onScriptNodeChanged(JZScriptItem *file, int node_id, const QByteArray &old)
{
    if (m_file != file)
        return;
    
    auto old_node = JZNodeFactory::instance()->loadNode(old);
    auto pre_list = old_node->pinList();
    delete old_node;

    auto node = getNode(node_id);            
    m_commandStack.beginMacro("node changed");
    auto new_list = node->pinList();
    auto remove_set = pre_list.toSet() - new_list.toSet();
    for (auto remove_prop : remove_set)
    {
        auto lines = m_file->getConnectPin(node->id(), remove_prop);
        for (int i = 0; i < lines.size(); i++)
            addRemoveLineCommand(lines[i]);
    }    
    addPropChangedCommand(node->id(), old);
    m_commandStack.endMacro();
}

void JZNodeView::onPropChanged(int id,int pinId,const QString &value)
{
    if(m_runningMode != Process_none)
    {
        int param_id = JZNodeCompiler::paramId(id,pinId);
        emit sigRuntimeValueChanged(param_id,value);
        return;
    }

    QString oldValue = getNode(id)->pinValue(pinId);
    if(oldValue == value)
        return;
        
    auto node = getNode(id);
    auto old = getNodeData(id);
    node->setPinValue(pinId,value);    
    addPinValueChangedCommand(node->id(), pinId, old);
}

void JZNodeView::onDependChanged()
{       
    autoRunning();
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
                QByteArray node_data = JZNodeFactory::instance()->saveNode(node->node());
                copydata.nodes.append(node_data);
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
    emit sigAutoCompiler();
}

void JZNodeView::autoRunning()
{
    if (m_autoRunning)
        emit sigAutoRun();
}

QString JZNodeView::getExpr(const QString &text)
{
    JZNodeExprEditDialog dlg(this);
    dlg.setExpr(text);
    if(dlg.exec() != QDialog::Accepted)
        return QString();

    return dlg.expr();
}

int JZNodeView::popMenu(QStringList list)
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

void JZNodeView::onMapSceneChanged(QRectF rc)
{
    setSceneRect(rc);
}

void JZNodeView::onMapSceneScaled(bool flag)
{
    sceneScale(rect().center(), flag);
}

void JZNodeView::setCompilerResult(const CompilerResult *compilerInfo)
{
    foreachNode([](JZNodeGraphItem *node) {
        node->clearError();
    });
        
    auto &nodeError = compilerInfo->nodeError;
    auto it = nodeError.begin();
    while (it != nodeError.end())
    {
        if (!it->isEmpty())
        {
            auto item = getNodeItem(it.key());
            if(item)
                item->setError(it.value());
        }
        it++;
    }   
    m_map->updateMap();

    if(compilerInfo->result)
        autoRunning();
}