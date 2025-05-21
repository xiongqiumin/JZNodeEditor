#include "JZRemoteClient.h"

JZRemoteClient::JZRemoteClient()
{
    connect(&m_client,&JZNetClient::sigConnect,this,&JZRemoteClient::onConnect);
	connect(&m_client,&JZNetClient::sigDisConnect,this,&JZRemoteClient::onDisConnect);
	connect(&m_client,&JZNetClient::sigNetPackRecv,this,&JZRemoteClient::onNetPackRecv);
}

JZRemoteClient::~JZRemoteClient()
{

}

bool JZRemoteClient::isConnect()
{
    return m_client.isConnect();
}

bool JZRemoteClient::connectToServer(QString ip,int port)
{
    return m_client.connectToHost(ip,port);
}

void JZRemoteClient::disconnectFromServer()
{
    m_client.disconnectFromHost();
}

bool JZRemoteClient::startProgram(JZNodeProgram *program)
{
    JZRemoteStartPacketReq req;
    req.setId(m_client.genPackId());
    program->copyTo(&req.program);
    m_client.sendPack(&req);
    
    JZNetPackPtr ptr = m_client.waitPackById(req.id());
    if (!ptr)
        return false;

    auto res = dynamic_cast<JZRemoteStartPacketRes*>(ptr.data());
    return (res->code == 0);
}

bool JZRemoteClient::stopProgram()
{
    JZRemoteStopPacketReq req;
    req.setId(m_client.genPackId());
    m_client.sendPack(&req);

    JZNetPackPtr ptr = m_client.waitPackById(req.id());
    if (!ptr)
        return false;

    auto res = dynamic_cast<JZRemoteStopPacketRes*>(ptr.data());
    return (res->code == 0);
}

void JZRemoteClient::onConnect()
{    
}

void JZRemoteClient::onDisConnect()
{    
}

void JZRemoteClient::onNetPackRecv(JZNetPackPtr ptr)
{    
}