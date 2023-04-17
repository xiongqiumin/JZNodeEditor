#ifndef JZNODE_DEBUG_SERVER_H_
#define JZNODE_DEBUG_SERVER_H_

#include <QThread>
#include <QTcpServer>
#include "JZNodeDebugPacket.h"

class JZNodeDebugServer : public QThread
{
    Q_OBJECT
    
public:
    JZNodeDebugServer();
    ~JZNodeDebugServer();

    int start();
    void stop();

protected slots:

protected:
    void onConnect();
	void onDisConnect();
	void onNetError();
	void onDataRecv(NetPackPtr ptr);    

    QTcpServer *m_server;
};




#endif