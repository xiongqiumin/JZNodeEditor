#ifndef JZNODE_DEBUG_CLIENT_H_
#define JZNODE_DEBUG_CLIENT_H_

#include <QObject>
#include "JZNetClient.h"
#include "JZNodeDebugPacket.h"
#include "JZNodeEngine.h"

class JZNodeDebugClient : public QObject
{
    Q_OBJECT

public:
    JZNodeDebugClient();
    ~JZNodeDebugClient();

    bool connectToServer(QString ip,int port);
    void disconnectFromServer();

    JZNodeRuntimeInfo runtimeInfo();
    int addBreakPoint(QString file,int nodeId);
    void removeBreakPoint(int id);    
    void clearBreakPoint();    
    QVariant getVariable(QString name);
    void setVariable(QString name,QVariant value);
    void pause();       
    void resume();
    void stop();
    void stepIn();
    void stepOver();
    void stepOut();    

signals:
    void sigNetError();
    void sigDisConnect();   
    void sigBreakTrigger(); 

protected slots:    
    void onConnect();
	void onDisConnect();
	void onNetPackRecv(JZNetPackPtr ptr);    

protected:    
    bool sendCommand(int command,QVariantList &params,QVariantList &result);

    JZNetClient m_client;
};


#endif