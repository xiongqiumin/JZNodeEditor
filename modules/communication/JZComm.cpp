#include <QDataStream>
#include "JZComm.h"

//JZCommConfig
JZCommConfig::JZCommConfig()
{
	type = Comm_None;
}

void JZCommConfig::saveToStream(QDataStream& s) const
{
	s << type << name;
}

void JZCommConfig::loadFromStream(QDataStream& s)
{
	s >> type >> name;
}

//JZComm
JZCommObject::JZCommObject(QObject* parent)
	:QObject(parent)
{
}

JZCommObject::~JZCommObject()
{

}

void JZCommObject::setConfig(JZCommConfigPtr config)
{
	m_config = config;
}