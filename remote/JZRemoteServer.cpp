#include <QApplication>
#include <QElapsedTimer>
#include "JZRemoteServer.h"
#include "JZNodeDebugPacket.h"
#include "JZNodeEngine.h"
#include "JZNodeVM.h"
#include "JZContainer.h"

JZRemoteServer::JZRemoteServer()
{
    m_client = -1;    
    m_server = new JZNetServer(this);
    connect(m_server,&JZNetServer::sigNewConnect,this,&JZRemoteServer::onNewConnect);
	connect(m_server,&JZNetServer::sigDisConnect,this,&JZRemoteServer::onDisConnect);
	connect(m_server,&JZNetServer::sigNetPackRecv,this,&JZRemoteServer::onNetPackRecv); 

    connect(&m_process, (void (QProcess::*)(int, QProcess::ExitStatus))&QProcess::finished, this, &JZRemoteServer::onRuntimeFinish);
}

JZRemoteServer::~JZRemoteServer()
{    
    stopServer();

    if (m_process.state() != QProcess::NotRunning)
        m_process.kill();
}

bool JZRemoteServer::startServer(int port)
{
    if(!m_server->startServer(port))
        return false;
    return true;
}

void JZRemoteServer::stopServer()
{
    m_server->stopServer();
} 

void JZRemoteServer::onNewConnect(int netId)
{
    if(m_client != -1)
        m_server->closeConnect(netId);

    m_client = netId;
}

void JZRemoteServer::onDisConnect(int netId)
{
    if(m_client == netId)
        m_client = -1;    
    
    exit(1);
}

void JZRemoteServer::onNetPackRecv(int netId,JZNetPackPtr ptr)
{
    int cmd = ptr->type();
    if (cmd == Remote_StartReq)
    {
        qDebug() << "start process";

        JZRemoteStartPacketRes res;
        res.code = 0;
        res.setId(ptr->id());

        JZRemoteStartPacketReq *pkt_start = (JZRemoteStartPacketReq*)ptr.data();
        if (m_process.state() == QProcess::NotRunning)
        {
            QString app = qApp->applicationDirPath() + "/JZNodeEditor.exe";
            QString exe_path = qApp->applicationDirPath();
            QString build_exe = exe_path + "/remote.program";

            QStringList params;
            params << "--run" << build_exe << "--debug";

            m_process.setWorkingDirectory(exe_path);
            m_process.start(app, params);
            if (!m_process.waitForStarted())
                res.code = 1;            
        }
        else
        {
            res.code = 1;
        }

        m_server->sendPack(netId, &res);
    }
    else if (cmd == Remote_StopReq)
    {
        qDebug() << "stop process";

        JZRemoteStopPacketReq *req = (JZRemoteStopPacketReq*)ptr.data();

        JZRemoteStopPacketRes res;
        res.setId(req->id());
        res.code = 0;

        if(m_process.state() != QProcess::NotRunning)
            m_process.kill();

        m_server->sendPack(netId, &res);
    }
}

void JZRemoteServer::onRuntimeFinish()
{

}