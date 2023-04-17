#ifndef JZNET_PACK_H_
#define JZNET_PACK_H_

#include <QByteArray>
#include <QDataStream>
#include <QMap>
#include <QVariant>
using namespace std;

enum{
	NetPack_none,
	NetPack_variant,
	NetPack_byteArray,
};

class NetPack
{
public:
	NetPack();
	virtual ~NetPack();
	
    virtual int type() const = 0 ;
	virtual void saveToStream(QDataStream &s) const;
	virtual void loadFromStream(QDataStream &s);

	int seq();
	void setSeq(int seq);

private:
	int m_seq;	
};

//NetPackVariant
class NetPackVariant : public NetPack
{
public:
	NetPackVariant();
	virtual ~NetPackVariant();
		
    virtual int type() const;
	virtual void saveToStream(QDataStream &s) const;
	virtual void loadFromStream(QDataStream &s);	

	QVariantMap params;	
};

//NetPackByteArray
class NetPackByteArray : public NetPack
{
public:
	NetPackByteArray();
	virtual ~NetPackByteArray();
		
    virtual int type() const;
	virtual void saveToStream(QDataStream &s) const;
	virtual void loadFromStream(QDataStream &s);

	QByteArray buffer;	
};

//NetPackManager
typedef NetPack *(*CreatePackFunc)();
class NetPackManager
{
public:
	static NetPackManager *instance();
	NetPackManager();

	void init();	
	void registPack(int type,CreatePackFunc func);
	NetPack *createPack(int id);

protected:
	QMap<int, CreatePackFunc> m_packFactory;	
};
template<class T> 
NetPack *createNetPackFunc(){ return new T();}

#endif