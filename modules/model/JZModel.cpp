#include <QDataStream>
#include "JZModel.h"

//JZModelConfig
JZModelConfig::JZModelConfig()
{
	type = Model_None;
}

void JZModelConfig::saveToStream(QDataStream& s) const
{
	s << name;
	s << type;
}

void JZModelConfig::loadFromStream(QDataStream& s)
{
	s >> name;
	s >> type;
}

//JZModel
JZModel::JZModel()
{
}

JZModel::~JZModel()
{
}

QString JZModel::name() const
{
    return m_config->name;
}

const JZModelConfigEnum &JZModel::config()
{
    return m_config;
}

void JZModel::setConfig(JZModelConfigEnum config)
{
	m_config = config;
}