﻿#ifndef JZNODE_DEBUG_CLIENT_H_
#define JZNODE_DEBUG_CLIENT_H_

#include <QObject>
#include "JZNetClient.h"
#include "JZNodeDebugPacket.h"
#include "JZNodeEngine.h"

class JZCORE_EXPORT JZNodeDebugClient : public QObject
{
    Q_OBJECT

public:
    JZNodeDebugClient();
    ~JZNodeDebugClient();

    bool connectToServer(QString ip,int port);
    void disconnectFromServer();
    bool isConnect();

    bool init(const JZNodeDebugInfo &info,JZNodeProgramInfo &ret);
    bool runtimeInfo(JZNodeRuntimeInfo &ret);
    bool addBreakPoint(const BreakPoint &pt);
    bool removeBreakPoint(QString file,int nodeId);
    bool clearBreakPoint();    
    bool getVariable(const JZNodeGetDebugParam &info,JZNodeGetDebugParamResp &ret);
    bool setVariable(const JZNodeSetDebugParam &info,JZNodeSetDebugParamResp &ret);
    
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
    void sigRuntimeError(JZNodeRuntimeError error);    
    void sigRuntimeWatch(const JZNodeRuntimeWatch &info);
    void sigLog(QString log);

protected slots:    
    void onConnect();
	void onDisConnect();
	void onNetPackRecv(JZNetPackPtr ptr);    

protected:    
    bool sendCommand(int command,QVariantList &params,QVariantList &result);    
    JZNetClient m_client;    
};


#endif
