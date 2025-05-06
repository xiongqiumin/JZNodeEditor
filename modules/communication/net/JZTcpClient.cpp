#include <QHostAddress>
#include "JZTcpClient.h"

JZTcpClient::JZTcpClient(QObject* parent)
    : QObject(parent)
{
    m_socket = new QTcpSocket(this);
}

JZTcpClient::~JZTcpClient()
{
    disconnectFromHost();
}

void JZTcpClient::connectToHost(QString ip, int port)
{
    m_socket->connectToHost(QHostAddress(ip), port);
}

void JZTcpClient::disconnectFromHost()
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