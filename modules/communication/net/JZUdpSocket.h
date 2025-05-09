#ifndef JZ_UDP_SOCKET_H_
#define JZ_UDP_SOCKET_H_

#include <QUdpSocket>
#include <QNetworkDatagram>

class JZUdpSocket : public QObject
{
    Q_OBJECT

public:
    JZUdpSocket(QObject* parent = nullptr);
    ~JZUdpSocket();

    void send(const QByteArray &buffer,QString ip,int port);
    QNetworkDatagram recv();

protected:
    QUdpSocket *m_socket;
};



#endif