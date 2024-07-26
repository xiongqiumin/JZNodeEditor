#include "JZNodeDebugClient.h"

JZNodeDebugClient::JZNodeDebugClient()
{
    connect(&m_client,&JZNetClient::sigConnect,this,&JZNodeDebugClient::onConnect);
	connect(&m_client,&JZNetClient::sigDisConnect,this,&JZNodeDebugClient::onDisConnect);
	connect(&m_client,&JZNetClient::sigNetPackRecv,this,&JZNodeDebugClient::onNetPackRecv);
}

JZNodeDebugClient::~JZNodeDebugClient()
{

}

bool JZNodeDebugClient::isConnect()
{
    return m_client.isConnect();
}

bool JZNodeDebugClient::connectToServer(QString ip,int port)
{
    return m_client.connectToHost(ip,port);
}

void JZNodeDebugClient::disconnectFromServer()
{
    m_client.disconnectFromHost();
}

bool JZNodeDebugClient::init(const JZNodeDebugInfo &info,JZNodeProgramInfo &ret)
{    
    QVariantList params, result;
    params << netDataPack(info);
    if (!sendCommand(Cmd_init, params, result))
        return false;

    ret = netDataUnPack<JZNodeProgramInfo>(result[0]);
    return true;
}

bool JZNodeDebugClient::runtimeInfo(JZNodeRuntimeInfo &ret)
{
    QVariantList params,result;
    if(!sendCommand(Cmd_runtimeInfo,params,result))
        return false;
        
    ret = netDataUnPack<JZNodeRuntimeInfo>(result[0]);
    return true;
}

bool JZNodeDebugClient::addBreakPoint(const BreakPoint &pt)
{
    Q_ASSERT(pt.type != BreakPoint::none);

    QVariantList params,result;
    params << netDataPack(pt);
    return sendCommand(Cmd_addBreakPoint,params,result);
}

bool JZNodeDebugClient::removeBreakPoint(QString file,int nodeId)
{
    QVariantList params,result;
    params << file << nodeId;
    return sendCommand(Cmd_removeBreakPoint,params,result);
}

bool JZNodeDebugClient::clearBreakPoint()
{
    QVariantList params,result;
    return sendCommand(Cmd_clearBreakPoint,params,result);
}

bool JZNodeDebugClient::getVariable(const JZNodeGetDebugParam &info,JZNodeGetDebugParamResp &ret)
{
    QVariantList params,result;
    params << netDataPack(info);
    if (!sendCommand(Cmd_getVariable, params, result))
        return false;

    ret = netDataUnPack<JZNodeGetDebugParamResp>(result[0]);
    return true;
}

bool JZNodeDebugClient::setVariable(const JZNodeSetDebugParam &info,JZNodeSetDebugParamResp &ret)
{
    QVariantList params,result;
    params << netDataPack(info);
    if (!sendCommand(Cmd_setVariable,params,result))
        return false;

    ret = netDataUnPack<JZNodeSetDebugParamResp>(result[0]);
    return true;
}

void JZNodeDebugClient::detach()
{
    QVariantList params,result;
    sendCommand(Cmd_detach,params,result);
    disconnectFromServer();
}

void JZNodeDebugClient::pause()
{
    QVariantList params,result;
    sendCommand(Cmd_pause,params,result);    
}

void JZNodeDebugClient::resume()
{
    QVariantList params,result;
    sendCommand(Cmd_resume,params,result);    
}

void JZNodeDebugClient::stop()
{
    QVariantList params,result;
    sendCommand(Cmd_stop,params,result);
    disconnectFromServer();
}

void JZNodeDebugClient::stepIn()
{
    QVariantList params,result;
    sendCommand(Cmd_stepIn,params,result);
}

void JZNodeDebugClient::stepOver()
{
    QVariantList params,result;
    sendCommand(Cmd_stepOver,params,result);
}

void JZNodeDebugClient::stepOut()
{
    QVariantList params,result;
    sendCommand(Cmd_stepOut,params,result);
}

void JZNodeDebugClient::onConnect()
{
    emit sigConnect();
}
	
void JZNodeDebugClient::onDisConnect()
{
    emit sigDisConnect();
}
	
void JZNodeDebugClient::onNetPackRecv(JZNetPackPtr ptr)
{
    JZNodeDebugPacket *packet = (JZNodeDebugPacket*)ptr.data();
    if(packet->cmd == Cmd_log)
        emit sigLog(packet->params[0].toString());
    else if (packet->cmd == Cmd_runtimeStatus)
    {
        int status = packet->params[0].toInt();
        emit sigRuntimeStatus(status);
    }
    else if (packet->cmd == Cmd_runtimeError)
    {   
        emit sigRuntimeError(netDataUnPack<JZNodeRuntimeError>(packet->params[0]));
    }
    else if (packet->cmd == Cmd_nodePropChanged)
    {
        emit sigRuntimeWatch(netDataUnPack<JZNodeRuntimeWatch>(packet->params[0]));
    }
}

bool JZNodeDebugClient::sendCommand(int command,QVariantList &params,QVariantList &result)
{
    if (!m_client.isConnect())
        return false;

    JZNodeDebugPacket packet;
    packet.cmd = command;
    packet.params = params;
    if(!m_client.sendPack(&packet))
        return false;

    JZNetPackPtr ret;    
    if(!(ret = m_client.waitPackBySeq(packet.seq(),30 * 1000)))
    {
        emit sigNetError();
        return false;
    }

    JZNodeDebugPacket *recv = (JZNodeDebugPacket *)ret.data();
    result = recv->params;
    return true;
}