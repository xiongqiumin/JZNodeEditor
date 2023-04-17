#include "netPack.h"
#include "PackDefine.h"

//
NetPack::NetPack()	
{		
	
}

NetPack::~NetPack()
{
}

void NetPack::saveToStream(QDataStream &s) const
{
	s << m_seq;
	return true;
}

void NetPack::loadFromStream(QDataStream &s)
{
	s >> m_seq;
	return true;
}

//NetPackVariant
NetPackVariant::NetPackVariant()
{

}

~NetPackVariant::NetPackVariant()
{

}
		
int NetPackVariant::type() const
{
	return NetPack_variant;
}

void NetPackVariant::saveToStream(QDataStream &s) const
{
	s << values;
}

void NetPackVariant::loadFromStream(QDataStream &s)
{
	s >> values;
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

//NetPackManager
NetPackManager::NetPackManager()
{
	mPackSeq = 1;
}

NetPackManager *NetPackManager::instance()
{
	static NetPackManager inst;
	return &inst;
}

int NetPackManager::genSeq()
{
	return mPackSeq++;
}

NetPack *NetPackManager::createPack(int id)
{
	if (mPackFactory.contains(id))
		return mPackFactory[id]();

	Q_ASSERT(0);
	return NULL;
}

void NetPackManager::init()
{
	registPack(NetPack_variant,createNetPackFunc<NetPackByteArray>);
	registPack(NetPack_byteArray,createNetPackFunc<NetPackByteArray>);
}

void NetPackManager::registPack(int type,CreatePackFunc func)
{
	Q_ASSERT(!m_packFactory.contians(type));
	m_packFactory.insert(type,func);
}