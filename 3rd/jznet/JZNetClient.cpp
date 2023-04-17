#include "netClient.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <netDataManager.h>
#include <QTimer>

NetClient::NetClient(QObject * parent) 
	: QObject(parent)	
{
	tcpSocket = new QTcpSocket();
	mWaitTime = 30000;

	mUser = 1;
    mWaitData = false;
    mUserDisconnect = false;

	//建立连接    
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(onDisConnected()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SIGNAL(sigError()));
}

NetClient::~NetClient()
{	
	disconnectFromHost();	
	delete tcpSocket;
}

void NetClient::disconnectFromHost()
{
	//从服务器断开
	if(isConnect())
	{
        mUserDisconnect = true;
		tcpSocket->disconnectFromHost();
		if(tcpSocket->state() != QAbstractSocket::UnconnectedState)
		{
			if(!tcpSocket->waitForDisconnected(1000))
				tcpSocket->abort();
		}
	}
}

bool NetClient::connectToHost(QString host,int port)
{
	//连接服务器
	tcpSocket->connectToHost(host, port);	
    mUserDisconnect = false;
	
	//等待连接成功
	bool ret = tcpSocket->waitForConnected(mWaitTime);
	if(ret)
	{
		//创建新的网络会话
		m_dataManager.newSession(mUser,tcpSocket);
	}
	return ret;
}

void NetClient::connectToHostAsync(QString host, int port)
{
    //连接异步服务器
    tcpSocket->abort();
    tcpSocket->connectToHost(host, port);
}

bool NetClient::isConnect()
{
	//判断连接状态
    return tcpSocket->state() == QTcpSocket::ConnectedState;
}

QTcpSocket *NetClient::clientSocket()
{
	//原始指针
	return tcpSocket;
}

void NetClient::onConnected()
{
    m_dataManager.newSession(mUser, tcpSocket);
    emit sigConnect();
}

void NetClient::onDisConnected()
{	
	//发送连接断开信号
    if(!mUserDisconnect)
	    emit sigDisConnect();
	
	//结束会话
	m_dataManager.endSession(mUser);
	mUser = 0;
    mUserDisconnect = false;
}

void NetClient::onReadyRead()
{		
	//是否正在等待数据
    if(mWaitData)
        return;

    dispatchPack();	
}

void NetClient::dispatchPack()
{
    //接收数据包
    m_dataManager.recvPack(mUser);
    while (true)
    {
        NetPackPtr pack = m_dataManager.takePack(mUser);
        if (pack)
        {
            emit newDataRecv(pack);
        }
        else
            break;
    }
}

bool NetClient::sendPack(const NetPack &pack)
{	
	//发送数据包
	return m_dataManager.sendPack(mUser, pack,true);
}

NetPackPtr NetClient::waitPack(int type, int param)
{
	NetPackPtr pack;
        
	//等待数据包
    mWaitData = true;    
    while(tcpSocket->waitForReadyRead(mWaitTime))
    {	    
        m_dataManager.recvPack(mUser);
		if(type == 0)
			pack = m_dataManager.takePack(mUser);
		else if(type == 1)
			pack = m_dataManager.takePackByType(mUser, param);
		else 
			pack = m_dataManager.takePackBySeq(mUser, param);
        if(pack)
            break;
    }
    mWaitData = false; 
    
    QTimer::singleShot(0, [this]{ dispatchPack();});

	return pack;
}

NetPackPtr NetClient::waitPackAny()
{
	return waitPack(0, 0);
}

NetPackPtr NetClient::waitPackByType(int type)
{
	return waitPack(1, type);
}

NetPackPtr NetClient::waitPackBySeq(int seq)
{
	return waitPack(2, seq);
}