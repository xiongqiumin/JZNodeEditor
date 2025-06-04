#include <QVector2D>
#include <QMouseEvent>
#include "JZVisionView.h"
#include "JZVisionSettingDialog.h"
#include "JZEditorGlobal.h"
#include "JZNodeUtils.h"

JZVisionView::JZVisionView(QWidget *parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &JZVisionView::customContextMenuRequested, this, &JZVisionView::onContextMenu);
}

JZVisionView::~JZVisionView()
{
}

bool JZVisionView::nodeIdCmp(const JZNode* n1, const JZNode* n2)
{
    return n1->id() < n2->id();
}

QString JZVisionView::nodeName(JZNode* node)
{
    auto node_list = m_file->nodeList();
    std::sort(node_list.begin(), node_list.end());
    if (node_list != m_cacheNodeList)
    {
        for (int i = 0; i < node_list.size(); i++)
        {
            JZNode* cur_node = getNode(node_list[i]);
            int seq = i;

            auto node_type_list = m_file->findNodeByType(cur_node->type());
            std::sort(node_type_list.begin(), node_type_list.end(), nodeIdCmp);
            int type_seq = node_type_list.indexOf(cur_node);
            m_nodeName[cur_node] = QString::number(seq) + "." + cur_node->name() + QString::number(type_seq + 1);
        }
    }
    return m_nodeName[node]; 
}

QString JZVisionView::pinName(JZNodeGemo gemo)
{
    if (gemo.isNull())
        return QString();

    JZNode* node = getNode(gemo.nodeId);
    QString ret = nodeName(node);
    return ret + "." + node->pinName(gemo.pinId);
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
    configNode(node);    
}

JZAbstractNodeItem *JZVisionView::createNodeItem(JZNode *node)
{
    return new JZVisionNodeItem(node);
}

JZAbstractLineItem *JZVisionView::createLineItem(JZNodeGemo from)
{
    return new JZVisionLineItem(from);
}

void JZVisionView::configNode(JZNode *node)
{
    QByteArray old_buffer = editorNodeFactory()->saveNode(node);
    JZVisionSettingDialog dialog(this);
    dialog.setNode(node);
    if (dialog.exec() != QDialog::Accepted)
        return;

    bool macro_flag = false;
    auto addMacro = [this, &macro_flag]
    {
        if (!macro_flag)
        {
            m_commandStack.beginMacro("change value");
            macro_flag = true;
        }
    };

    QByteArray new_buffer = editorNodeFactory()->saveNode(node);
    auto block_list = dialog.blockList();
    auto it = block_list.begin();
    while (it != block_list.end())
    {
        int pin_id = it.key();
        auto in_list = m_file->getConnectInput(node->id(), pin_id);
        bool pre_link = (in_list.size() > 0);
        QString pre_value = node->pinValue(pin_id);
        JZNodeGemo pre_gemo;
        if (pre_link)
            pre_gemo = m_file->getConnect(in_list[0])->from;

        auto pin_widget = it->pinWidget;
        if (pre_link && pin_widget->isLink())
        {
            if (pre_gemo != pin_widget->linkGemo())
            {
                addMacro();
                addRemoveLineCommand(in_list[0]);
                addCreateLineConmmand(pin_widget->linkGemo(), JZNodeGemo(node->id(), pin_id));
            }
        }
        else if (pre_link && !pin_widget->isLink())
        {
            addMacro();
            addRemoveLineCommand(in_list[0]);
        }
        else if (!pre_link && pin_widget->isLink())
        {
            addMacro();
            addCreateLineConmmand(pin_widget->linkGemo(), JZNodeGemo(node->id(), pin_id));
        }
        else if (!pre_link && !pin_widget->isLink())
        {

        }

        it++;
    }

    if (old_buffer != new_buffer)
    {
        addMacro();
        addNodeChangedCommand(node->id(), old_buffer);
    }

    if (macro_flag)
        m_commandStack.endMacro();

    addNodeChangedCommand(node->id(), old_buffer);
}

void JZVisionView::onContextMenu(const QPoint &pos)
{
    auto item = itemAt(pos);

    QMenu menu(this);
    QAction *actSetting = nullptr;
    QList<QAction*> addList;
    if (item)
        actSetting = menu.addAction("设置");
    else
    {

    }


    QAction *ret = menu.exec(this->mapToGlobal(pos));
    setFocusProxy(this);
    if (!ret)
        return;

    if (ret == actSetting)
    {

    }
}