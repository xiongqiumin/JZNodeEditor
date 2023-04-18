#ifndef JZNODE_DEBUG_SERVER_H_
#define JZNODE_DEBUG_SERVER_H_

#include <QThread>
#include <QTcpServer>
#include "JZNetServer.h"
#include "JZNodeEngine.h"

class JZNodeDebugServer : public QThread
{
    Q_OBJECT
    
public:
    JZNodeDebugServer();
    ~JZNodeDebugServer();

    void setEngine(JZNodeEngine *eng);
    bool waitAttach();

protected slots:
    void onNewConnect(int netId);
	void onDisConnect(int netId);
	void onNetPackRecv(int netId,JZNetPackPtr ptr);

protected:        

    JZNetServer m_server;
    int m_netId;
    JZNodeEngine *m_engine;
};




#endif