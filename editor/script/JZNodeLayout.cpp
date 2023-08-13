#include "JZNodeLayout.h"
#include "JZNodeView.h"

//JZNodeLayoutNode
JZNodeLayoutNode::JZNodeLayoutNode()
{
    nodeId = -1;
    isFlow = false;
    row = -1;
    col = -1;
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
    max_row = 0;
    max_col = 0;
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
    m_posCache.clear();    
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
        tree_node->isFlow = graph_node->node->isFlowNode();

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
    
    //先计算固定深度
    for (int node_idx = 0; node_idx < graph->topolist.size(); node_idx++)
        calcCol(graph->topolist[node_idx]->node->id());

    //压缩列数
    bool compress = false;
    do
    {
        compress = false;
        for (int node_idx = 0; node_idx < graph->topolist.size(); node_idx++)
        {
            int node_id = graph->topolist[node_idx]->node->id();
            auto tree_node = getNode(node_id);
            if (tree_node->isFlow)
                continue;

            int min_col = 0;
            for (int i = 0; i < tree_node->output.size(); i++)
            {
                auto sub = getNode(tree_node->output[i]);
                if (i == 0)
                    min_col = sub->col;
                else
                    min_col = qMin(min_col, sub->col);
            }
            if (tree_node->col < min_col - 1)
            {
                tree_node->col = min_col - 1;
                compress = true;
            }
        }
    }while (compress);    
        
    //calc 
    int row = 0;
    m_posCache.clear();    
    for (int i = 0; i < top_list.size(); i++)
    {        
        calcNodeOuput(top_list[i], row);        
    }

    //计算max
    auto it = m_nodeMap.begin();
    while (it != m_nodeMap.end())
    {
        int node_id = it.key();
        auto tree_node = it->data();
        max_row = qMax(max_row, tree_node->row);
        max_col = qMax(max_col, tree_node->col);
        it++;
    }

    //压缩行数
    compress = false;
    do
    {
        compress = false;
        for (int node_idx = 0; node_idx < graph->topolist.size(); node_idx++)
        {
            int node_id = graph->topolist[node_idx]->node->id();
            auto tree_node = getNode(node_id);
            if (tree_node->isFlow || tree_node->row <= 0)
                continue;

            int key = toInt(tree_node->row - 1, tree_node->col);
            if (!m_posCache.contains(key))
            {
                m_posCache.insert(key);
                m_posCache.remove(toInt(tree_node->row, tree_node->col));
                tree_node->row--;
                compress = true;
            }
        }
    } while (compress);

    for (int node_idx = 0; node_idx < graph->topolist.size(); node_idx++)
    {
        int node_id = graph->topolist[node_idx]->node->id();
        auto tree_node = getNode(node_id);
        if (tree_node->row == -1)
            tree_node->row = max_row + 1;
    }
    max_row++;
}

int JZNodeLayoutTree::toInt(int row, int col)
{
    return row * 1000000 + col;
}

int JZNodeLayoutTree::calcCol(int id)
{
    int col = 0;
    JZNodeLayoutNode *tree_node = getNode(id);
    if (tree_node->col != -1)
        return tree_node->col;

    for (int flow_idx = 0; flow_idx < tree_node->input.size(); flow_idx++)
    {
        int sub_id = tree_node->input[flow_idx];
        int node_col = calcCol(sub_id);
        col = qMax(col, node_col + 1);
    }

    tree_node->col = col;
    return col;
}

void JZNodeLayoutTree::calcNodeInput(int id,int row,int &next_row)
{
    if (m_nodeCache.contains(id))
    {
        JZNodeLayoutNode *tree_node = m_nodeCache[id];
        next_row = tree_node->row + 1;
        return;
    }

    int col = 0;
    auto cur_node = getNode(id);
    int pre_row = row;
    for (int flow_idx = 0; flow_idx < cur_node->input.size(); flow_idx++)
    {        
        calcNodeInput(cur_node->input[flow_idx], pre_row, pre_row);        
    }
        
    while (m_posCache.contains(toInt(row, col)))
    {
        row++;
    }        
    cur_node->row = row;
    m_nodeCache[id] = cur_node;
    m_posCache.insert(toInt(row, col));
    next_row = row + 1;
}

int JZNodeLayoutTree::calcNodeOuput(int id, int row)
{        
    if (m_nodeCache.contains(id))
        return m_nodeCache[id]->row + 1;
    
    JZNodeLayoutNode *tree_node = getNode(id);
    int col = tree_node->col;

    int input_next_row = row;
    for (int flow_idx = 0; flow_idx < tree_node->input.size(); flow_idx++)
    {
        int sub_id = tree_node->input[flow_idx];        
        calcNodeInput(sub_id, input_next_row, input_next_row);               
    }

    while (m_posCache.contains(toInt(row, col)))
    {
        row++;
    }    

    tree_node->row = row;       
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