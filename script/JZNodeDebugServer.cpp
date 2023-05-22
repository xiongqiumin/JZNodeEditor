#include "JZNodeDebugServer.h"
#include "JZNodeDebugPacket.h"
#include "JZNodeEngine.h"

JZNodeDebugServer::JZNodeDebugServer()
{
    m_client = -1;
    m_engine = nullptr;    
    this->moveToThread(this);

    connect(&m_server,&JZNetServer::sigNewConnect,this,&JZNodeDebugServer::onNewConnect);
	connect(&m_server,&JZNetServer::sigDisConnect,this,&JZNodeDebugServer::onDisConnect);
	connect(&m_server,&JZNetServer::sigNetPackRecv,this,&JZNodeDebugServer::onNetPackRecv);
}

JZNodeDebugServer::~JZNodeDebugServer()
{
    
}

bool JZNodeDebugServer::startServer(int port)
{
    if(!m_server.startServer(port))
        return false;

    m_server.moveToThread(this);
    QThread::start();
    return true;
}

void JZNodeDebugServer::stopServer()
{
    m_server.stopServer();
    QThread::quit();
    QThread::wait();
} 

bool JZNodeDebugServer::waitForAttach()
{
    while(true)
    {
        if(m_client != -1)
            break;
        QThread::msleep(50);
    }
    return true;
}

void JZNodeDebugServer::setEngine(JZNodeEngine *eng)
{
    m_engine = eng;
}

void JZNodeDebugServer::onNewConnect(int netId)
{
    if(m_client != -1)
        m_server.closeConnect(netId);

    m_client = netId;
}

void JZNodeDebugServer::onDisConnect(int netId)
{
    if(m_client == netId)
        m_client = -1;
}

void JZNodeDebugServer::onNetPackRecv(int netId,JZNetPackPtr ptr)
{
    JZNodeDebugPacket *packet = (JZNodeDebugPacket*)(ptr.data());    
    int cmd = packet->cmd;
    QVariantList &params = packet->params;
    QVariantList result;
    
    if(cmd == Cmd_addBreakPoint)    
        result << m_engine->addBreakPoint(params[0].toString(),params[1].toInt());
    else if(cmd == Cmd_removeBreakPoint)    
        m_engine->removeBreakPoint(params[0].toInt());    
    else if(cmd == Cmd_clearBreakPoint)
        m_engine->clearBreakPoint();
    else if(cmd == Cmd_pause)
        m_engine->pause();
    else if(cmd == Cmd_resume)    
        m_engine->resume();
    else if(cmd == Cmd_stepIn)
        m_engine->stepIn();
    else if(cmd == Cmd_stop)
    {
        m_engine->stop();
        emit sigStop();
    }
    else if(cmd == Cmd_stepOver)
        m_engine->stepOver();
    else if(cmd == Cmd_stepOut)                                       
        m_engine->stepOut();
    else if(cmd == Cmd_runtimeInfo)    
        result << netDataPack(m_engine->runtimeInfo());    
    else if(cmd == Cmd_getVariable)
        result << m_engine->getVariable(params[0].toString());
    else if(cmd == Cmd_setVariable)
        m_engine->setVariable(params[0].toString(),params[1]);

    JZNodeDebugPacket result_pack;
    result_pack.setSeq(packet->seq());
    result_pack.params = result;
    m_server.sendPack(netId,&result_pack);
}
