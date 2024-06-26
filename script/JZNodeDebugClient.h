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
    bool isConnect();

    bool init(const JZNodeDebugInfo &info,JZNodeProgramInfo &ret);
    bool runtimeInfo(JZNodeRuntimeInfo &ret);
    bool addBreakPoint(QString file,int nodeId);
    bool removeBreakPoint(QString file,int nodeId);
    bool clearBreakPoint();    
    bool getVariable(const JZNodeDebugParamInfo &info,JZNodeDebugParamInfo &ret);
    bool setVariable(const JZNodeSetDebugParamInfo &info,JZNodeDebugParamInfo &ret);
    
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
    void sigNodePropChanged(const JZNodeValueChanged &info);
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
