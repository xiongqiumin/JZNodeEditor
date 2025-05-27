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

void JZModel::setConfig(JZModelConfigEnum config)
{
	m_config = config;
}