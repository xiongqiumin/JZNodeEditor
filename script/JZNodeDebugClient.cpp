#include "JZNodeDebugClient.h"

JZNodeDebugClient::JZNodeDebugClient()
{
    connect(&m_client,&JZNetClient::sigConnect,this,&JZNodeDebugClient::onConnect);
	connect(&m_client,&JZNetClient::sigDisConnect,this,&JZNodeDebugClient::onDisConnect);
	connect(&m_client,&JZNetClient::sigNetPackRecv,this,&JZNodeDebugClient::onNetPackRecv);
    m_status = Status_none;
}

JZNodeDebugClient::~JZNodeDebugClient()
{

}

int JZNodeDebugClient::status()
{
    return m_status;
}

void JZNodeDebugClient::setStatus(int status)
{
    if (m_status != status)
    {
        m_status = status;
        emit sigRuntimeStatus(status);
    }
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

void JZNodeDebugClient::init(const JZNodeDebugInfo &info)
{    
    QVariantList params, result;
    params << netDataPack(info);
    sendCommand(Cmd_init, params, result);
}

JZNodeRuntimeInfo JZNodeDebugClient::runtimeInfo()
{
    JZNodeRuntimeInfo info;
    QVariantList params,result;
    if(sendCommand(Cmd_runtimeInfo,params,result))
        info = netDataUnPack<JZNodeRuntimeInfo>(result[0].toByteArray());

    return info;
}

void JZNodeDebugClient::addBreakPoint(QString file,int nodeId)
{
    QVariantList params,result;
    params << file << nodeId;
    sendCommand(Cmd_addBreakPoint,params,result);
}

void JZNodeDebugClient::removeBreakPoint(QString file,int nodeId)
{
    QVariantList params,result;
    params << file << nodeId;
    sendCommand(Cmd_removeBreakPoint,params,result);
}

void JZNodeDebugClient::clearBreakPoint()
{
    QVariantList params,result;
    sendCommand(Cmd_clearBreakPoint,params,result);
}

QVariant JZNodeDebugClient::getVariable(QString name)
{
    QVariant ret;
    QVariantList params,result;
    params << name;
    if(sendCommand(Cmd_getVariable,params,result))
        ret = result[0];

    return ret;
}

void JZNodeDebugClient::setVariable(QString name,QVariant value)
{
    QVariantList params,result;
    params << name << value;
    sendCommand(Cmd_setVariable,params,result);
}

void JZNodeDebugClient::detach()
{
    QVariantList params,result;
    sendCommand(Cmd_detach,params,result);
    disconnectFromServer();
    setStatus(Status_none);
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
    setStatus(Status_none);
}

void JZNodeDebugClient::stepIn()
{
    QVariantList params,result;
    sendCommand(Cmd_stepIn,params,result);
    setStatus(Status_running);
}

void JZNodeDebugClient::stepOver()
{
    QVariantList params,result;
    sendCommand(Cmd_stepOver,params,result);
    setStatus(Status_running);
}

void JZNodeDebugClient::stepOut()
{
    QVariantList params,result;
    sendCommand(Cmd_stepOut,params,result);
    setStatus(Status_running);
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
        setStatus(status);

        if (status == Status_pause)
        {
            JZNodeRuntimeInfo info = runtimeInfo();
            emit sigRuntimeInfo(info);
        }
    }
    else if (packet->cmd == Cmd_runtimeInfo)
    {
        JZNodeRuntimeInfo info = netDataUnPack<JZNodeRuntimeInfo>(packet->params[0].toByteArray());        
        emit sigRuntimeInfo(info);
    }
    else if (packet->cmd == Cmd_runtimeError)
    {
        setStatus(Status_pause);        
        emit sigRuntimeError(netDataUnPack<JZNodeRuntimeError>(packet->params[0].toByteArray()));
    }
}

bool JZNodeDebugClient::sendCommand(int command,QVariantList &params,QVariantList &result)
{
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