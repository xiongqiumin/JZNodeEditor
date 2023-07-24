#ifndef JZNODE_DEBUG_SERVER_H_
#define JZNODE_DEBUG_SERVER_H_

#include <QThread>
#include <QTcpServer>
#include "JZNetServer.h"
#include "JZNodeEngine.h"
#include "JZNodeDebugPacket.h"

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
    JZNodeDebugInfo debugInfo();

signals:
    void sigStop(QPrivateSignal);

protected slots:
    void onNewConnect(int netId);
	void onDisConnect(int netId);
	void onNetPackRecv(int netId,JZNetPackPtr ptr);
    void onStop();

    void onRuntimeError(JZNodeRuntimeError error);    
    void onStatusChanged(int status);
    void onLog(const QString &log);

protected:        
    JZNetServer m_server;
    int m_client;
    JZNodeEngine *m_engine;
    bool m_init;
    JZNodeDebugInfo m_debugInfo;
};




#endif
