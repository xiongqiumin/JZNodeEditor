#include <QVBoxLayout>
#include "JZNodeViewPanel.h"
#include "JZScriptItem.h"
#include "JZNodeView.h"
#include "JZNodeValue.h"
#include "3rd/jzprofiler/JZTx.h"

// JZNodeViewPanel
JZNodeViewPanel::JZNodeViewPanel(QWidget *widget)
    : QWidget(widget)
{        
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(1);
    m_file = nullptr;

    m_lineSearch = new QLineEdit();
    layout->addWidget(m_lineSearch);
    connect(m_lineSearch,&QLineEdit::returnPressed,this,&JZNodeViewPanel::onSearch);

    m_tree = new QTreeWidget();
    m_tree->setColumnCount(1);
    m_tree->setHeaderHidden(true);

    connect(m_tree, &QTreeWidget::itemClicked, this, &JZNodeViewPanel::onTreeItemClicked);
    setContextMenuPolicy(Qt::CustomContextMenu);

    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tree, &QTreeWidget::customContextMenuRequested, this, &JZNodeViewPanel::onContextMenu);

    layout->addWidget(m_tree);
    setLayout(layout);    
}

JZNodeViewPanel::~JZNodeViewPanel()
{
}

QString JZNodeViewPanel::inputText(JZNode *to_node, int pin_id)
{
    QList<int> in_list = m_file->getConnectInput(to_node->id(), pin_id);
    if (in_list.size() == 0)
        return to_node->pinValue(pin_id);
    else if (in_list.size() > 1)
        return "...";

    auto line = m_file->getConnect(in_list[0]);
    JZNode *node = m_file->getNode(line->from.nodeId);
    if (node->isFlowNode())
        return "...";

    pin_id = line->from.pinId;
    if (node->type() == Node_param)    
        return node->name();
    else if (node->type() >= Node_add && node->type() <= Node_or)
    {
        QStringList in_text;
        auto in_list = node->paramInList();
        for (int i = 0; i < in_list.size(); i++)
            in_text << inputText(node, in_list[i]);

        auto node_op = dynamic_cast<JZNodeOperator*>(node);
        QString op = JZNodeType::opName(node_op->op()) + " ";
        return in_text.join(op);        
    }
    else if (node->type() == Node_function)
    {
        QStringList in_text;
        auto in_list = node->paramInList();
        for (int i = 0; i < in_list.size(); i++)
            in_text << inputText(node, in_list[i]);

        return node->name() + "(" + in_text.join(", ") + ")";
    }
    else if (node->type() == Node_literal)
    {
        return node->paramOutValue(0);
    }

    return node->name();
}

QString JZNodeViewPanel::nodeText(JZNode *node)
{
    if (node->type() == Node_setParam)
    {
        return node->paramInValue(0) + " = " + inputText(node, node->paramIn(1));
    }
    else if (node->type() == Node_for)
    {
        JZNodeFor *for_node = dynamic_cast<JZNodeFor*>(node);
        return "for()";
    }    
    else if (node->type() == Node_while)
    {
        return "while(" + inputText(node, node->paramIn(0)) + ")";
    }
    else if (node->type() == Node_function)
    {
        QStringList in_text;
        auto in_list = node->paramInList();
        for (int i = 0; i < in_list.size(); i++)
            in_text << inputText(node, in_list[i]);

        return node->name() + "(" + in_text.join(", ") + ")";
    }

    return node->name();
}

void JZNodeViewPanel::setView(JZNodeView *view)
{
    m_view = view;
}

void JZNodeViewPanel::updateFlow(JZScriptItem *file)
{        
    m_file = file;
    m_tree->clear();
    m_lcaMap.clear();

    auto start = file->getNode(0);
    createNode(m_tree->invisibleRootItem(), start, nullptr);
    m_tree->expandAll();
}

JZNode *JZNodeViewPanel::nextNode(JZNode *node, int pin_id)
{
    auto lines = m_file->getConnectPin(node->id(), pin_id);
    if (lines.size() == 0)
        return nullptr;

    auto line = m_file->getConnect(lines[0]);
    return m_file->getNode(line->to.nodeId);
}

void JZNodeViewPanel::addFlow(QTreeWidgetItem *parent, QTreeWidgetItem *lca_parent, JZNode *node, QList<int> flow_list)
{
    JZNode *lca_node = nullptr;
    if(flow_list.size() > 1)
        lca_node = findLCA(node, flow_list);
    for (int i = 0; i < flow_list.size(); i++)
    {
        auto lines = m_file->getConnectPin(node->id(), flow_list[i]);
        if (lines.size() == 0)
            continue;

        auto line = m_file->getConnect(lines[0]);
        auto sub_node = m_file->getNode(line->to.nodeId);

        auto flow_root = parent;
        if (flow_list.size() > 1)
        {
            auto flow_pin = m_file->getNode(line->from.nodeId)->pinName(line->from.pinId);
            flow_root = new QTreeWidgetItem();
            flow_root->setText(0, flow_pin);
            parent->addChild(flow_root);
        }
        createNode(flow_root, m_file->getNode(line->to.nodeId), lca_node);
    }
    if(lca_node)
        createNode(lca_parent, lca_node, nullptr);
}

JZNode *JZNodeViewPanel::findLCA(JZNode *node, QList<int> flow_list)
{
    if (m_lcaMap.contains(node))
        return m_lcaMap[node];

    int full = 0;
    QMap<JZNode*, int> path;
    for (int i = 0; i < flow_list.size(); i++)
        full |= (1 << i);

    JZNode *ret = nullptr;
    for (int i = 0; i < flow_list.size(); i++)
    {
        JZNode *next_node = nextNode(node, flow_list[i]);
        while (next_node)
        {
            path[next_node] |= (1 << i);
            if (path[next_node] == full)
            {
                ret = next_node;
                goto lca_end;
            }

            auto next_flow_list = next_node->flowOutList();
            if (next_flow_list.size() == 0)
                next_node = nullptr;
            else if (next_flow_list.size() == 1)
                next_node = nextNode(next_node, next_flow_list[0]);
            else if (next_flow_list.size() > 1)
                next_node = findLCA(next_node, next_flow_list);
        }                               
    }

lca_end:
    m_lcaMap[node] = ret;
    return ret;
}

void JZNodeViewPanel::createNode(QTreeWidgetItem *parent, JZNode *node, JZNode *lca_node)
{        
    while (node)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, nodeText(node));
        item->setData(0, Qt::UserRole, node->id());
        parent->addChild(item);

        auto sub_list = node->subFlowList();        
        addFlow(item, item, node, sub_list);

        auto flow_list = node->flowOutList();
        if (flow_list.size() == 0)
            break;
        else if(flow_list.size() == 1)
        {
            auto lines = m_file->getConnectPin(node->id(), flow_list[0]);
            if (lines.size() == 0)
                break;

            auto line = m_file->getConnect(lines[0]);
            node = m_file->getNode(line->to.nodeId);
            if (node == lca_node)
                break;
        }
        else
        {
            addFlow(item, parent, node, flow_list);
            break;
        }
    }
}

void JZNodeViewPanel::onSearch()
{

}

void JZNodeViewPanel::onTreeItemClicked(QTreeWidgetItem *current, int col)
{
    QVariant v = current->data(0, Qt::UserRole);
    if (v.isNull())
        return;

    int node_id = v.toInt();
    m_view->selectNode(node_id);
}

void JZNodeViewPanel::onContextMenu(const QPoint &pos)
{

}
