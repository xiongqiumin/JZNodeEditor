#include "JZScriptItemHelp.h"
#include "JZNodeFactory.h"
#include "JZNodeEvent.h"

JZScriptItemHelp::Flow::Flow()
{
    index = 0;
}

JZScriptItemHelp::JZScriptItemHelp()
{
    m_script = nullptr;	
}

JZScriptItemHelp::~JZScriptItemHelp()
{
}

void JZScriptItemHelp::init(JZScriptItem* item)
{
    m_script = item;
    m_flowStack.clear();

    Flow f;
    f.nodes.push_back(item->startNode());
}

void JZScriptItemHelp::addCall(QString function, QStringList params)
{
}

void JZScriptItemHelp::addSet(QString dst, QString expr)
{
    JZNode *node_expr = addExpr(expr);
    
    JZNode* node_set = getSetParamNode(dst);
    m_script->addConnect(node_expr->paramOutGemo(0), node_set->paramInGemo(0));
    nextFlow(node_set);
}

void JZScriptItemHelp::addFor(QString start, int op, QString end)
{
    JZNodeFor *node_for = new JZNodeFor();
    m_script->addNode(node_for);
    nextFlow(node_for);
    
    JZNode *node_start = addExpr(start);
    JZNode *node_end = addExpr(end);
    node_for->setOp(op);
    m_script->addConnect(node_start->paramOutGemo(0), node_for->paramInGemo(0));
    m_script->addConnect(node_end->paramOutGemo(0), node_for->paramInGemo(0));

    pushSubFlow(1);
}

void JZScriptItemHelp::addWhile(QString cond)
{
    JZNodeWhile *node_while = new JZNodeWhile();
    m_script->addNode(node_while);
    nextFlow(node_while);

    JZNode *node_expr = addExpr(cond);
    m_script->addConnect(node_expr->paramOutGemo(0), node_while->paramInGemo(0));
    pushSubFlow(1);
}

void JZScriptItemHelp::addIf(QStringList condList)
{
    JZNodeIf *node_if = (JZNodeIf*)JZNodeFactory::instance()->createNode(Node_if);
    m_script->addNode(node_if);
    nextFlow(node_if);

    for (int i = 0; i < condList.size(); i++)
    {
        if (condList[i] != "else")
        {
            JZNode *node_expr = addExpr(condList[i]);
            if (i > 0)
                node_if->addCondPin();

            m_script->addConnect(node_expr->paramOutGemo(0), node_if->paramInGemo(i));
        }
        else
        {
            Q_ASSERT(i == condList.size());
            node_if->addElsePin();            
        }
    }
    pushSubFlow(condList.size());
}

void JZScriptItemHelp::addSwitch(QStringList cond_list,bool has_default)
{
    JZNodeSwitch *node_switch = (JZNodeSwitch*)JZNodeFactory::instance()->createNode(Node_if);
    m_script->addNode(node_switch);
    nextFlow(node_switch);

    node_switch->clearCaseAndDefault();
    int sub_count = cond_list.size();
    for (int i = 0; i < cond_list.size(); i++)
    {
        node_switch->addCase();
        node_switch->setParamOutValue(node_switch->paramOut(0), cond_list[i]);
    }

    if (has_default)
    {
        node_switch->addDefault();
        sub_count++;
    }

    pushSubFlow(sub_count);
}

void JZScriptItemHelp::addOperator(QString dst, int op, QString src1, QString src2)
{
	JZNode *node_op = JZNodeFactory::instance()->createNode(op);
    m_script->addNode(node_op);

	JZNode* node_src1 = getParamNode(src1);
	JZNode* node_src2 = getParamNode(src2);	

	m_script->addConnect(node_src1->paramOutGemo(0), node_op->paramInGemo(0));
	m_script->addConnect(node_src2->paramOutGemo(0), node_op->paramInGemo(1));	

    JZNode* node_set = getSetParamNode(dst);
    m_script->addConnect(node_op->paramOutGemo(0), node_set->paramInGemo(0));
    nextFlow(node_set);
}

void JZScriptItemHelp::pushSubFlow(int sub_count)
{
    JZNode *last = lastFlow();

    Flow f;
    for (int i = 0; i < sub_count; i++)
    {
        JZNodeNop *nop = new JZNodeNop();
        m_script->addNode(nop);
        m_script->addConnect(last->subFlowOutGemo(i), nop->flowInGemo());        
    }
    m_flowStack.push_back(f);
}

void JZScriptItemHelp::popSubFlow()
{
    m_flowStack.pop_back();
}

void JZScriptItemHelp::switcbFlow(int idx)
{
    auto &f = m_flowStack.back();
    f.index = idx;
}

JZNode *JZScriptItemHelp::lastFlow()
{
    auto &f = m_flowStack.back();
    return f.nodes[f.index];
}

void JZScriptItemHelp::nextFlow(JZNode *node)
{
    m_script->addConnect(lastFlow()->flowOutGemo(), node->flowInGemo());
    
    Flow f;
    f.nodes[0] = node;
    f.index = 0;
    m_flowStack.back() = f;
}

JZNode* JZScriptItemHelp::addExpr(QString expr)
{
    return nullptr;
}

JZNode* JZScriptItemHelp::getParamNode(QString param)
{
    return nullptr;
}

JZNode* JZScriptItemHelp::getSetParamNode(QString param)
{
    return nullptr;
}