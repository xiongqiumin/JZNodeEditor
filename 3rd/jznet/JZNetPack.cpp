#include "JZNetPack.h"

//JZNetPack
JZNetPack::JZNetPack()	
{		
	m_seq == -1;
}

JZNetPack::~JZNetPack()
{
}

int JZNetPack::seq()
{
	return m_seq;
}

void JZNetPack::setSeq(int seq)
{
	m_seq = seq;
}

//NetPackVariant
NetPackVariant::NetPackVariant()
{

}

NetPackVariant::~NetPackVariant()
{

}
		
int NetPackVariant::type() const
{
	return NetPack_variant;
}

void NetPackVariant::saveToStream(QDataStream &s) const
{
	s << params;
}

void NetPackVariant::loadFromStream(QDataStream &s)
{
	s >> params;
}

//NetPackByteArray
NetPackByteArray::NetPackByteArray()
{

}
	
NetPackByteArray::~NetPackByteArray()
{

}
		
int NetPackByteArray::type() const
{
	return NetPack_byteArray;
}

void NetPackByteArray::saveToStream(QDataStream &s) const
{
	s << buffer;
}

void NetPackByteArray::loadFromStream(QDataStream &s)
{
	s >> buffer;
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

	Q_ASSERT(0);
	return nullptr;
}

void JZNetPackManager::init()
{
	registPack(NetPack_variant,createNetPackFunc<NetPackByteArray>);
	registPack(NetPack_byteArray,createNetPackFunc<NetPackByteArray>);
}

void JZNetPackManager::registPack(int type,CreatePackFunc func)
{
	Q_ASSERT(!m_packFactory.contains(type));
	m_packFactory.insert(type,func);
}