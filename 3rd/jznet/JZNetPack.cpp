#include "JZNetPack.h"

//JZNetPack
JZNetPack::JZNetPack()	
{		
    m_seq = -1;
}

JZNetPack::~JZNetPack()
{
}

int JZNetPack::seq() const
{
	return m_seq;
}

void JZNetPack::setSeq(int seq)
{
	m_seq = seq;
}

//JZNetPackVariant
JZNetPackVariant::JZNetPackVariant()
{

}

JZNetPackVariant::~JZNetPackVariant()
{

}
		
int JZNetPackVariant::type() const
{
	return NetPack_variant;
}

void JZNetPackVariant::saveToStream(QDataStream &s) const
{
	s << params;
}

void JZNetPackVariant::loadFromStream(QDataStream &s)
{
	s >> params;
}

//JZNetPackManager
JZNetPackManager *JZNetPackManager::instance()
{
	static JZNetPackManager inst;
	return &inst;
}

JZNetPackManager::JZNetPackManager()
{	
}

JZNetPack *JZNetPackManager::createPack(int id)
{
	if (m_packFactory.contains(id))
		return m_packFactory[id]();
	
	return nullptr;
}

void JZNetPackManager::init()
{
    registPack(NetPack_variant, JZNetPackCreate<JZNetPackVariant>);
}

void JZNetPackManager::registPack(int type,CreatePackFunc func)
{
	Q_ASSERT(!m_packFactory.contains(type));
	m_packFactory.insert(type,func);
}
