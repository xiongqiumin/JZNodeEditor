#ifndef JZNODE_DEBUG_CLIENT_H_
#define JZNODE_DEBUG_CLIENT_H_

#include <QObject>
#include "JZNetClient.h"
#include "JZNodeDebugPacket.h"

class JZNodeDebugClient : public QObject
{
    Q_OBJECT

public:
    JZNodeDebugClient();
    ~JZNodeDebugClient();

    void addBreakPoint(QString file,int nodeId);    
    void removeBreakPoint(QString file,int nodeId);    
    void clearBreakPoint();    
    void pause();       
    void resume();       
    void stepIn();
    void stepOver();
    void stepOut();

signals:
    void sigDisConnect();   
    void sigBreakTrigger(); 

protected slots:    
    void onConnect();
	void onDisConnect();
	void onNetPackRecv(JZNetPackPtr ptr);    

protected:    
    bool sendCommand(int command,QVariantMap &params,QVariantMap &result);

    JZNetClient m_client;
};


#endif