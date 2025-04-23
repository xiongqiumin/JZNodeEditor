#include <QSet>
#include "JZScriptItemVisitor.h"

JZScriptItemVistor::JZScriptItemVistor()
{
    m_script = nullptr;
}

JZScriptItemVistor::~JZScriptItemVistor()
{
}

void JZScriptItemVistor::visitorScript(const JZScriptItem *item)
{
    m_script = item;

    auto start = m_script->startNode();
    visitor(start);
}

void JZScriptItemVistor::visitor(const JZNode *node)
{
    while(node)
    {
        QList<const JZNode*> input_list = dataInputNode(node);
        for(int i = 0; i < input_list.size(); i++)
            visitor(input_list[i]);

        auto sub_list = node->subFlowList();
        for(int i = 0; i < sub_list.size(); i++)
        {
            auto sub_node = nextFlowNode(node,sub_list[i]);
            visitor(sub_node);
        }

        visitorSelf(node);

        if(node->flowOut() != -1)
            node = nextFlowNode(node,node->flowOut());
        else
            node = nullptr;
    }
}

void JZScriptItemVistor::visitorSelf(const JZNode *node)
{

}

const JZNode *JZScriptItemVistor::flowInputNode(const JZNode *node,int id)
{
    QList<const JZNode*> pre_list = getPinNode(node,node->flowIn());
    if(pre_list.size() == 0)
        return nullptr;

    return pre_list[0];
}

const JZNode *JZScriptItemVistor::nextFlowNode(const JZNode *node,int flow_out)
{
    Q_ASSERT(node->pin(flow_out)->isFlow() || node->pin(flow_out)->isSubFlow());

    QList<int> next_flow = m_script->getConnectPin(node->id(), flow_out);
    if (next_flow.size() > 0)
    {
        Q_ASSERT(next_flow.size() == 1);
        auto line = m_script->getConnect(next_flow[0]);
        return m_script->getNode(line->to.nodeId);
    }
    else
    {
        return nullptr;
    }
}

QList<const JZNode*> JZScriptItemVistor::allDataInputNode(const JZNode *node)
{
    QList<const JZNode*> all_in_list = dataInputNode(node);
    QList<const JZNode*> cur_list = all_in_list;
    QList<const JZNode*> next_list;
    while(cur_list.size() != 0)
    {
        for(int i = 0; i < cur_list.size(); i++)
        {
            QList<const JZNode*> tmp_in_list = dataInputNode(cur_list[i]);
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

QList<const JZNode*> JZScriptItemVistor::getPinNode(const JZNode *node,int pin)
{
    QSet<const JZNode*> from_nodes;

    QList<int> in_line_list = m_script->getConnectPin(node->id(), pin);
    for(int line_idx = 0; line_idx < in_line_list.size(); line_idx++)
    {
        auto line = m_script->getConnect(in_line_list[line_idx]);
        auto from_node = m_script->getNode(line->from.nodeId);
        from_nodes << from_node;
    }

    return from_nodes.values();
}

QList<const JZNode*> JZScriptItemVistor::dataInputNode(const JZNode *node)
{
    QSet<const JZNode*> from_nodes;

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