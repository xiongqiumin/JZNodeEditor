#ifndef JZNET_SERVER_H_
#define JZNET_SERVER_H_

#include <QObject>
#include <QMap>
#include "JZNetDataManager.h"

class QTcpSocket;
class QTcpServer;
class NetUser;
class NetPack;

class JZNetServer : public QObject {
	Q_OBJECT

public:
	JZNetServer(QObject * parent = NULL);
	~JZNetServer();
		
	bool startServer(int port);
	void stopServer();
	bool isOpen();	

    void closeConnect(int netId);	

    bool sendPack(int netId,JZNetPack *pack);
    bool sendPackExclude(int netId, JZNetPack *pack);
    bool sendPackToAll(JZNetPack *pack);

signals:		
	void sigNewConnect(int netId);	
	void sigDisConnect(int netId);	
	void sigNetPackRecv(int netId,JZNetPackPtr ptr);

private slots:
	void onNewConnect();
	void onReadyRead();
	void onDisconnected();

private:	
	void closeSocket(int netId);
	
	QTcpServer *tcpServer;					
	QMap<int, QTcpSocket*> m_tcpClients;

	int m_netId;
	JZNetDataManager m_dataManager;
};

#endif
