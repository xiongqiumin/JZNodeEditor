﻿#ifndef JZNODE_DEBUG_CLIENT_H_
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
    bool isConnect();
    int status();

    void init(const JZNodeDebugInfo &info);
    JZNodeRuntimeInfo runtimeInfo();
    void addBreakPoint(QString file,int nodeId);
    void removeBreakPoint(QString file,int nodeId);
    void clearBreakPoint();    
    QVariant getVariable(QString name);
    void setVariable(QString name,QVariant value);
    
    void detach();
    void pause();       
    void resume();
    void stop();
    void stepIn();
    void stepOver();
    void stepOut();    

signals:
    void sigNetError();
    void sigConnect();
    void sigDisConnect();       

    void sigRuntimeStatus(int stauts);
    void sigRuntimeInfo(JZNodeRuntimeInfo info);
    void sigRuntimeError(JZNodeRuntimeError error);
    void sigLog(QString log);

protected slots:    
    void onConnect();
	void onDisConnect();
	void onNetPackRecv(JZNetPackPtr ptr);    

protected:    
    bool sendCommand(int command,QVariantList &params,QVariantList &result);
    void setStatus(int status);

    JZNetClient m_client;
    int m_status;
};


#endif
