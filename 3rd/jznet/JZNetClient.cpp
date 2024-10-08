﻿#include <QTcpSocket>
#include <QTimer>
#include <QThread>
#include <QElapsedTimer>
#include "JZNetClient.h"
#include "JZNetDataManager.h"

JZNetClient::JZNetClient(QObject * parent) 
	: QObject(parent)	
{
	m_socket = new QTcpSocket(this);

    m_net = -1;
    m_waitRecv = false;
    m_userDisconnect = false;

	//建立连接    
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(m_socket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(onDisConnected()));
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SIGNAL(sigError()));
}

JZNetClient::~JZNetClient()
{	
	disconnectFromHost();		
}

void JZNetClient::disconnectFromHost()
{
	//从服务器断开
	if(isConnect())
	{
        m_userDisconnect = true;
		m_socket->disconnectFromHost();
		if(m_socket->state() != QAbstractSocket::UnconnectedState)
		{
			if(!m_socket->waitForDisconnected(1000))
				m_socket->abort();
		}
	}
}

bool JZNetClient::connectToHost(QString host,int port)
{
	//连接服务器
	m_socket->connectToHost(host, port);	
    m_userDisconnect = false;
	
	//等待连接成功
	bool ret = m_socket->waitForConnected();
	if(ret)
	{
		//创建新的网络会话
        m_dataManager.newSession(m_net,m_socket);
	}
	return ret;
}

void JZNetClient::connectToHostAsync(QString host, int port)
{
    //连接异步服务器
    m_socket->abort();
    m_socket->connectToHost(host, port);
}

bool JZNetClient::isConnect()
{
	//判断连接状态
    return m_socket->state() == QTcpSocket::ConnectedState;
}

void JZNetClient::onConnected()
{
    m_net = 1;
    m_dataManager.newSession(m_net, m_socket);
    emit sigConnect();
}

void JZNetClient::onDisConnected()
{	
	//发送连接断开信号
    if(!m_userDisconnect)
	    emit sigDisConnect();
	
	//结束会话
    m_dataManager.endSession(m_net);
    m_net = -1;
    m_userDisconnect = false;
}

void JZNetClient::onReadyRead()
{		
	//是否正在等待数据
    if(m_waitRecv)
        return;

    dispatchPack();	
}

void JZNetClient::dispatchPack()
{
    //接收数据包
    m_dataManager.recvPack(m_net);
    while (true)
    {
        JZNetPackPtr pack = m_dataManager.takePack(m_net);
        if (pack)
        {
            emit sigNetPackRecv(pack);
        }
        else
            break;
    }
}

bool JZNetClient::sendPack(JZNetPack *pack)
{	
	//发送数据包
    return m_dataManager.sendPack(m_net, pack);
}

JZNetPackPtr JZNetClient::waitPack(int type, int param,int timeout)
{
	JZNetPackPtr pack;

	QElapsedTimer t;
	t.start();
	//等待数据包
    m_waitRecv = true;               
    while(timeout == -1 || t.elapsed() <= timeout)
    {	            
        m_dataManager.socket(m_net)->waitForReadyRead(10);
        if (m_net == -1)
        {
            m_waitRecv = false;
            return JZNetPackPtr();
        }

        m_dataManager.recvPack(m_net);
		if(type == 0)
            pack = m_dataManager.takePack(m_net);
		else if(type == 1)
            pack = m_dataManager.takePackByType(m_net, param);
		else 
            pack = m_dataManager.takePackBySeq(m_net, param);
        if(pack)
            break;
    }
    m_waitRecv = false; 
    
    QTimer::singleShot(0, this, [this]{ 
        if(m_net != -1) 
            dispatchPack();
    });
	return pack;
}

JZNetPackPtr JZNetClient::waitPackAny(int timeout)
{
	return waitPack(0, 0, timeout);
}

JZNetPackPtr JZNetClient::waitPackByType(int type,int timeout)
{
	return waitPack(1, type, timeout);
}

JZNetPackPtr JZNetClient::waitPackBySeq(int seq,int timeout)
{
	return waitPack(2, seq, timeout);
}
