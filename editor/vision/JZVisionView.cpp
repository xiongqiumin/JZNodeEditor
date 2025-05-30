#include <QVector2D>
#include <QMouseEvent>
#include "JZVisionView.h"
#include "JZVisionSettingDialog.h"
#include "JZEditorGlobal.h"
#include "JZNodeUtils.h"

JZVisionView::JZVisionView(QWidget *parent) : QGraphicsView(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &JZVisionView::customContextMenuRequested, this, &JZVisionView::onContextMenu);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    setAlignment(Qt::AlignTop | Qt::AlignLeft);

    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}

JZVisionView::~JZVisionView()
{
}

void JZVisionView::mouseDoubleClickEvent(QMouseEvent* event)
{
    auto item = itemAt(event->pos());
    if (!item || item->type() != Vision_itemNode)
        return;
    
    auto node_item = dynamic_cast<JZVisionNodeItem*>(item);
    auto node = getNode(node_item->id());

    QByteArray old_buffer = editorNodeFactory()->saveNode(node);
    JZVisionSettingDialog dialog;
    dialog.setNode(node);
    if (dialog.exec() != QDialog::Accepted)
        return;

    QByteArray new_buffer = editorNodeFactory()->saveNode(node);
    if (new_buffer == old_buffer)
        return;

    addNodeChangedCommand(node->id(), old_buffer);
}

JZNode* JZVisionView::getNode(int id)
{
    return m_file->getNode(id);
}

JZVisionNodeItem* JZVisionView::createNode(JZNode* node)
{
    int id = m_file->addNode(node);
    return createNodeItem(id);
}

JZVisionNodeItem* JZVisionView::insertNode(JZNode* node)
{
    m_file->insertNode(node);
    return createNodeItem(node->id());
}

void JZVisionView::removeNode(int id)
{
    auto item = getNodeItem(id);
    if (!item)
        return;

    Q_ASSERT(m_file->getConnectPin(id).size() == 0);
    m_scene->removeItem(item);
    delete item;
    m_file->removeNode(id);
}

QByteArray JZVisionView::getNodeData(int id)
{
    auto node = getNode(id);
    return node->toBuffer();
}

void JZVisionView::setNodeData(int id, const QByteArray& buffer)
{
    auto node = getNode(id);
    int old_group = node->group();

    node->fromBuffer(buffer);
    
    updateNode(id);
}

void JZVisionView::setNodePos(int node_id, QPointF pos)
{
    m_recordMove = false;
    auto node = getNode(node_id);
    getNodeItem(node_id)->setPos(pos);
    m_file->setNodePos(node_id, pos);

    auto lineId = m_file->getConnectPin(node_id);
    for (int i = 0; i < lineId.size(); i++)
        getLineItem(lineId[i])->updateNode();

    m_recordMove = true;
}

void JZVisionView::setNodePinValue(int node_id, int pin, QString value)
{
    auto node = getNode(node_id);
    node->setPinValue(pin, value);
}

void JZVisionView::updateNode(int id)
{
    getNodeItem(id)->updateNode();
    auto lineId = m_file->getConnectPin(id);
    for (int i = 0; i < lineId.size(); i++)
        getLineItem(lineId[i])->updateNode();
}

JZVisionNodeItem* JZVisionView::createNodeItem(int id)
{
    auto e = editorManager();

    auto node = m_file->getNode(id);

    JZVisionNodeItem* item = new JZVisionNodeItem();
    m_scene->addItem(item);
    item->setPos(node->pos());
    item->updateNode();
    return item;
}

JZVisionLineItem* JZVisionView::createLine(JZNodeGemo from, JZNodeGemo to)
{
    int id = m_file->addConnect(from, to);
    return createLineItem(id);
}

JZVisionLineItem* JZVisionView::insertLine(const JZNodeConnect& connect)
{
    m_file->insertConnect(connect);
    return createLineItem(connect.id);
}

void JZVisionView::removeLine(int id)
{
    auto item = getLineItem(id);
    if (!item)
        return;

    m_file->removeConnect(id);
    m_scene->removeItem(item);
    delete item;
}

JZVisionLineItem* JZVisionView::createLineItem(int id)
{
    auto info = m_file->getConnect(id);
    JZVisionLineItem* line = new JZVisionLineItem(info->from.nodeId);
    line->setEndTraget(info->to.nodeId);
    line->setId(info->id);
    m_scene->addItem(line);
    line->updateNode();

    return line;
}

JZVisionLineItem* JZVisionView::getLineItem(int id)
{
    auto items = m_scene->items();
    for (int i = 0; i < items.size(); i++)
    {
        if (items[i]->type() == Vision_itemLine && ((JZVisionLineItem*)items[i])->id() == id)
            return (JZVisionLineItem*)items[i];
    }
    return nullptr;
}

void JZVisionView::startLine(JZNodeGemo from)
{
    if (m_selLine)
        return;

    JZVisionNodeItem* node_from = getNodeItem(from.nodeId);
    auto pt = node_from->sceneBoundingRect().center();

    m_selLine = new JZVisionLineItem(from.nodeId);
    m_selLine->setZValue(1);
    m_selLine->setEndPoint(pt);
    m_scene->addItem(m_selLine);
    m_selLine->grabMouse();
    m_selLine->updateNode();
}

void JZVisionView::endLine(JZNodeGemo to)
{
    if (!m_selLine)
        return;

    JZNodeConnect line;
    line.from.nodeId = m_selLine->startTraget();
    line.to = to;

    m_commandStack.beginMacro("create line");

    JZVisionCommand* cmd = new JZVisionCommand(this, JZVisionCommand::CreateLine);
    cmd->itemId = -1;
    cmd->newValue = JZNodeUtils::toBuffer(line);
    m_commandStack.push(cmd);

    auto node = getNode(line.to.nodeId);
    if (node->pin(line.to.pinId)->isParam())
    {
        auto old = getNodeData(line.to.nodeId);
        node->setPinValue(line.to.pinId, QString());
        addPinValueChangedCommand(line.to.nodeId, line.to.pinId, old);
    }
    m_commandStack.endMacro();

    m_selLine->ungrabMouse();
    delete m_selLine;
    m_selLine = nullptr;
}

void JZVisionView::cancelLine()
{
    if (!m_selLine)
        return;

    m_selLine->ungrabMouse();
    if (m_selLine->endTraget() == INVALID_ID)
    {
        m_scene->removeItem(m_selLine);
        delete m_selLine;
        m_scene->update();
    }
    m_selLine = nullptr;
}

void JZVisionView::addCreateNodeCommand(const QByteArray& buffer, QPointF pt)
{
    JZVisionCommand* cmd = new JZVisionCommand(this, JZVisionCommand::CreateNode);
    cmd->itemId = -1;
    cmd->newValue = buffer;
    cmd->newPos = pt;
    m_commandStack.push(cmd);
}

void JZVisionView::addRemoveLineCommand(int line_id)
{
    auto line = m_file->getConnect(line_id);

    JZVisionCommand* cmd = new JZVisionCommand(this, JZVisionCommand::RemoveLine);
    cmd->itemId = line->id;
    cmd->oldValue = JZNodeUtils::toBuffer(*line);
    m_commandStack.push(cmd);
}

void JZVisionView::addNodeChangedCommand(int id, const QByteArray& oldValue)
{
    JZVisionCommand* cmd = new JZVisionCommand(this, JZVisionCommand::NodeChange);
    cmd->itemId = id;
    cmd->oldValue = oldValue;
    m_commandStack.push(cmd);
}

void JZVisionView::addPinValueChangedCommand(int id, int pin_id, const QString& value)
{
    JZVisionPinValueChangedCommand* cmd = new JZVisionPinValueChangedCommand(this);
    cmd->nodeId = id;
    cmd->pinId = pin_id;
    cmd->newValue = value;
    cmd->oldValue = getNode(id)->pinValue(pin_id);
    m_commandStack.push(cmd);
}

void JZVisionView::addMoveNodeCommand(int id, QPointF pt)
{
    JZVisionMoveCommand* cmd = new JZVisionMoveCommand(this);
    JZVisionMoveCommand::NodePosInfo info;
    info.itemId = id;
    info.oldPos = getNodeItem(id)->pos();
    info.newPos = pt;
    cmd->nodeList.push_back(info);
    m_commandStack.push(cmd);
}