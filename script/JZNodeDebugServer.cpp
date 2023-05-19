#include "JZNodeDebugServer.h"
#include "JZNodeDebugPacket.h"
#include "JZNodeEngine.h"

JZNodeDebugServer::JZNodeDebugServer()
{
    m_client = -1;
    m_engine = nullptr;
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
    if(m_client != -1)
        m_server.closeConnect(netId);
}

void JZNodeDebugServer::onDisConnect(int netId)
{
    if(m_client == netId)
        m_client = -1;
}

void JZNodeDebugServer::onNetPackRecv(int netId,JZNetPackPtr ptr)
{
    JZNodeDebugPacket *packet = (JZNodeDebugPacket*)(ptr.data());
    JZNodeDebugPacket *result = new JZNodeDebugPacket();
    int cmd = packet->cmd;
    QVariantMap &params = packet->params;

    QString filepath;        
    if(cmd == Cmd_addBreakPoint)
    {
        QString file = params["file"].toString();
        int nodeId = params["nodeId"].toInt();
        m_engine->addBreakPoint(filepath,nodeId);
    }
    else if(cmd == Cmd_removeBreakPoint)
    {
        int id = params["id"].toInt();
        m_engine->removeBreakPoint(id);
    }
    else if(cmd == Cmd_clearBreakPoint)
        m_engine->clearBreakPoint();
    else if(cmd == Cmd_pause)
        m_engine->pause();
    else if(cmd == Cmd_resume)    
        m_engine->resume();
    else if(cmd == Cmd_stepIn)
        m_engine->stepIn();
    else if(cmd == Cmd_stepOver)
        m_engine->stepOver();
    else if(cmd == Cmd_stepOut)                                       
        m_engine->stepOut();
    else if(cmd == Cmd_heart){
        m_engine->runtimeInfo();
    }

    m_server.sendPack(netId,JZNetPackPtr(result));
}

bool JZNodeDebugServer::waitAttach()
{
    while(true)
    {
        if(m_client != -1)
            break;
        QThread::msleep(50);
    }
    return true;
}
