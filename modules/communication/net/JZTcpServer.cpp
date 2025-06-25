#include <QTcpSocket>
#include <QDataStream>
#include "JZTcpServer.h"

//JZCommTcpServerConfig
JZCommTcpServerConfig::JZCommTcpServerConfig()
{
	type = Comm_TcpServer;
	ip = "127.0.0.1";
	port = 0;
}

void JZCommTcpServerConfig::saveToStream(QDataStream& s) const
{
    JZCommConfig::saveToStream(s);
	s << ip << port << packFormat;
}

void JZCommTcpServerConfig::loadFromStream(QDataStream& s)
{
    JZCommConfig::loadFromStream(s);
	s >> ip >> port >> packFormat;
}

//JZTcpServer
JZTcpServer::JZTcpServer(QObject* parent)
	:JZCommObject(parent)
{
	m_server = new QTcpServer(this);
	m_stopServer = false;
	m_netId = 0;

	connect(m_server, SIGNAL(newConnection()), this, SLOT(onNewConnect()));
}

JZTcpServer::~JZTcpServer()
{
}

bool JZTcpServer::open()
{
	//启动监听
    auto info = dynamic_cast<JZCommTcpServerConfig*>(m_config.data());
	if (!m_server->listen(QHostAddress::AnyIPv4, info->port)) {
		return false;
	}

	return true;
}

void JZTcpServer::close()
{
	//停止监听
	m_stopServer = true;
	if (m_server->isListening())
	{
		m_server->close();

		//断开所有连接
		QList<int> socket_list = m_tcpClients.keys();
		for (int i = 0; i < socket_list.size(); i++)
		{
			int socket_id = socket_list[i];
			auto socket = m_tcpClients[socket_id]->socket;

			socket->disconnect();
			socket->disconnectFromHost();
			emit sigDisConnect(socket_id);
			
			m_tcpClients.remove(socket_id);
			socket->deleteLater();
		}
	}
	m_stopServer = false;
}

bool JZTcpServer::isOpen()
{
	//是否正在监听
	return m_server->isListening();
}

void JZTcpServer::closeConnect(int netId)
{
	//断开连接
	Q_ASSERT(m_tcpClients.contains(netId));
	QTcpSocket* socket = m_tcpClients[netId]->socket;
	socket->disconnectFromHost();
}

bool JZTcpServer::isConnect(int netId)
{
	return m_tcpClients.contains(netId);
}

bool JZTcpServer::sendPack(int netId, const QByteArray& body)
{
	//发送数据包给指定客户
	auto s = m_tcpClients[netId]->socket;
	
	QByteArray send = m_tcpClients[netId]->pack.makePack(body);
	s->write(send);
	return true;
}

bool JZTcpServer::sendPackExclude(int netId, const QByteArray& pack)
{
	//发送数据包,排除指定客户
	bool ret = true;
	auto it = m_tcpClients.begin();
	while (it != m_tcpClients.end())
	{
		if (it.key() != netId)
			ret &= sendPack(it.key(), pack);

		it++;
	}
	return ret;
}

bool JZTcpServer::sendPackToAll(const QByteArray& pack)
{
	//发送数据包给所有
	bool ret = true;
	auto it = m_tcpClients.begin();
	while (it != m_tcpClients.end())
	{
		ret &= sendPack(it.key(), pack);
		it++;
	}
	return ret;
}


void JZTcpServer::onNewConnect()
{
	//获取新连接
	QTcpSocket* socket = m_server->nextPendingConnection();
	if (m_stopServer)
	{
		socket->close();
		return;
	}

	//创建信号
	connect(socket, &QTcpSocket::readyRead, this, &JZTcpServer::onReadyRead);
	connect(socket, &QTcpSocket::disconnected, this, &JZTcpServer::onDisconnected);

	//开启新的会话
	int netId = m_netId++;
	socket->setProperty("NetId", netId);

    auto info = dynamic_cast<JZCommTcpServerConfig*>(m_config.data());
	ClientPtr ptr = ClientPtr(new Client());
	ptr->pack.setFormat(info->packFormat);
	ptr->socket = socket;
	m_tcpClients[netId] = ptr;

	//发送新连接
	emit sigNewConnect(netId);
}


void JZTcpServer::onDisconnected()
{
	//发送连接断开
	QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
	int netId = socket->property("NetId").toInt();
	emit sigDisConnect(netId);

	//关闭
	m_tcpClients.remove(netId);
	socket->deleteLater();
}

void JZTcpServer::onReadyRead()
{
	QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
	int netId = socket->property("NetId").toInt();

	//接收数据包
	m_tcpClients[netId]->pack.appendBuffer(socket->readAll());
	
	while (true)
	{
		QByteArray pack;
		if (!m_tcpClients[netId]->pack.takePack(pack))
			break;

		emit sigNetPackRecv(netId, pack);
	}
}