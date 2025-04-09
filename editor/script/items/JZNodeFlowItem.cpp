#include "JZNodeFlowItem.h"
#include "JZNode.h"

JZNodeForItem::JZNodeForItem(JZNode *node)
{
}

JZNodeForItem::~JZNodeForItem()
{
}

void JZNodeForItem::onCompareOpChanged()
{
    QByteArray oldValue = saveNode(m_node);

    JZNodeFor *node_for = (JZNodeFor*)m_node;
    node_for->setOp(op);
    notifyPropChanged(m_node,oldValue);
}