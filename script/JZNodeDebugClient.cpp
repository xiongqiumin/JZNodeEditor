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
    QByteArray params, result;
    params = netDataPack(info);
    if (!sendCommand(Cmd_init, params, result))
        return false;

    ret = netDataUnPack<JZNodeProgramInfo>(result);
    return true;
}

bool JZNodeDebugClient::runtimeInfo(JZNodeRuntimeInfo &ret)
{
    QByteArray params,result;
    if(!sendCommand(Cmd_runtimeInfo,params,result))
        return false;
        
    ret = netDataUnPack<JZNodeRuntimeInfo>(result);
    return true;
}

bool JZNodeDebugClient::addBreakPoint(const BreakPoint &pt)
{
    Q_ASSERT(pt.type != BreakPoint::none);

    QByteArray params,result;
    params = netDataPack(pt);
    return sendCommand(Cmd_addBreakPoint,params,result);
}

bool JZNodeDebugClient::removeBreakPoint(QString file,int nodeId)
{
    QByteArray params,result;
    //params = file << nodeId;
    return sendCommand(Cmd_removeBreakPoint,params,result);
}

bool JZNodeDebugClient::clearBreakPoint()
{
    QByteArray params,result;
    return sendCommand(Cmd_clearBreakPoint,params,result);
}

bool JZNodeDebugClient::getVariable(const JZNodeGetDebugParam &info,JZNodeGetDebugParamResp &ret)
{
    QByteArray params,result;
    params = netDataPack(info);
    if (!sendCommand(Cmd_getVariable, params, result))
        return false;

    ret = netDataUnPack<JZNodeGetDebugParamResp>(result);
    return true;
}

bool JZNodeDebugClient::setVariable(const JZNodeSetDebugParam &info,JZNodeSetDebugParamResp &ret)
{
    QByteArray params,result;
    params = netDataPack(info);
    if (!sendCommand(Cmd_setVariable,params,result))
        return false;

    ret = netDataUnPack<JZNodeSetDebugParamResp>(result);
    return true;
}

bool JZNodeDebugClient::detach()
{
    QByteArray params,result;
    bool ret = sendCommand(Cmd_detach,params,result);
    disconnectFromServer();
    return ret;
}

bool JZNodeDebugClient::pause()
{
    QByteArray params,result;
    return sendCommand(Cmd_pause,params,result);    
}

bool JZNodeDebugClient::resume()
{
    QByteArray params,result;
    return sendCommand(Cmd_resume,params,result);
}

bool JZNodeDebugClient::stop()
{
    QByteArray params,result;
    bool ret = sendCommand(Cmd_stop,params,result);
    disconnectFromServer();
    return ret;
}

bool JZNodeDebugClient::stepIn()
{
    QByteArray params,result;
    return sendCommand(Cmd_stepIn,params,result);
}

bool JZNodeDebugClient::stepOver()
{
    QByteArray params,result;
    return sendCommand(Cmd_stepOver,params,result);
}

bool JZNodeDebugClient::stepOut()
{
    QByteArray params,result;
    return sendCommand(Cmd_stepOut,params,result);
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
    if (packet->cmd == Cmd_log)
    {
        //emit sigLog(packet->buffer.toString());
    }
    else if (packet->cmd == Cmd_runtimeStatus)
    {
        int status = packet->buffer.toInt();
        emit sigRuntimeStatus(status);
    }
    else if (packet->cmd == Cmd_runtimeError)
    {   
        emit sigRuntimeError(netDataUnPack<JZNodeRuntimeError>(packet->buffer));
    }
    else if (packet->cmd == Cmd_watchChanged)
    {
        emit sigRuntimeWatch(netDataUnPack<JZNodeRuntimeWatchResult>(packet->buffer));
    }
}

bool JZNodeDebugClient::sendCommand(int command,const QByteArray &buffer,QByteArray &result)
{
    if (!m_client.isConnect())
        return false;

    JZNodeDebugPacket packet;
    packet.cmd = command;
    packet.setId(m_client.genPackId());
    packet.buffer = buffer;
    if(!m_client.sendPack(&packet))
        return false;

    JZNetPackPtr ret;    
    if(!(ret = m_client.waitPackById(packet.id(),30 * 1000)))
    {
        emit sigNetError();
        return false;
    }

    JZNodeDebugPacket *recv = (JZNodeDebugPacket *)ret.data();
    result = recv->buffer;
    return true;
}