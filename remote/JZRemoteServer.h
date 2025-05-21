#ifndef JZ_REMOTE_SERVER_H_
#define JZ_REMOTE_SERVER_H_

#include <QThread>
#include <QTcpServer>
#include <QProcess>
#include "3rd/JZCommon/jzNet/JZNetServer.h"
#include "JZRemotePacket.h"

class JZRemoteServer : public QObject
{
    Q_OBJECT
    
public:
    JZRemoteServer();
    ~JZRemoteServer();

    bool startServer(int port);
    void stopServer();

signals:    
    

protected slots:
    void onNewConnect(int netId);
	void onDisConnect(int netId);
	void onNetPackRecv(int netId,JZNetPackPtr ptr);

    void onRuntimeFinish();

protected:

    int m_client;
    JZNetServer *m_server;
    QProcess m_process;
};




#endif
