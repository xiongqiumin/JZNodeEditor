#include <QHostAddress>
#include "JZUdpSocket.h"

JZUdpSocket::JZUdpSocket(QObject* parent)
    : QObject(parent)
{
    m_socket = new QUdpSocket(this);
}

JZUdpSocket::~JZUdpSocket()
{
}

void JZUdpSocket::send(const QByteArray& buffer, QString ip, int port)
{
    m_socket->writeDatagram(buffer, QHostAddress(ip), port);
}

QNetworkDatagram JZUdpSocket::recv()
{
    m_socket->waitForReadyRead();
    return m_socket->receiveDatagram();
}