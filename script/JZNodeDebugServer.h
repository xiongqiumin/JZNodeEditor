#ifndef JZNODE_DEBUG_SERVER_H_
#define JZNODE_DEBUG_SERVER_H_

#include <QThread>
#include <QTcpServer>
#include "3rd/JZCommon/jzNet/JZNetServer.h"
#include "JZNodeEngine.h"
#include "JZNodeDebugPacket.h"

class JZNodeEngine;
class JZNodeVM;
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

    bool waitForAttach(int timeout = 30 * 1000);
    JZNodeDebugInfo debugInfo();

signals:    
    

protected slots:
    void onNewConnect(int netId);
	void onDisConnect(int netId);
	void onNetPackRecv(int netId,JZNetPackPtr ptr);

    void onRuntimeError(JZNodeRuntimeError error);    
    void onStatusChanged(int status);
    void onWatchNotify();
    void onLog(const QString &log);

protected:        
    virtual void run() override;

    JZNodeGetDebugParamResp getVariable(const JZNodeGetDebugParam &list);
    JZNodeSetDebugParamResp setVariable(const JZNodeSetDebugParam &list);
    JZNodeDebugParamValue toDebugParam(const QVariant &value);          

    bool m_init;
    int m_client;

    JZNodeRuntimeWatch m_watch;
    JZNodeDebugInfo m_debugInfo;

    JZNetServer *m_server;    
    JZNodeEngine *m_engine;        
    QThread* m_preThread;    
};




#endif
