#include "JZVisionAppLayout.h"

JZVisionAppLayout::JZVisionAppLayout()
{
}

void JZVisionAppLayout::layout(JZProject *project)
{
    auto item_list = project->itemList("./", ProjectItem_any);
    for (int i = 0; i < item_list.size(); i++)
    {
        int item_type = item_list[i]->itemType();
        if (item_type == ProjectItem_scriptItem)
        {
            JZScriptItem *item = (JZScriptItem *)item_list[i];
            layout(item);
        }
    }
}

void JZVisionAppLayout::layout(JZScriptItem *item)
{
    struct NodeInfo
    {
        JZNode *node;
        int x;
    };

    JZVisionView *view = new JZVisionView();
    view->setFile(item);

    int x = 0, y = 0;
    int node_num = 0;

    QList<QList<NodeInfo>> m_nodeStack;

    JZNode *node = item->startNode();
    while(node)
    {    
        node->setPos(QPointF(x, y));

        x += 200;
        if (x >= 1200)
        {
            y += 50;
            x = 0;
        }

        QList<NodeInfo> next_list;
        if (node->subFlowCount() > 0)
        {
            auto sub_list = node->subFlowList();
            for (int i = 0; i < sub_list.size(); i++)
            {
                JZNode* next_node = item->nextFlowNode(node, sub_list[i]);
                if(next_node)
                    next_list << NodeInfo{next_node, x };
            }        
        }
        
        if (node->flowOut() != -1)
        {
            auto next_node = item->nextFlowNode(node, node->flowOut());
            if(next_node)
                next_list << NodeInfo{ next_node, x };
        }
        
        if (next_list.size() == 1)
        {
            node = next_list[0].node;
            x = next_list[0].x;
        }
        else if (next_list.size() > 1)
        {
            node = next_list.front().node;
            x = next_list.front().x;

            next_list.pop_front();
            m_nodeStack.push_back(next_list);
        }
        else
        {
            if (m_nodeStack.size() == 0)
                break;

            auto &back = m_nodeStack.back();
            node = back.front().node;
            x = back.front().x;

            back.pop_front();
            if(back.size() == 0)
                m_nodeStack.pop_back();
        }
    }        
    delete view;
}