#ifndef _NET_CLIENT_H_
#define _NET_CLIENT_H_

#include <QObject>
#include <QMap>
#include "JZNetDataManager.h"

class QTcpSocket;
class QTcpServer;

class JZNetClient : public QObject {
	Q_OBJECT

public:
	JZNetClient(QObject * parent = NULL);
	~JZNetClient();

	bool connectToHost(QString host, int port);
    void connectToHostAsync(QString host, int port);
	void disconnectFromHost();
    bool isConnect();
	
    bool sendPack(JZNetPack *pack);
	JZNetPackPtr waitPackAny(int timeout = -1);	
	JZNetPackPtr waitPackByType(int type,int timeout = -1);
	JZNetPackPtr waitPackBySeq(int seq,int timeout = -1);

signals:
    void sigConnect();
	void sigDisConnect();
    void sigError();
	void sigNetPackRecv(JZNetPackPtr pack);    

private slots:	     
    void onConnected();
    void onDisConnected();
    void onReadyRead();

private:		
	JZNetPackPtr waitPack(int type,int param,int timeout);
    void dispatchPack();

	QTcpSocket *tcpSocket;
    int m_net;
    bool m_waitRecv;
    bool m_userDisconnect;

	JZNetDataManager m_dataManager;
};

#endif
