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

//JZNetPackByteArray
JZNetPackByteArray::JZNetPackByteArray()
{

}
	
JZNetPackByteArray::~JZNetPackByteArray()
{

}
		
int JZNetPackByteArray::type() const
{
	return NetPack_byteArray;
}

void JZNetPackByteArray::saveToStream(QDataStream &s) const
{
	s << buffer;
}

void JZNetPackByteArray::loadFromStream(QDataStream &s)
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
    registPack(NetPack_variant,createNetPackFunc<JZNetPackVariant>);
    registPack(NetPack_byteArray,createNetPackFunc<JZNetPackByteArray>);
}

void JZNetPackManager::registPack(int type,CreatePackFunc func)
{
	Q_ASSERT(!m_packFactory.contains(type));
	m_packFactory.insert(type,func);
}
