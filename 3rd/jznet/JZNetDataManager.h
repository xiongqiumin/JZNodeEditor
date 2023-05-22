#ifndef JZNET_MANAGER_H_
#define JZNET_MANAGER_H_

#include <QObject>
#include <QMap>
#include <QByteArray>
#include <QVector>
#include <QTcpSocket>
#include <QSharedPointer>
#include "JZNetPack.h"

class JZNetDataManager
{
public:	
	JZNetDataManager();
	~JZNetDataManager();

	void newSession(int sessionId, QTcpSocket*);
	void endSession(int sessionId);
	
    bool sendPack(int sessionId, JZNetPack *pack);
	void recvPack(int sessionId);		
	void recvData(int sessionId, QByteArray data);

	void parsePack(int sessionId);
	JZNetPackPtr takePack(int sessionId);
	JZNetPackPtr takePackByType(int sessionId, int type);
	JZNetPackPtr takePackBySeq(int sessionId, int seq);

    QTcpSocket *socket(int sessionId);

private:		
    QByteArray packData(const JZNetPack *pack);
	JZNetPackPtr takePack(int sessionId,int type,int param);	

	struct SessionInfo
	{
		QTcpSocket *socket;
		QVector<JZNetPackPtr> packList;
		QByteArray buffer;		
	};	
		
	QMap<int, SessionInfo> m_sessionInfo;
	
    char packHead;
    char packTail;
    char *mRecvBuffer;
    int mRecvSize;
	int m_packSeq;
};

#endif
