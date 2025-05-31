#include <QVector2D>
#include <QMouseEvent>
#include "JZVisionView.h"
#include "JZVisionSettingDialog.h"
#include "JZEditorGlobal.h"
#include "JZNodeUtils.h"

JZVisionView::JZVisionView(QWidget *parent)
{

}

JZVisionView::~JZVisionView()
{
}

void JZVisionView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_selLine)
    {
        JZAbstractNodeItem *node_item = nodeItemAt(event->pos());
        if (node_item && m_selLine->startTraget().nodeId != node_item->id())
        {
            auto scene_pos = mapToScene(event->pos());
            auto item_pos = node_item->mapFromScene(scene_pos);
            auto pin_id = node_item->node()->flowIn();
            if (pin_id >= 0)
            {
                JZNodeGemo to(node_item->id(), pin_id);
                QString error;
                if (!m_file->canConnect(m_selLine->startTraget(), to, error))
                    showTip(scene_pos, "无法连接: " + error);
            }
        }
    }

    JZNodeAbstractView::mouseMoveEvent(event);
}

void JZVisionView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_selLine)
    {
        JZAbstractNodeItem *node_item = nodeItemAt(event->pos());
        JZNodeGemo gemo;
        if (node_item)
        {
            gemo.nodeId = node_item->node()->id();
            gemo.pinId = node_item->node()->flowIn();
            QString error;
            if (gemo.pinId == -1 || !m_file->canConnect(m_selLine->startTraget(), gemo, error))
                gemo = JZNodeGemo();
        }

        if (gemo.nodeId != INVALID_ID)
            endLine(gemo);
        else
            cancelLine();
    }

    JZNodeAbstractView::mouseReleaseEvent(event);
}

void JZVisionView::mouseDoubleClickEvent(QMouseEvent* event)
{
    auto item = itemAt(event->pos());
    if (!item || item->type() != Item_node)
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

JZAbstractNodeItem *JZVisionView::createNodeItem(JZNode *node)
{
    return new JZVisionNodeItem(node);
}

JZAbstractLineItem *JZVisionView::createLineItem(JZNodeGemo from)
{
    return new JZVisionLineItem(from);
}