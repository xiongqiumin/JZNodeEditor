#include <QTcpServer>
#include <QTcpSocket>
#include "JZNetServer.h"

JZNetServer::JZNetServer(QObject * parent) 
	: QObject(parent)
	, tcpServer(NULL)
{
    tcpServer = new QTcpServer(this);
	m_netId = 1;

	//连接信号
	connect(tcpServer, SIGNAL(newConnection()), this, SLOT(onNewConnect()));
}

JZNetServer::~JZNetServer() 
{	
	stopServer();	    
	delete tcpServer;
}

bool JZNetServer::startServer(int port)
{	
	//启动监听
	if (!tcpServer->listen(QHostAddress::AnyIPv4, port)) {		
		return false;
	}	
	
	return true;
}

void JZNetServer::stopServer()
{
	//停止监听
	if(tcpServer->isListening())
	{
		tcpServer->close();

		//断开所有连接
		QMap<int,QTcpSocket*>::iterator it = m_tcpClients.begin();
		while(it != m_tcpClients.end())
		{
			closeSocket(it.key());
			it = m_tcpClients.begin();
		}
	}
}

bool JZNetServer::isOpen()
{
	//是否正在监听
	return tcpServer->isListening();
}

void JZNetServer::closeConnect(int netId)
{
	//断开连接
    QTcpSocket *socket = m_tcpClients[netId];
    socket->disconnectFromHost();
}

bool JZNetServer::sendPack(int netId, JZNetPack *pack)
{
	//发送数据包给指定客户
	return m_dataManager.sendPack(netId, pack);
}

bool JZNetServer::sendPackExclude(int netId, JZNetPack *pack)
{
	//发送数据包,排除指定客户
	bool ret = true;
	QMap<int,QTcpSocket*>::iterator it = m_tcpClients.begin();
	while(it != m_tcpClients.end())
	{
		if(it.key() != netId)
			ret &= sendPack(it.key(),pack);
		it++;
	}
	return ret;
}

bool JZNetServer::sendPackToAll(JZNetPack *pack)
{
	//发送数据包给所有
	bool ret = true;
	QMap<int,QTcpSocket*>::iterator it = m_tcpClients.begin();
	while(it != m_tcpClients.end())
	{
		ret &= sendPack(it.key(),pack);
		it++;
	}
	return ret;
}

void JZNetServer::onNewConnect()
{	
	//获取新连接
	QTcpSocket *socket = tcpServer->nextPendingConnection();    
	
	//创建信号
	connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	connect(socket, SIGNAL(disconnected()),this, SLOT(onDisconnected()));

	//开启新的会话
	int netId = m_netId++;
	m_dataManager.newSession(netId,socket);
	m_tcpClients[netId] = socket;	

	//发送新连接
	emit sigNewConnect(netId);
}

void JZNetServer::onDisconnected()
{	
	//发送连接断开
	QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
	int netId = m_tcpClients.key(socket);
	emit sigDisConnect(netId);
	
	//关闭
	closeSocket(netId);		
}

void JZNetServer::onReadyRead()
{	
	QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());	

	//接收数据包
	int netId = m_tcpClients.key(socket);	
	m_dataManager.recvPack(netId);	 
	while(true)
	{
		JZNetPackPtr pack = m_dataManager.takePack(netId);
		if (pack)           
			emit sigNetPackRecv(netId, pack);		        
		else
			break;
	}	
}

void JZNetServer::closeSocket(int netId)
{
	//移除会话
	QTcpSocket *socket = m_tcpClients[netId];	
	m_tcpClients.remove(netId);	    
	m_dataManager.endSession(netId);	
	
	//关闭socket
	socket->disconnect();
	socket->close();
    socket->deleteLater();
}
