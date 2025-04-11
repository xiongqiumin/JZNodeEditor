#include "JZScriptItemHelp.h"
#include "JZNodeFactory.h"

JZScriptItemHelp::JZScriptItemHelp(JZScriptItem* item)
{
	m_script = item;
}

JZScriptItemHelp::~JZScriptItemHelp()
{
}

void JZScriptItemHelp::addFunction(QString function, QStringList params)
{
}

void JZScriptItemHelp::addSet(QString dst, QString src)
{
}

void JZScriptItemHelp::addFor(QString start, int op, QString end)
{
}

void JZScriptItemHelp::addIf(QStringList condList)
{
}

void JZScriptItemHelp::addSwitch(QStringList condList)
{
}

void JZScriptItemHelp::addExpr(QString dst, int op, QString src1, QString src2)
{
	JZNode *node_op = JZNodeFactory::instance()->createNode(op);

	JZNode* node_src1 = getParamNode(src1);
	JZNode* node_src2 = getParamNode(src2);
	JZNode* node_set = getSetParamNode(dst);

	m_script->addNode(node_op);
	m_script->addNode(node_src1);
	m_script->addNode(node_src2);
	m_script->addNode(node_set);

	m_script->addConnect(node_src1->paramOutGemo(0), node_op->paramInGemo(0));
	m_script->addConnect(node_src2->paramOutGemo(0), node_op->paramInGemo(1));

	
	m_script->addConnect(m_lastFlow->flowOutGemo(), node_set->flowInGemo());

	m_script->addConnect(m_lastFlow->flowOutGemo(), node_set->flowInGemo());
}