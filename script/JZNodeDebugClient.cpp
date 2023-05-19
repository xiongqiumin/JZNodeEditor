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

void JZNodeDebugClient::addBreakPoint(QString file,int nodeId)
{
    QVariantMap params,result;
    sendCommand(Cmd_addBreakPoint,params,result);
}

void JZNodeDebugClient::removeBreakPoint(QString file,int nodeId)
{
    QVariantMap params,result;
    sendCommand(Cmd_removeBreakPoint,params,result);
}

void JZNodeDebugClient::clearBreakPoint()
{
    QVariantMap params,result;
    sendCommand(Cmd_clearBreakPoint,params,result);
}

void JZNodeDebugClient::pause()
{
    QVariantMap params,result;
    sendCommand(Cmd_pause,params,result);
}

void JZNodeDebugClient::resume()
{
    QVariantMap params,result;
    sendCommand(Cmd_resume,params,result);
}

void JZNodeDebugClient::stepIn()
{
    QVariantMap params,result;
    sendCommand(Cmd_stepIn,params,result);
}

void JZNodeDebugClient::stepOver()
{
    QVariantMap params,result;
    sendCommand(Cmd_stepOver,params,result);
}

void JZNodeDebugClient::stepOut()
{
    QVariantMap params,result;
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
    {

    }
}

bool JZNodeDebugClient::sendCommand(int command,QVariantMap &params,QVariantMap &result)
{
    JZNodeDebugPacket *packet = new JZNodeDebugPacket();
    packet->cmd = command;
    packet->params = params;
    if(!m_client.sendPack(JZNetPackPtr(packet)))
        return false;

    JZNetPackPtr ret;    
    if(!(ret = m_client.waitPackBySeq(packet->seq(),30 * 1000)))
        return false;

    JZNodeDebugPacket *recv = (JZNodeDebugPacket *)ret.data();
    return true;
}