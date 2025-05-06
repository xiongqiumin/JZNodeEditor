#include "JZOpencvNode.h"

//JZTemplateConfig
JZTemplateConfig::JZTemplateConfig()
{
	confidence = 0.95;
}

QDataStream& operator<<(QDataStream& s, const JZTemplateConfig& config)
{
	s << config.templatePath;
	return s;
}

QDataStream& operator>>(QDataStream& s, JZTemplateConfig& config)
{
	s >> config.templatePath;
	return s;
}

//JZNodeOpencvInit
JZNodeOpencvInit::JZNodeOpencvInit()
{
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