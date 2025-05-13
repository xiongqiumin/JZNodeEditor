#ifndef JZ_UDP_SOCKET_H_
#define JZ_UDP_SOCKET_H_

#include <QUdpSocket>
#include <QNetworkDatagram>

//JZUdpInfo
class JZUdpInfo
{
public:
    JZUdpInfo();

    int port;
};
QDataStream& operator<<(QDataStream& s, const JZUdpInfo& param);
QDataStream& operator>>(QDataStream& s, JZUdpInfo& param);

//JZUdpSocket
class JZUdpSocket : public QObject
{
    Q_OBJECT

public:
    JZUdpSocket(QObject* parent = nullptr);
    ~JZUdpSocket();

    void init(JZUdpInfo info);
    bool isOpen();
    bool open();
    void close();

    void write(const QByteArray &buffer,QString ip,int port);
    QNetworkDatagram read();

protected:
    void onReadyRead();

signals:
    void sigDataRecv(const QNetworkDatagram &data);

protected:
    bool m_waitRecv;
    JZUdpInfo m_info;
    QUdpSocket *m_socket;
};



#endif