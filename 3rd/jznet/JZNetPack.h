#ifndef JZNET_PACK_H_
#define JZNET_PACK_H_

#include <QByteArray>
#include <QDataStream>
#include <QMap>
#include <QVariant>
#include <QSharedPointer>
using namespace std;

enum{
	NetPack_none,
	NetPack_variant,
	NetPack_user = 0x100,
};

class JZNetPack
{
public:
	JZNetPack();
	virtual ~JZNetPack();
	
    virtual int type() const = 0;
	virtual void saveToStream(QDataStream &s) const = 0;
	virtual void loadFromStream(QDataStream &s) = 0;

    int seq() const;
	void setSeq(int seq);

private:
	int m_seq;	
};
typedef QSharedPointer<JZNetPack> JZNetPackPtr;

//JZNetPackVariant
template<class T>
QVariant netDataPack(const T &param)
{
    QByteArray buffer;
    QDataStream s(&buffer, QIODevice::WriteOnly);
    s << param;
    return buffer;
}

template<class T>
T netDataUnPack(const QVariant &v)
{
    Q_ASSERT(v.type() == QVariant::ByteArray);

    QByteArray buffer = v.toByteArray();
    T param;
    QDataStream s(buffer);
    s >> param;
    return param;
}

class JZNetPackVariant : public JZNetPack
{
public:
    JZNetPackVariant();
    virtual ~JZNetPackVariant();
		
    virtual int type() const;
	virtual void saveToStream(QDataStream &s) const;
	virtual void loadFromStream(QDataStream &s);	

	QVariantMap params;	
};

//JZNetPackManager
typedef JZNetPack *(*CreatePackFunc)();
class JZNetPackManager
{
public:
	static JZNetPackManager *instance();
	JZNetPackManager();

	void init();	
	void registPack(int type,CreatePackFunc func);
	JZNetPack *createPack(int id);

protected:
	QMap<int, CreatePackFunc> m_packFactory;	
};
template<class T> 
JZNetPack *JZNetPackCreate(){ return new T();}

#endif
