#include "JZNodeLayout.h"
#include "JZNodeView.h"

//JZNodeLayoutNode
JZNodeLayoutNode::JZNodeLayoutNode()
{
    nodeId = -1;
    row = 0;
    col = 0;
}

void JZNodeLayoutNode::clear()
{
    input.clear();
    output.clear();
}

//JZNodeLayoutTree
JZNodeLayoutTree::JZNodeLayoutTree()
{
    gap = 20;
}

JZNodeLayoutNode *JZNodeLayoutTree::getNode(int id)
{
    Q_ASSERT(id >= 0);

    auto it = m_nodeMap.find(id);
    if (it != m_nodeMap.end())
        return it->data();

    LayoutNodePtr ptr = LayoutNodePtr(new JZNodeLayoutNode());
    ptr->nodeId = id;
    m_nodeMap[id] = ptr;
    return ptr.data();
}

void JZNodeLayoutTree::make(Graph *graph)
{
    m_nodeMap.clear();
    m_graph = graph;
    if (graph->topolist.size() == 0)
        return;
    
    QList<int> top_list;

    //init
    for (int node_idx = 0; node_idx < graph->topolist.size(); node_idx++)
    {
        auto graph_node = graph->topolist[node_idx];                
        int node_id = graph_node->node->id();

        JZNodeLayoutNode *tree_node = getNode(node_id);
        auto it = graph_node->paramIn.begin();
        while (it != graph_node->paramIn.end())
        {
            auto &gemo_list = it.value();
            for (int i = 0; i < gemo_list.size(); i++)
                tree_node->input << gemo_list[i].nodeId;
            it++;
        }

        it = graph_node->paramOut.begin();
        while (it != graph_node->paramOut.end())
        {
            auto &gemo_list = it.value();
            for (int i = 0; i < gemo_list.size(); i++)
                tree_node->output << gemo_list[i].nodeId;
            it++;
        }
        if (graph_node->paramIn.size() == 0 && graph_node->node->isFlowNode())
            top_list.push_back(node_id);
    }
    
    int row = 0;
    m_posCache.clear();    
    for (int i = 0; i < top_list.size(); i++)
    {        
        calcNodeOuput(top_list[i], row);        
    }
}

int JZNodeLayoutTree::toInt(int row, int col)
{
    return row * 1000000 + col;
}

int JZNodeLayoutTree::calcNodeInput(int id,int row,int &next_row)
{
    if (m_nodeCache.contains(id))
    {
        JZNodeLayoutNode *tree_node = m_nodeCache[id];
        next_row = tree_node->row + 1;
        return tree_node->col;
    }

    int col = 0;
    auto cur_node = getNode(id);
    int pre_row = row;
    for (int flow_idx = 0; flow_idx < cur_node->input.size(); flow_idx++)
    {        
        int pre_col = calcNodeInput(cur_node->input[flow_idx], pre_row, pre_row);
        col = qMax(col, pre_col + 1);
    }
        
    while (m_posCache.contains(toInt(row, col)))
    {
        row++;
    }    
    cur_node->col = col;
    cur_node->row = row;
    m_nodeCache[id] = cur_node;
    m_posCache.insert(toInt(row, col));
    next_row = row + 1;

    return col;
}

int JZNodeLayoutTree::calcNodeOuput(int id, int row)
{        
    if (m_nodeCache.contains(id))
        return m_nodeCache[id]->row + 1;

    int col = 0;
    JZNodeLayoutNode *tree_node = getNode(id);

    int input_next_row = row;
    for (int flow_idx = 0; flow_idx < tree_node->input.size(); flow_idx++)
    {
        int sub_id = tree_node->input[flow_idx];        
        int node_col = calcNodeInput(sub_id, input_next_row, input_next_row);
        col = qMax(col, node_col + 1);        
    }

    while (m_posCache.contains(toInt(row, col)))
    {
        row++;
    }    

    tree_node->row = row;
    tree_node->col = col;        
    m_posCache.insert(toInt(row, col));
    m_nodeCache.insert(id,tree_node);

    int sub_row = row;
    for (int flow_idx = 0; flow_idx < tree_node->output.size(); flow_idx++)
    {
        int sub_id = tree_node->output[flow_idx];
        sub_row = calcNodeOuput(sub_id, sub_row);        
    }

    return row + 1;
}