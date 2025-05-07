#include "JZOpencvNode.h"
#include "JZNodeCompiler.h"
#include "JZNodeUtils.h"

//JZNodeOpencvInit
JZNodeOpencvInit::JZNodeOpencvInit()
{
	m_type = Node_OpencvInit;
	m_name = "OpencvInit";
}
JZNodeOpencvInit::~JZNodeOpencvInit()
{
}

bool JZNodeOpencvInit::compiler(JZNodeCompiler* c, QString& error)
{
	return true;
}

//JZNodeTemplateMatch
JZNodeTemplateMatch::JZNodeTemplateMatch()
{
	m_type = Node_OpencvTemplate;
	m_name = "OpencvTemplate";
}

JZNodeTemplateMatch::~JZNodeTemplateMatch()
{
}

void JZNodeTemplateMatch::setConfig(const JZTemplateConfig& config)
{
	m_config = config;
}

JZTemplateConfig JZNodeTemplateMatch::config()
{
	return m_config;
}

bool JZNodeTemplateMatch::compiler(JZNodeCompiler* c, QString& error)
{
	if (!c->addFlowInput(m_id, error))
		return false;

	QByteArray buffer = JZNodeUtils::toBuffer(m_config);
	//int id = c->addGetOrInitCall(m_id,"JZTemplate","", buffer);

	return true;
}

void JZNodeTemplateMatch::saveToStream(QDataStream& s) const
{
	JZNode::saveToStream(s);
	s << m_config;
}

void JZNodeTemplateMatch::loadFromStream(QDataStream& s)
{
	JZNode::loadFromStream(s);
	s >> m_config;
}