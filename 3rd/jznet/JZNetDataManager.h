#pragma once
#include <QObject>
#include <QMap>
#include <QByteArray>
#include <QVector>
#include <QTcpSocket>
#include <QSharedPointer>
#include "netPack.h"

class NetDataManager
{
public:	
	NetDataManager();
	~NetDataManager();

	void newSession(int sessionId, QTcpSocket*);
	void endSession(int sessionId);
	
	bool sendPack(int sessionId, NetPackPtr pack);
	void recvPack(int sessionId);		
	void recvData(int sessionId, QByteArray data);

	void parsePack(int sessionId);
	NetPackPtr takePack(int sessionId);
	NetPackPtr takePackByType(int sessionId, int type);
	NetPackPtr takePackBySeq(int sessionId, int seq);

private:	
	QByteArray packData(const NetPackPtr &pack);
	NetPackPtr takePack(int sessionId,int type,int param);	

	struct SessionInfo
	{
		QTcpSocket *socket;
		QVector<NetPackPtr> packList;
		QByteArray buffer;		
	};	
		
	QMap<int, SessionInfo> m_sessionInfo;
	
    QByteArray packHead;
    QByteArray packTail;

    char *mRecvBuffer;
    int mRecvSize;
};