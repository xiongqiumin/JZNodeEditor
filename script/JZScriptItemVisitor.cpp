#include <QSet>
#include "JZScriptItemVisitor.h"

JZScriptItemVisitor::JZScriptItemVisitor(JZScriptItem *item)
{
    m_script = item;
}

JZScriptItemVisitor::~JZScriptItemVisitor()
{
}

void JZScriptItemVisitor::setScript(JZScriptItem *item)
{
    m_script = item;
}

void JZScriptItemVisitor::visitor()
{    
    m_hasVistorNode.clear();

    auto start = m_script->startNode();
    visitorNode(start);
}

void JZScriptItemVisitor::visitorNode(JZNode *node)
{
    if (m_hasVistorNode.contains(node))
        return;

    m_hasVistorNode.push_back(node);
    while(node)
    {
        QList<JZNode*> input_list = dataInputNode(node);
        for(int i = 0; i < input_list.size(); i++)
            visitorNode(input_list[i]);

        auto sub_list = node->subFlowList();
        for(int i = 0; i < sub_list.size(); i++)
        {
            auto sub_node = nextFlowNode(node,sub_list[i]);
            visitorNode(sub_node);
        }

        visitorSelf(node);

        if(node->flowOut() != -1)
            node = nextFlowNode(node,node->flowOut());
        else
            node = nullptr;
    }
}

void JZScriptItemVisitor::visitorSelf(JZNode *node)
{

}

JZNode *JZScriptItemVisitor::prevFlowNode(JZNode *node)
{
    if (node->flowInCount() == 0)
        return nullptr;

    auto input = m_script->getConnectInput(node->id(), node->flowIn());
    if (input.size() == 0)
        return nullptr;

    auto c = m_script->getConnect(input[0]);
    return m_script->getNode(c->from.nodeId);
}

JZNode *JZScriptItemVisitor::nextFlowNode(JZNode *node,int flow_out)
{
    return m_script->nextFlowNode(node,flow_out);
}

QList<JZNode*> JZScriptItemVisitor::dataInputNodeRecursively(JZNode *node)
{
    QList<JZNode*> all_in_list = dataInputNode(node);
    QList<JZNode*> cur_list = all_in_list;
    QList<JZNode*> next_list;
    while(cur_list.size() != 0)
    {
        for(int i = 0; i < cur_list.size(); i++)
        {
            QList<JZNode*> tmp_in_list = dataInputNode(cur_list[i]);
            for(auto tmp : tmp_in_list)
            {
                if(!all_in_list.contains(tmp))
                    next_list << tmp;
            }
        }

        all_in_list << next_list;
        cur_list = next_list;
        next_list.clear();
    }

    return all_in_list;
}

QList<JZNode*> JZScriptItemVisitor::dataInputNode(JZNode *node)
{
    QSet<JZNode*> from_nodes;

    auto in_list = node->paramInList();
    for(int i = 0; i < in_list.size(); i++)
    {
        QList<int> in_line_list = m_script->getConnectInput(node->id(), in_list[i]);
        for(int line_idx = 0; line_idx < in_line_list.size(); line_idx++)
        {
            auto line = m_script->getConnect(in_line_list[line_idx]);
            auto from_node = m_script->getNode(line->from.nodeId);
            from_nodes << from_node;
        }
    }
    return from_nodes.toList();
}

QList<JZNode*> JZScriptItemVisitor::dataInputNode(JZNode* node, int pin_id)
{
    QSet<JZNode*> from_nodes;
    QList<int> in_line_list = m_script->getConnectInput(node->id(), pin_id);
    for (int line_idx = 0; line_idx < in_line_list.size(); line_idx++)
    {
        auto line = m_script->getConnect(in_line_list[line_idx]);
        auto from_node = m_script->getNode(line->from.nodeId);
        from_nodes << from_node;
    }
    return from_nodes.toList();
}

QList<JZNodePin*> JZScriptItemVisitor::inputPin(int node_id, int pin_id)
{
    QList<JZNodePin*> ret;
    QList<int> in_line_list = m_script->getConnectInput(node_id, pin_id);
    for (int line_idx = 0; line_idx < in_line_list.size(); line_idx++)
    {
        auto line = m_script->getConnect(in_line_list[line_idx]);
        ret << m_script->getNode(line->from.nodeId)->pin(line->from.pinId);        
    }
    return ret;
}