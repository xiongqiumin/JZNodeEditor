#ifndef JZ_UDP_SOCKET_H_
#define JZ_UDP_SOCKET_H_

#include <QUdpSocket>


class JZUdpSocket : public QObject
{
    Q_OBJECT

public:
    JZUdpSocket(QObject* parent = nullptr);
    ~JZUdpSocket();

protected:
    QUdpSocket *m_socket;
};



#endif