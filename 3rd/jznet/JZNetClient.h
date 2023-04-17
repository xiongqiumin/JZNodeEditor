#ifndef _NET_CLIENT_H_
#define _NET_CLIENT_H_

#include <QObject>
#include <QMap>
#include "netDataManager.h"

class QTcpSocket;
class QTcpServer;

class NetClient : public QObject {
	Q_OBJECT

public:
	NetClient(QObject * parent = NULL);
	~NetClient();

	bool connectToHost(QString host, int port);
    void connectToHostAsync(QString host, int port);
	void disconnectFromHost();
    bool isConnect();
	
	QTcpSocket *clientSocket();

	bool sendPack(NetPackPtr pack);
	bool waitPackAny(NetPackPtr &pack,int timeout = -1);	
	bool waitPackByType(int type,NetPackPtr &pack,int timeout = -1);
	bool waitPackBySeq(int seq,NetPackPtr &pack,int timeout = -1);

signals:
    void sigConnect();
	void sigDisConnect();
    void sigError();
	void sigNetPackRecv(NetPackPtr pack);    

private slots:	     
    void onConnected();
    void onDisConnected();
    void onReadyRead();

private:		
	NetPackPtr waitPack(int type,int param);
    void dispatchPack();

	QTcpSocket *tcpSocket;
	int mUser;
	int mWaitTime;
    bool mWaitData;
    bool mUserDisconnect;

	NetDataManager m_dataManager;
};

#endif
