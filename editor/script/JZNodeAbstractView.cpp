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
#include "JZNodeAbstractPanel.h"
#include "JZNodeFlowPanel.h"
#include "JZNodeUtils.h"
#include "JZEditorGlobal.h"
#include "JZNodeAbstractView.h"
#include "JZNodeDisplayItem.h"
#include "JZScriptItemVisitor.h"
#include "JZNodeParamEditWidget.h"
#include "JZNodeUtils.h"

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
    QDataStream s(&buffer, QIODevice::WriteOnly);
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

//JZNodeAbstractView
JZNodeAbstractView::JZNodeAbstractView(QWidget *widget)
    : QGraphicsView(widget)
{    
    m_selLine = nullptr;    
    m_panel = nullptr;
    m_loadFlag = false;    
    m_file = nullptr;        
    m_recordMove = true;        
    m_groupIsMoving = false;        

    connect(&m_commandStack,&QUndoStack::cleanChanged, this, &JZNodeAbstractView::onCleanChanged);
    connect(&m_commandStack,&QUndoStack::canRedoChanged,this,&JZNodeAbstractView::redoAvailable);
    connect(&m_commandStack,&QUndoStack::canUndoChanged,this,&JZNodeAbstractView::undoAvailable);
    connect(&m_commandStack,&QUndoStack::indexChanged, this,&JZNodeAbstractView::onUndoStackChanged);

    m_map = new JZNodeViewMap(this);
    m_map->setFixedSize(160, 120);
    m_map->setView(this);
    connect(m_map, &JZNodeViewMap::mapSceneChanged, this, &JZNodeAbstractView::onMapSceneChanged);
    connect(m_map, &JZNodeViewMap::mapSceneScaled, this, &JZNodeAbstractView::onMapSceneScaled);

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->addStretch();
    hbox->addWidget(m_map);
    hbox->setAlignment(Qt::AlignTop);
    this->setLayout(hbox);

    m_scene = new QGraphicsScene();
    setScene(m_scene);

    setAcceptDrops(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); 

    m_mouseMoveTimer = new QTimer(this);
    connect(m_mouseMoveTimer, &QTimer::timeout, this, &JZNodeAbstractView::onMouseMoveTimer);    
    
    setAlignment(Qt::AlignTop | Qt::AlignLeft);
    setRenderHint(QPainter::Antialiasing);    
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
}

JZNodeAbstractView::~JZNodeAbstractView()
{
    this->disconnect();
}

void JZNodeAbstractView::drawBackground(QPainter* painter, const QRectF& r)
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

void JZNodeAbstractView::setPanel(JZNodeAbstractPanel *panel)
{
    m_panel = panel;
}

void JZNodeAbstractView::setFile(JZScriptItem *file)
{
    m_file = file;
    connect(m_file->project(), &JZProject::sigScriptNodeChanged, this, &JZNodeAbstractView::onScrpitNodeChanged);    

    initGraph();
    autoCompiler();    
}

JZScriptItem *JZNodeAbstractView::file()
{
    return m_file;
}

void JZNodeAbstractView::resetFile()
{    
    m_file->loadFinish();
}

bool JZNodeAbstractView::isModified()
{
    return !m_commandStack.isClean();
}

void JZNodeAbstractView::saveNodePos()
{
    foreachNode([this](JZAbstractNodeItem *node) {
        m_file->setNodePos(node->id(), node->pos());
    });
}

JZNode *JZNodeAbstractView::getNode(int id)
{
    return m_file->getNode(id);
}

JZNodePin *JZNodeAbstractView::getPin(JZNodeGemo gemo)
{
    return m_file->getPin(gemo);
}

JZAbstractNodeItem *JZNodeAbstractView::createNode(JZNode *node)
{        
    int id = m_file->addNode(node);
    return createNodeItem(id);
}

JZAbstractNodeItem *JZNodeAbstractView::insertNode(JZNode *node)
{
    m_file->insertNode(node);
    return createNodeItem(node->id());
}

void JZNodeAbstractView::removeNode(int id)
{
    auto item = getNodeItem(id);
    if (!item)
        return;
            
    Q_ASSERT(m_file->getConnectPin(id).size() == 0);
    m_scene->removeItem(item);
    delete item;
    m_file->removeNode(id);
    m_map->updateMap();
}

QByteArray JZNodeAbstractView::getNodeData(int id)
{
    auto node = getNode(id);
    return node->toBuffer();
}

void JZNodeAbstractView::setNodeData(int id,const QByteArray &buffer)
{    
    auto node = getNode(id);
    int old_group = node->group();

    node->fromBuffer(buffer);
    if (old_group != -1 && old_group != node->group())
        getGroupItem(old_group)->updateNode();

    updateNode(id);
}

void JZNodeAbstractView::setNodePos(int node_id, QPointF pos)
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

void JZNodeAbstractView::setNodePinValue(int id, int pin, QString value)
{
    getNode(id)->setPinValue(pin, value);
}

bool JZNodeAbstractView::isPropEditable(int id,int prop_id)
{    
    QList<int> lines = m_file->getConnectInput(id,prop_id);
    return lines.size() == 0;
}

JZAbstractNodeItem *JZNodeAbstractView::createNodeItem(int id)
{        
    auto e = editorManager();

    auto node = m_file->getNode(id);

    JZAbstractNodeItem *item = createNodeItem(node);
    m_scene->addItem(item);
    item->setPos(node->pos());
    item->updateNode();    
    m_map->updateMap();
    return item;
}

JZAbstractNodeItem *JZNodeAbstractView::getNodeItem(int id)
{
    auto items = m_scene->items();
    for (int i = 0; i < items.size(); i++)
    {
        if (items[i]->type() == Item_node && ((JZAbstractNodeItem *)items[i])->id() == id)
            return (JZAbstractNodeItem *)items[i];
    }
    return nullptr;
}

void JZNodeAbstractView::onMouseMoveTimer()
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

void JZNodeAbstractView::updateNode(int id)
{
    getNodeItem(id)->updateNode();
    auto lineId = m_file->getConnectPin(id);
    for (int i = 0; i < lineId.size(); i++)
        getLineItem(lineId[i])->updateNode();

    int group = getNode(id)->group();
    if (group != -1)
        getGroupItem(group)->updateNode();
}

void JZNodeAbstractView::updatePropEditable(const JZNodeGemo &gemo)
{
    getNodeItem(gemo.nodeId)->updateNode();    
}

JZAbstractLineItem *JZNodeAbstractView::createLine(JZNodeGemo from, JZNodeGemo to)
{
    int id = m_file->addConnect(from, to);    
    updatePropEditable(to);        
    return createLineItem(id);
}

JZAbstractLineItem *JZNodeAbstractView::insertLine(const JZNodeConnect &connect)
{
    m_file->insertConnect(connect);
    updatePropEditable(connect.to);    
    return createLineItem(connect.id);
}

void JZNodeAbstractView::removeLine(int id)
{
    auto item = getLineItem(id);
    if (!item)
        return;

    m_file->removeConnect(id);
    JZNodeGemo to = item->endTraget();
    if(m_file->getPin(to))       //节点内部事件处理先删除pin, 外面再删除线条，会存在to 不存在的情况
        updatePropEditable(to);
    
    m_scene->removeItem(item);
    delete item;
}

JZAbstractLineItem *JZNodeAbstractView::createLineItem(int id)
{
    auto info = m_file->getConnect(id);
    JZAbstractLineItem *line = createLineItem(info->from);
    line->setEndTraget(info->to);
    line->setId(info->id);
    m_scene->addItem(line);
    line->updateNode();

    return line;
}

JZAbstractLineItem *JZNodeAbstractView::getLineItem(int id)
{
    auto items = m_scene->items();
    for (int i = 0; i < items.size(); i++)
    {
        if (items[i]->type() == Item_line && ((JZAbstractLineItem *)items[i])->id() == id)
            return (JZAbstractLineItem *)items[i];
    }
    return nullptr;
}

void JZNodeAbstractView::startLine(JZNodeGemo from)
{
    if (m_selLine)
        return;

    JZAbstractNodeItem *node_from = getNodeItem(from.nodeId);
    auto pt = node_from->mapToScene(node_from->pinRect(from.pinId).center());

    m_selLine = createLineItem(from);
    m_selLine->setZValue(1);
    m_selLine->setEndPoint(pt);
    m_scene->addItem(m_selLine);
    m_selLine->grabMouse();
    m_selLine->updateNode();
}

void JZNodeAbstractView::endLine(JZNodeGemo to)
{
    if (!m_selLine)
        return;

    JZNodeConnect line;
    line.from = m_selLine->startTraget();
    line.to = to;

    m_commandStack.beginMacro("create line");

    JZNodeViewCommand *cmd = new JZNodeViewCommand(this, ViewCommand::CreateLine);
    cmd->itemId = -1;
    cmd->newValue = JZNodeUtils::toBuffer(line);
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

void JZNodeAbstractView::cancelLine()
{
    if (!m_selLine)
        return;

    m_selLine->ungrabMouse();
    if (m_selLine->endTraget().nodeId == INVALID_ID)
    {
        m_scene->removeItem(m_selLine);
        delete m_selLine;
        m_scene->update();
    }
    m_selLine = nullptr;
}

JZNodeGroupItem *JZNodeAbstractView::createGroupItem(int id)
{
    JZNodeGroupItem *item = new JZNodeGroupItem(id);
    m_scene->addItem(item);
    item->updateNode();
    return item;
}

JZNodeGroupItem *JZNodeAbstractView::createGroup(const JZNodeGroup &group)
{
    int id = m_file->addGroup(group);
    return createGroupItem(id);
}

JZNodeGroupItem *JZNodeAbstractView::insertGroup(const JZNodeGroup &group)
{
    m_file->insertGroup(group);
    return createGroupItem(group.id);
}
void JZNodeAbstractView::removeGroup(int id)
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

JZNodeGroupItem *JZNodeAbstractView::getGroupItem(int id)
{
    auto items = m_scene->items();
    for (int i = 0; i < items.size(); i++)
    {
        if (items[i]->type() == Item_group && ((JZNodeGroupItem *)items[i])->id() == id)
            return (JZNodeGroupItem *)items[i];
    }
    return nullptr;
}

QByteArray JZNodeAbstractView::getGroupData(int id)
{
    auto group = m_file->getGroup(id);
    return JZNodeUtils::toBuffer(*group);
}

void JZNodeAbstractView::setGroupData(int id, QByteArray buffer)
{
    auto group = m_file->getGroup(id);
    JZNodeGroup g = JZNodeUtils::fromBuffer<JZNodeGroup>(buffer);
    *group = g;
}

void JZNodeAbstractView::updateGroup(int id)
{
    getGroupItem(id)->updateNode();
}

void JZNodeAbstractView::addLocalVariableCommand(JZParamDefine def)
{
    JZNodeVariableCommand *cmd = new JZNodeVariableCommand(this, ViewCommand::AddLocalVariable);
    cmd->newParam = def;
    m_commandStack.push(cmd);
}

void JZNodeAbstractView::removeLocalVariableCommand(QString name)
{
    JZNodeVariableCommand *cmd = new JZNodeVariableCommand(this, ViewCommand::RemoveLocalVariable);
    auto def = *m_file->localVariable(name);
    cmd->oldParam = def;
    m_commandStack.push(cmd);
}

void JZNodeAbstractView::changeLocalVariableCommand(QString name, JZParamDefine def)
{
    JZNodeVariableCommand *cmd = new JZNodeVariableCommand(this, ViewCommand::ChangeLocalVariable);
    auto old_def = *m_file->localVariable(name);
    cmd->newParam = def;
    cmd->oldParam = old_def;
    m_commandStack.push(cmd);
}

void JZNodeAbstractView::addLocalVariable(JZParamDefine def)
{
    m_file->addLocalVariable(def);    
    m_panel->updateDefine();
}

void JZNodeAbstractView::removeLocalVariable(QString name)
{
    m_file->removeLocalVariable(name);    
    m_panel->updateDefine();
}

void JZNodeAbstractView::changeLocalVariable(QString name, JZParamDefine def)
{
    m_file->setLocalVariable(name, def);    
    m_panel->updateDefine();
}

void JZNodeAbstractView::showTip(QPointF pt,QString text)
{            
    auto tip = this->mapFromScene(pt);
    if (QToolTip::isVisible() && (tip - m_tipPoint).manhattanLength() < 15)
        return;

    LinkInfo link = JZNodeUtils::parseLink(text);

    m_tipPoint = tip;
    tip = mapToGlobal(tip);    
    QToolTip::showText(tip , link.text, nullptr, QRect(), 15 * 1000);
}

void JZNodeAbstractView::clearTip()
{
    QToolTip::hideText();
    m_tipPoint = QPoint();
}

JZAbstractNodeItem *JZNodeAbstractView::nodeItemAt(QPoint pos)
{
    QList<QGraphicsItem *> list = m_scene->items(mapToScene(pos));
    for(int i = 0; i < list.size(); i++)
    {
        if(list[i]->type() == Item_node)
            return dynamic_cast<JZAbstractNodeItem*>(list[i]);
    }
    return nullptr;
}

JZNodeGemo JZNodeAbstractView::pinAt(QPoint pos)
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

void JZNodeAbstractView::foreachNode(std::function<void(JZAbstractNodeItem *)> func)
{
    foreachNode(-1, func);
}

void JZNodeAbstractView::foreachNode(int nodeType, std::function<void(JZAbstractNodeItem *)> func)
{
    auto items = m_scene->items();
    for (int i = 0; i < items.size(); i++)
    {
        if (items[i]->type() == Item_node)
        {
            JZAbstractNodeItem *node_item = (JZAbstractNodeItem *)items[i];
            if (nodeType == -1 || node_item->node()->type() == nodeType)
                func(node_item);
        }
    }
}

void JZNodeAbstractView::foreachLine(std::function<void(JZAbstractLineItem *)> func)
{
    auto items = m_scene->items();
    for (int i = 0; i < items.size(); i++)
    {
        if (items[i]->type() == Item_line)
            func((JZAbstractLineItem *)items[i]);
    }
}

QVariant JZNodeAbstractView::onItemChange(JZNodeBaseItem *item, QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (m_loadFlag)
        return value;

    if (change == QGraphicsItem::ItemPositionChange)
    {
        if (item->type() == Item_node)
        {
            if (m_recordMove)
            {
                auto node = ((JZAbstractNodeItem *)item)->node();
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
                    auto node = ((JZAbstractNodeItem *)list[i])->id();
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

void JZNodeAbstractView::initGraph()
{    
    m_loadFlag = true;
    m_scene->clear();

    QList<int> node_list = m_file->nodeList();
    for (int i = 0; i < node_list.size(); i++)
    {
        createNodeItem(node_list[i]);                
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
}

void JZNodeAbstractView::clear()
{
    m_scene->clear();
    m_file->clear();
    m_commandStack.clear();    
}

void JZNodeAbstractView::save()
{
    auto project = m_file->project();
    saveNodePos();        
    project->saveItem(m_file);
    m_commandStack.setClean();
}

void JZNodeAbstractView::redo()
{
    m_commandStack.redo();    
}

void JZNodeAbstractView::undo()
{
    m_commandStack.undo();    
}

void JZNodeAbstractView::remove()
{
    auto items = m_scene->selectedItems();
    removeItems(items);
}

void JZNodeAbstractView::cut()
{
    auto items = m_scene->selectedItems();
    copyItems(items);
    removeItems(items);
}

void JZNodeAbstractView::copy()
{
    auto items = m_scene->selectedItems();
    copyItems(items);
}

void JZNodeAbstractView::paste()
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
        JZNode *node = editorNodeFactory()->loadNode(copyData.nodes[i]);
        int old_id = node->id();
        node->setId(-1);
        node->setGroup(-1);

        JZNodeViewCommand *cmd = new JZNodeViewCommand(this, ViewCommand::CreateNode);
        cmd->itemId = -1;
        cmd->newValue = editorNodeFactory()->saveNode(node);
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
        cmd->newValue = JZNodeUtils::toBuffer(line);
        m_commandStack.push(cmd);
    }
    m_commandStack.endMacro();
}

void JZNodeAbstractView::selectAll()
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

void JZNodeAbstractView::onCleanChanged(bool clean)
{
    emit modifyChanged(!clean);
}

void JZNodeAbstractView::setSelectNode(int id)
{
    if(id != -1)
    {
        auto node = getNode(id);
        emit sigSelectNodeChanged(getNodeItem(id));
    }
    else
    {
        emit sigSelectNodeChanged(nullptr);
    }
}

void JZNodeAbstractView::fitNodeView()
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

void JZNodeAbstractView::ensureNodeVisible(int id)
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

void JZNodeAbstractView::selectNode(int id)
{
    ensureNodeVisible(id);
    m_scene->clearSelection();
    getNodeItem(id)->setSelected(true);
}

void JZNodeAbstractView::setCompilerResult(const CompilerResult *compilerInfo)
{
    foreachNode([](JZAbstractNodeItem *node) {
        node->clearError();
    });

    auto &nodeError = compilerInfo->nodeError;
    auto it = nodeError.begin();
    while (it != nodeError.end())
    {
        if (!it->isEmpty())
        {
            auto item = getNodeItem(it.key());
            if (item)
                item->setError(it.value());
        }
        it++;
    }
    m_map->updateMap();
}

bool JZNodeAbstractView::canRemoveItem(QGraphicsItem *item)
{
    auto item_id = ((JZNodeBaseItem *)item)->id();
    if (item->type() == Item_node)
    {
        if (!getNode(item_id)->canRemove())
            return false;
    }
    return true;
}

void JZNodeAbstractView::removeItem(QGraphicsItem *item)
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
            cmd->oldValue = JZNodeUtils::toBuffer(*line);
            m_commandStack.push(cmd);
        }

        auto node = m_file->getNode(item_id);
        int group = node->group();

        JZNodeViewCommand *cmd = new JZNodeViewCommand(this, ViewCommand::RemoveNode);
        cmd->itemId = node->id(); 
        cmd->oldValue = editorNodeFactory()->saveNode(node);
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
        cmd->oldValue = JZNodeUtils::toBuffer(*line);
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
            addNodeValueChangedCommand(list[i], oldValue);
        }

        addRemoveGroupCommand(item_id);
        m_commandStack.endMacro();
    }
}

QList<JZAbstractNodeItem*> JZNodeAbstractView::nodeItems()
{
    QList<JZAbstractNodeItem*> result;
    auto list = m_scene->items();
    for (int i = 0; i < list.size(); i++)
    {
        if (list[i]->type() == Item_node)
            result << dynamic_cast<JZAbstractNodeItem *>(list[i]);
    }
    return result;
}

QList<JZAbstractNodeItem*> JZNodeAbstractView::selectNodeItems()
{
    QList<JZAbstractNodeItem*> result;
    auto list = m_scene->selectedItems();
    for (int i = 0; i < list.size(); i++)
    {
        if (list[i]->type() == Item_node)
            result << dynamic_cast<JZAbstractNodeItem *>(list[i]);
    }
    return result;
}

void JZNodeAbstractView::addCreateNodeCommand(const QByteArray &buffer,QPointF pt)
{
    JZNodeViewCommand *cmd = new JZNodeViewCommand(this, ViewCommand::CreateNode);
    cmd->itemId = -1;
    cmd->newValue = buffer;
    cmd->newPos = pt;
    m_commandStack.push(cmd);
}

void JZNodeAbstractView::addRemoveLineCommand(int line_id)
{
    auto line = m_file->getConnect(line_id);

    JZNodeViewCommand *cmd = new JZNodeViewCommand(this, ViewCommand::RemoveLine);
    cmd->itemId = line->id;
    cmd->oldValue = JZNodeUtils::toBuffer(*line);
    m_commandStack.push(cmd);
}

void JZNodeAbstractView::addNodeChangedCommand(int node_id, const QByteArray &old)
{
    auto node = getNode(node_id);
    m_commandStack.beginMacro("node changed");
    auto pin_list = node->pinList();

    //删除输入线
    QList<int> lines = m_file->getConnectInput(node_id);
    for (int i = 0; i < lines.size(); i++)
    {
        auto line = m_file->getConnect(lines[i]);
        if (!pin_list.contains(line->to.pinId))
            addRemoveLineCommand(lines[i]);
    }

    //删除输出线
    lines = m_file->getConnectOut(node_id);
    for (int i = 0; i < lines.size(); i++)
    {
        auto line = m_file->getConnect(lines[i]);
        if (!pin_list.contains(line->from.pinId))
            addRemoveLineCommand(lines[i]);
    }

    addNodeValueChangedCommand(node->id(), old);
    m_commandStack.endMacro();
}

void JZNodeAbstractView::addNodeValueChangedCommand(int id,const QByteArray &oldValue)
{
    JZNodeViewCommand *cmd = new JZNodeViewCommand(this, ViewCommand::NodeChange);
    cmd->itemId = id;
    cmd->oldValue = oldValue;
    m_commandStack.push(cmd);
}

void JZNodeAbstractView::addPinValueChangedCommand(int id, int pin_id, const QString &value)
{
    JZNodePinValueChangedCommand *cmd = new JZNodePinValueChangedCommand(this);
    cmd->nodeId = id;
    cmd->pinId = pin_id;
    cmd->newValue = value;
    cmd->oldValue = getNode(id)->pinValue(pin_id);
    m_commandStack.push(cmd);
}

void JZNodeAbstractView::addMoveNodeCommand(int id, QPointF pt)
{
    JZNodeMoveCommand *cmd = new JZNodeMoveCommand(this, ViewCommand::MoveNode);
    JZNodeMoveCommand::NodePosInfo info;
    info.itemId = id;
    info.oldPos = getNodeItem(id)->pos();
    info.newPos = pt;
    cmd->nodeList.push_back(info);
    m_commandStack.push(cmd);
}

int JZNodeAbstractView::addCreateGroupCommand(const JZNodeGroup &group)
{
    int id = m_file->nextId();

    JZNodeViewCommand *cmd = new JZNodeViewCommand(this, ViewCommand::CreateGroup);
    cmd->newValue = JZNodeUtils::toBuffer(group);
    m_commandStack.push(cmd);    
    return id;
}

void JZNodeAbstractView::addRemoveGroupCommand(int id)
{
    auto group = m_file->getGroup(id);

    JZNodeViewCommand *cmd = new JZNodeViewCommand(this, ViewCommand::RemoveGroup);
    cmd->itemId = id;
    cmd->oldValue = JZNodeUtils::toBuffer(*group);
    m_commandStack.push(cmd);
}

void JZNodeAbstractView::addSetGroupCommand(int id, const JZNodeGroup &new_group)
{
    auto group = m_file->getGroup(id);    

    JZNodeViewCommand *cmd = new JZNodeViewCommand(this, ViewCommand::SetGroup);
    cmd->itemId = id;
    cmd->oldValue = JZNodeUtils::toBuffer(*group);
    *group = new_group;
    m_commandStack.push(cmd);    
}

void JZNodeAbstractView::dragEnterEvent(QDragEnterEvent *event)
{    
    if (event->mimeData()->hasFormat("node_data")
        || event->mimeData()->hasFormat("node_param")
        || event->mimeData()->hasFormat("node_class"))
        event->acceptProposedAction();
}

void JZNodeAbstractView::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void JZNodeAbstractView::dropEvent(QDropEvent *event)
{        
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

void JZNodeAbstractView::showEvent(QShowEvent *event)
{
    QGraphicsView::showEvent(event);
}

void JZNodeAbstractView::resizeEvent(QResizeEvent *event)
{
    m_map->updateMap();
    QGraphicsView::resizeEvent(event);
}

void JZNodeAbstractView::sceneScale(QPoint center,bool up)
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

void JZNodeAbstractView::sceneTranslate(int x,int y)
{
    QPoint center_pt = rect().center();
    center_pt += QPoint(x,y);   
    sceneCenter(mapToScene(center_pt));
}

void JZNodeAbstractView::sceneCenter(QPointF pt)
{
    QRectF view_rc = mapToScene(rect()).boundingRect();
    int w = view_rc.width();
    int h = view_rc.height();
    QRectF rc(pt.x() - w/2,pt.y() - h/2,w,h);
    setSceneRect(rc);
}

void JZNodeAbstractView::wheelEvent(QWheelEvent *event)
{
    if(scene()->focusItem())
    {
        QGraphicsView::wheelEvent(event);
        return;
    }

    sceneScale(event->pos(),event->angleDelta().y() > 0);
    event->accept();
}

void JZNodeAbstractView::mousePressEvent(QMouseEvent *event)
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

void JZNodeAbstractView::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    viewport()->setCursor(QCursor());

    if (m_selLine)
    {
        m_selLine->setEndPoint(mapToScene(event->pos()));

        JZAbstractNodeItem *node_item = nodeItemAt(event->pos());
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

void JZNodeAbstractView::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    viewport()->setCursor(QCursor());

    if (m_selLine)
    {
        JZNodeGemo gemo;
        JZAbstractNodeItem *node_item = nodeItemAt(event->pos());
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

void JZNodeAbstractView::keyPressEvent(QKeyEvent *event)
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

void JZNodeAbstractView::keyReleaseEvent(QKeyEvent *event)
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

bool JZNodeAbstractView::event(QEvent *event)
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

void JZNodeAbstractView::onScrpitNodeChanged(JZScriptItem *item, int nodeId, const QByteArray &buffer)
{
    if (m_file != item)
        return;

    addNodeChangedCommand(nodeId, buffer);
}

void JZNodeAbstractView::copyItems(QList<QGraphicsItem*> items)
{
    CopyData copydata;

    QVector<int> node_ids;
    for (int i = 0; i < items.size(); i++)
    {
        auto item = items[i];
        if (item->type() == Item_node)
        {
            JZAbstractNodeItem *node = (JZAbstractNodeItem*)item;
            if (node->node()->canRemove())
            {
                node_ids << ((JZAbstractNodeItem*)item)->id();
                QByteArray node_data = editorNodeFactory()->saveNode(node->node());
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
            JZAbstractLineItem *line = (JZAbstractLineItem*)item;
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

void JZNodeAbstractView::removeItems(QList<QGraphicsItem*> items)
{
    bool can_remove = false;;
    for (int i = 0; i < items.size(); i++)
    {
        if (canRemoveItem(items[i]))
        {
            can_remove = true;
            break;
        }
    }
    if (!can_remove)
        return;

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

void JZNodeAbstractView::autoCompiler()
{        
    emit sigAutoCompiler();
}

int JZNodeAbstractView::popMenu(QStringList list)
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

void JZNodeAbstractView::onUndoStackChanged()
{
    autoCompiler();
}

void JZNodeAbstractView::onMapSceneChanged(QRectF rc)
{
    setSceneRect(rc);
}

void JZNodeAbstractView::onMapSceneScaled(bool flag)
{
    sceneScale(rect().center(), flag);
}