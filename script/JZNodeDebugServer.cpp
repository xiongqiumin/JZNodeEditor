#include "JZNodeDebugServer.h"
#include "JZNodeDebugPacket.h"

JZNodeDebugServer::JZNodeDebugServer()
{
    m_server.moveToThread(this);

    connect(&m_server,&JZNetServer::sigNewConnect,this,&JZNodeDebugServer::onNewConnect);
	connect(&m_server,&JZNetServer::sigDisConnect,this,&JZNodeDebugServer::onDisConnect);
	connect(&m_server,&JZNetServer::sigNetPackRecv,this,&JZNodeDebugServer::onNetPackRecv);
}

JZNodeDebugServer::~JZNodeDebugServer()
{
    
}

void JZNodeDebugServer::setEngine(JZNodeEngine *eng)
{
    m_engine = eng;
}

void JZNodeDebugServer::onNewConnect(int netId)
{
    if(m_netId != -1)
        m_server.closeConnect(netId);
}

void JZNodeDebugServer::onDisConnect(int netId)
{
    if(netId == m_netId)
        m_netId = -1;
}

void JZNodeDebugServer::onNetPackRecv(int netId,JZNetPackPtr ptr)
{
    JZNodeDebugPacket *packet = (JZNodeDebugPacket*)(ptr.data());

    JZNodeDebugPacket *result = new JZNodeDebugPacket();
    m_server.sendPack(netId,JZNetPackPtr(result));
}