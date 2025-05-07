#include "JZNodeFunctionItem.h"
#include "JZNodeFunction.h"

//JZNodeFunctionItem
JZNodeFunctionItem::JZNodeFunctionItem(JZNode *node)
    :JZNodeGraphItem(node)
{
}

void JZNodeFunctionItem::updatePin()
{
    JZNodeGraphItem::updatePin();

    JZNodeFunction *node_func = dynamic_cast<JZNodeFunction*>(m_node);
    m_title = node_func->function();

    auto env = m_node->environment();
    auto func_inst = env->functionManager();
    auto meta = func_inst->function(m_title);
    if (meta && meta->isMemberFunction() && !node_func->isDirectCall())
    {
        QString v = node_func->variable();
        if (v.isEmpty())
        {
            if (!node_func->isMemberCall())
                return;

            v = "this";
        }

        QString name = v + "." + meta->name;
        m_title = name;
    }
}