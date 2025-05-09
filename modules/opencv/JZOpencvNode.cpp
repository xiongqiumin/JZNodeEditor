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

	int in = addParamIn("image");
	setPinType(in, { "Mat" });
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

	QString obj_name = m_file->name() + "_" + QString::number(m_id);

	QByteArray buffer = JZNodeUtils::toBuffer(m_config);
	
	int obj_id;
	c->addGetOrInit(obj_name,"JZTemplate", buffer, obj_id);

	QList<JZNodeIRParam> in,out;
	in << irId(obj_id) << irId(c->paramId(m_id,paramIn(0)));
	c->addCall("JZOpencvTemplateMatch",in,out);

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