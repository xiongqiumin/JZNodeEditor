#include <QHostAddress>
#include <QDataStream>
#include <QDebug>
#include <QTimer>
#include "JZUdpSocket.h"

//JZUdpInfo
JZUdpInfo::JZUdpInfo()
{
    port = -1;
}

QDataStream& operator<<(QDataStream& s, const JZUdpInfo& param)
{
    s << param.port;
    return s;
}
QDataStream& operator>>(QDataStream& s, JZUdpInfo& param)
{
    s >> param.port;
    return s;
}

//JZUdpSocket
JZUdpSocket::JZUdpSocket(QObject* parent)
    : QObject(parent)
{
    m_socket = new QUdpSocket(this);
    connect(m_socket, &QUdpSocket::readyRead, this, &JZUdpSocket::onReadyRead);
    m_waitRecv = false;
}

JZUdpSocket::~JZUdpSocket()
{
}

void JZUdpSocket::init(JZUdpInfo info)
{
    m_info = info;
}

bool JZUdpSocket::isOpen()
{
    return m_socket->state() == QAbstractSocket::BoundState;
}

bool JZUdpSocket::open()
{
    if (m_info.port != -1)
        return m_socket->bind(m_info.port);
    else
        return m_socket->bind();
}

void JZUdpSocket::close()
{
    m_socket->close();
}

void JZUdpSocket::write(const QByteArray& buffer, QString ip, int port)
{
    m_socket->writeDatagram(buffer, QHostAddress(ip), port);
}

void JZUdpSocket::onReadyRead()
{
    if (m_waitRecv)
        return;

    while (m_socket->hasPendingDatagrams())
    {
        QNetworkDatagram d = m_socket->receiveDatagram();
        emit sigDataRecv(d);
    }
}

QNetworkDatagram JZUdpSocket::read()
{
    QNetworkDatagram d;

    m_waitRecv = true;
    m_socket->waitForReadyRead();
    d = m_socket->receiveDatagram();
    m_waitRecv = false;

    QTimer::singleShot(0, this, [this] {
        onReadyRead();
    });
    
    return d.data();
}