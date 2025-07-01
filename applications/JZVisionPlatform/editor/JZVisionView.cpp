#include <QVector2D>
#include <QMouseEvent>
#include "JZVisionView.h"
#include "JZVisionSettingDialog.h"
#include "JZEditorGlobal.h"
#include "JZNodeUtils.h"
#include "JZVisionAppNode.h"
#include "JZScriptItemVisitor.h"
#include "JZVisionStringTable.h"

class NodeSeqVisitor : public JZScriptItemVisitor
{
public:
    NodeSeqVisitor()
    {
        seq = 0;
    }

    virtual void visitSelf(JZNode* node)
    {
        if (node->isFlowNode())
            nodeSeq[node] = seq++;
    }

    QMap<JZNode*,int> nodeSeq;
    int seq;
};

//JZVisionView
JZVisionView::JZVisionView(QWidget *parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &JZVisionView::customContextMenuRequested, this, &JZVisionView::onContextMenu);
}

JZVisionView::~JZVisionView()
{
}

JZVisionParamLink JZVisionView::linkInfo(int node_id, int pin_id)
{
    QList<int> in_list = m_file->getConnectInput(node_id, pin_id);
    if (in_list.size() == 0)
        return JZVisionParamLink();
    
    JZVisionParamLink linkGemo;
    auto from_gemo = m_file->getConnect(in_list[0])->from;
    auto from_node = m_file->getNode(from_gemo.nodeId);
    if (from_node->type() == Node_visionAppParam)
    {
        JZNodeVisionParam* node_param = dynamic_cast<JZNodeVisionParam*>(from_node);        
        linkGemo = node_param->link();
    }
    else
    {
        linkGemo.type = JZVisionParamLink::Link_Node;
        linkGemo.gemo = from_gemo;        
    }
    return linkGemo;
}

bool JZVisionView::nodeIdCmp(const JZNode* n1, const JZNode* n2)
{
    return n1->id() < n2->id();
}

QString JZVisionView::nodeBaseName(JZNode* node)
{
    return JZVisionStringTable::instance()->nodeName(node);
}

QString JZVisionView::nodeName(int node_id)
{
    JZNode* node = m_file->getNode(node_id);
    if (!m_nodeName.contains(node))
        return nodeBaseName(node);

    return m_nodeName[node];
}

void JZVisionView::updateNodeName()
{
    NodeSeqVisitor visitor;
    visitor.setScript(m_file);
    visitor.visit();

    m_nodeName.clear();
    auto it = visitor.nodeSeq.begin();
    while (it != visitor.nodeSeq.end())
    {
        int seq = it.value();
        QString name = QString::number(seq) + "." + nodeBaseName(it.key());
        m_nodeName[it.key()] = name;
        it++;
    }

    foreachNode([this](JZAbstractNodeItem* node) {
        auto node_item = dynamic_cast<JZVisionNodeItem*>(node);
        node_item->setNodeName(nodeName(node_item->id()));
    });
}

QString JZVisionView::pinName(JZNodeGemo gemo)
{
    if (gemo.isNull())
        return QString();

    JZNode* node = getNode(gemo.nodeId);
    QString ret = nodeName(gemo.nodeId);
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

void JZVisionView::initGraph()
{
    JZNodeAbstractView::initGraph();
    updateNodeName();
}

JZAbstractNodeItem *JZVisionView::createNodeItem(JZNode *node)
{
    auto item = new JZVisionNodeItem(node);
    item->setNodeName(nodeName(node->id()));
    return item;
}

JZAbstractLineItem *JZVisionView::createLineItem(JZNodeGemo from)
{
    return new JZVisionLineItem(from);
}

void JZVisionView::addCreateLinkCommand(const JZVisionParamLink &link, const JZNodeGemo &to_gemo)
{
    if(link.type == JZVisionParamLink::Link_Node && link.path.isEmpty())
    {
        addCreateLineConmmand(link.gemo,to_gemo);
    }
    else
    {
        auto factory = editorNodeFactory();

        JZNodeVisionParam param;
        param.setLink(link);
        addCreateNodeCommand(factory->saveNode(&param),QPoint(0,0));
        if(link.type == JZVisionParamLink::Link_Node)
            addCreateLineConmmand(link.gemo,param.paramInGemo(0));
            
        addCreateLineConmmand(param.paramOutGemo(0),to_gemo);
    }
}

void JZVisionView::addRemoveLinkCommand(int id)
{
    auto line = getLineItem(id);

    auto node = m_file->getNode(line->startTraget().nodeId);
    addRemoveLineCommand(line->id());
    if (node->type() == Node_visionAppParam)
    {
        auto node_item = getNodeItem(node->id());
        removeItem(node_item);
    }
}

void JZVisionView::configNode(JZNode *node)
{
    QByteArray old_buffer = editorNodeFactory()->saveNode(node);
    JZVisionSettingDialog dialog(this);
    dialog.setNode(node);
    if (dialog.exec() != QDialog::Accepted)
    {
        QByteArray tmp_buffer = editorNodeFactory()->saveNode(node);
        if (old_buffer != tmp_buffer)
            node->fromBuffer(old_buffer);
        return;
    }

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
        auto pin_widget = it->pinWidget;
        
        QString pre_value = node->pinValue(pin_id);
        QList<int> in_list = m_file->getConnectInput(node->id(), pin_id);        
        JZVisionParamLink pre_gemo = linkInfo(node->id(), pin_id);
        JZVisionParamLink cur_gemo = pin_widget->linkInfo();
        bool pre_link = !pre_gemo.isNull();
        
        if (pre_link && !cur_gemo.isNull())
        {
            //连接不一样
            if (pre_gemo != cur_gemo)
            {
                addMacro();
                addRemoveLinkCommand(in_list[0]);
                addCreateLinkCommand(pin_widget->linkInfo(), JZNodeGemo(node->id(), pin_id));
            }
        }
        else if (pre_link && cur_gemo.isNull())
        {
            //删除之前连接
            addMacro();
            addRemoveLinkCommand(in_list[0]);
        }
        else if (!pre_link && !cur_gemo.isNull())
        {
            //添加新连接
            addMacro();
            addCreateLinkCommand(pin_widget->linkInfo(), JZNodeGemo(node->id(), pin_id));
        }
        else if (!pre_link && cur_gemo.isNull())
        {
            //这里在后续node change中处理
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
}

void JZVisionView::onContextMenu(const QPoint &pos)
{
    auto item = itemAt(pos);

    QMenu menu(this);
    QAction *actSetting = nullptr;
    QList<QAction*> addList;
    if (item && item->type() == Item_node)
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
        JZVisionNodeItem *node_item = dynamic_cast<JZVisionNodeItem*>(item);
        configNode(node_item->node());
    }
}