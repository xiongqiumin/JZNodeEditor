#ifndef JZNODE_DEBUG_CLIENT_H_
#define JZNODE_DEBUG_CLIENT_H_

#include <QObject>
#include "3rd/JZCommon/jzNet/JZNetClient.h"
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
    bool addBreakPoint(const BreakPoint &pt);
    bool removeBreakPoint(QString file,int nodeId);
    bool clearBreakPoint();    
    bool getVariable(const JZNodeGetDebugParam &info,JZNodeGetDebugParamResp &ret);
    bool setVariable(const JZNodeSetDebugParam &info,JZNodeSetDebugParamResp &ret);
    
    bool detach();
    bool pause();
    bool resume();
    bool stop();
    bool stepIn();
    bool stepOver();
    bool stepOut();

signals:
    void sigNetError();
    void sigConnect();
    void sigDisConnect();       

    void sigRuntimeStatus(int stauts);    
    void sigRuntimeError(JZNodeRuntimeError error);    
    void sigRuntimeWatch(const JZNodeRuntimeWatchResult &info);
    void sigLog(QString log);

protected slots:    
    void onConnect();
	void onDisConnect();
	void onNetPackRecv(JZNetPackPtr ptr);    

protected:    
    bool sendCommand(int command, const QByteArray &send, QByteArray &result);
    JZNetClient m_client;    
};


#endif
