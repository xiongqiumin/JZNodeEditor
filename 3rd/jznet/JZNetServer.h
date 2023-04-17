#pragma once
#include <QObject>
#include <QMap>
#include "netDataManager.h"
#include "NetInfo.h"

class QTcpSocket;
class QTcpServer;
class NetUser;
class NetPack;

class NetServer : public QObject {
	Q_OBJECT

public:
	NetServer(QObject * parent = NULL);
	~NetServer();
		
	bool startServer(int port);
	void stopServer();
	bool isOpen();	

	NetInfo netInfo(int netId);	
    void closeConnect(int netId);	

	bool sendPack(int netId,NetPackPtr pack);	
	bool sendPackExclude(int netId, NetPackPtr pack);
	bool sendPackToAll(NetPackPtr pack);

signals:		
	void onNewConnect(int netId);	
	void onDisConnect(int netId);	
	void onNetPackRecv(int netId,NetPackPtr ptr);

private slots:
	void onNewConnect();
	void onReadyRead();
	void onDisconnected();

private:	
	void closeSocket(int netId);
	
	QTcpServer *tcpServer;					
	QMap<int, QTcpSocket*> m_tcpClients;

	int m_netId;
	NetDataManager m_dataManager;
};
