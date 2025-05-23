#include <QHostAddress>
#include <QDataStream>
#include "JZTcpClient.h"

//JZCommTcpClientConfig
JZCommTcpClientConfig::JZCommTcpClientConfig()
{
    ip = "127.0.0.1";
    port = 8888;
}

void JZCommTcpClientConfig::saveToStream(QDataStream& s) const
{
    s << ip << port << format;
}

void JZCommTcpClientConfig::loadFromStream(QDataStream& s)
{
    s >> ip >> port >> format;
}

//JZTcpClient
JZTcpClient::JZTcpClient(QObject* parent)
    : JZCommObject(parent)
{
    m_socket = new QTcpSocket(this);
}

JZTcpClient::~JZTcpClient()
{
    close();
}

bool JZTcpClient::isOpen()
{
    return (m_socket->state() == QTcpSocket::ConnectedState);
}

bool JZTcpClient::open()
{
    m_socket->connectToHost(QHostAddress(m_info.ip), m_info.port);
    return m_socket->waitForConnected();
}

void JZTcpClient::close()
{
    m_socket->disconnectFromHost();
}

void JZTcpClient::write(const QByteArray &buffer)
{
    QByteArray pack = m_pack.makePack(buffer);
    m_socket->write(pack);
}

QByteArray JZTcpClient::read()
{
    QByteArray buffer;
    while (true)
    {
        if (m_socket->waitForReadyRead(20))
        {
            m_pack.appendBuffer(m_socket->readAll());
            if (m_pack.takePack(buffer))
                break;
        }
    }
    return buffer;
}

void JZTcpClient::writeText(const QString &buffer)
{
    write(buffer.toUtf8());
}

QString JZTcpClient::readText()
{
    QByteArray buffer = read();
    return QString::fromUtf8(buffer);
}