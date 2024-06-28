#ifndef JZNODE_DEBUG_SERVER_H_
#define JZNODE_DEBUG_SERVER_H_

#include <QThread>
#include <QTcpServer>
#include "JZNetServer.h"
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
    void setVM(JZNodeVM *vm);

    bool waitForAttach(int timeout = 30 * 1000);
    JZNodeDebugInfo debugInfo();

signals:
    void sigStop(QThread *stopThread, QPrivateSignal);

protected slots:
    void onNewConnect(int netId);
	void onDisConnect(int netId);
	void onNetPackRecv(int netId,JZNetPackPtr ptr);
    void onStop(QThread *stopThread);

    void onRuntimeError(JZNodeRuntimeError error);    
    void onStatusChanged(int status);
    void onNodePropChanged(QString file,int id,QString value);
    void onLog(const QString &log);

protected:        
    QVariant getVariable(const JZNodeDebugParamInfo &list);
    QVariant setVariable(const JZNodeSetDebugParamInfo &list);
    JZNodeDebugParamValue toDebugParam(const QVariant &value);      
    JZVariant *getVariableRef(int stack,const JZNodeParamCoor &coor);

    bool m_init;
    int m_client;

    JZNodeDebugInfo m_debugInfo;
    JZNetServer m_server;    
    JZNodeEngine *m_engine;    
    JZNodeVM *m_vm;        
};




#endif
