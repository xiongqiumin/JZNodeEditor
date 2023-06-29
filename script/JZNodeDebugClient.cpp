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

JZNodeRuntimeInfo JZNodeDebugClient::runtimeInfo()
{
    JZNodeRuntimeInfo info;
    QVariantList params,result;
    if(sendCommand(Cmd_runtimeInfo,params,result))
        info = netDataUnPack<JZNodeRuntimeInfo>(result[0].toByteArray());

    return info;
}

int JZNodeDebugClient::addBreakPoint(QString file,int nodeId)
{
    int id = -1;
    QVariantList params,result;
    params << file << nodeId;
    if(sendCommand(Cmd_addBreakPoint,params,result))
        id = result[0].toInt();

    return id;
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
    
}
	
void JZNodeDebugClient::onDisConnect()
{
    emit sigDisConnect();
}
	
void JZNodeDebugClient::onNetPackRecv(JZNetPackPtr ptr)
{
    JZNodeDebugPacket *packet = new JZNodeDebugPacket();
    if(packet->cmd == Cmd_breakTrigger)
        emit sigBreakTrigger();
    else if(packet->cmd == Cmd_log)
        emit sigLog(packet->params[0].toString());
    else if(packet->cmd == Cmd_runtimeError)
        emit sigRuntimeError(netDataUnPack<JZNodeRuntimeError>(packet->params[0].toByteArray()));
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
