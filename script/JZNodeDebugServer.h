#ifndef JZNODE_DEBUG_SERVER_H_
#define JZNODE_DEBUG_SERVER_H_

#include <QThread>
#include <QTcpServer>
#include "JZNetServer.h"
#include "JZNodeEngine.h"

class JZNodeEngine;
class JZNodeDebugServer : public QThread
{
    Q_OBJECT
    
public:
    JZNodeDebugServer();
    ~JZNodeDebugServer();

    bool startServer(int port);
    void stopServer();
    void log(QString log);

    void setEngine(JZNodeEngine *eng);
    bool waitForAttach();

signals:
    void sigStop(QPrivateSignal);

protected slots:
    void onNewConnect(int netId);
	void onDisConnect(int netId);
	void onNetPackRecv(int netId,JZNetPackPtr ptr);
    void onStop();
    void onRuntimeError(JZNodeRuntimeError error);

protected:        
    JZNetServer m_server;
    int m_client;
    JZNodeEngine *m_engine;
    bool m_init;
};




#endif
