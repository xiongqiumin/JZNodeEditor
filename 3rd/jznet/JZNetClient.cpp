#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QThread>
#include <QElapsedTimer>
#include "JZNetClient.h"
#include "JZNetDataManager.h"

JZNetClient::JZNetClient(QObject * parent) 
	: QObject(parent)	
{
	tcpSocket = new QTcpSocket();

	mUser = 1;
    m_waitRecv = false;
    mUserDisconnect = false;

	//建立连接    
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(onDisConnected()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SIGNAL(sigError()));
}

JZNetClient::~JZNetClient()
{	
	disconnectFromHost();	
	delete tcpSocket;
}

void JZNetClient::disconnectFromHost()
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

bool JZNetClient::connectToHost(QString host,int port)
{
	//连接服务器
	tcpSocket->connectToHost(host, port);	
    mUserDisconnect = false;
	
	//等待连接成功
	bool ret = tcpSocket->waitForConnected();
	if(ret)
	{
		//创建新的网络会话
		m_dataManager.newSession(mUser,tcpSocket);
	}
	return ret;
}

void JZNetClient::connectToHostAsync(QString host, int port)
{
    //连接异步服务器
    tcpSocket->abort();
    tcpSocket->connectToHost(host, port);
}

bool JZNetClient::isConnect()
{
	//判断连接状态
    return tcpSocket->state() == QTcpSocket::ConnectedState;
}

void JZNetClient::onConnected()
{
    m_dataManager.newSession(mUser, tcpSocket);
    emit sigConnect();
}

void JZNetClient::onDisConnected()
{	
	//发送连接断开信号
    if(!mUserDisconnect)
	    emit sigDisConnect();
	
	//结束会话
	m_dataManager.endSession(mUser);
	mUser = 0;
    mUserDisconnect = false;
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
    m_dataManager.recvPack(mUser);
    while (true)
    {
        JZNetPackPtr pack = m_dataManager.takePack(mUser);
        if (pack)
        {
            emit sigNetPackRecv(pack);
        }
        else
            break;
    }
}

bool JZNetClient::sendPack(JZNetPackPtr pack)
{	
	//发送数据包
	return m_dataManager.sendPack(mUser, pack);
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
        m_dataManager.recvPack(mUser);
		if(type == 0)
			pack = m_dataManager.takePack(mUser);
		else if(type == 1)
			pack = m_dataManager.takePackByType(mUser, param);
		else 
			pack = m_dataManager.takePackBySeq(mUser, param);
        if(pack)
            break;

		QThread::msleep(10);
    }
    m_waitRecv = false; 
    
    QTimer::singleShot(0, this, [this]{ dispatchPack();});
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