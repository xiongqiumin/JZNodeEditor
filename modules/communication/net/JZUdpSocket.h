#ifndef JZ_UDP_SOCKET_H_
#define JZ_UDP_SOCKET_H_

#include <QUdpSocket>
#include <QNetworkDatagram>
#include "../JZComm.h"

//JZCommUdpConfig
class JZCommUdpConfig : public JZCommConfig
{
public:
    JZCommUdpConfig();

    virtual void saveToStream(QDataStream& s) const;
    virtual void loadFromStream(QDataStream& s);

    QString ip;
    int port;
};

//JZUdpSocket
class JZUdpSocket : public JZCommObject
{
    Q_OBJECT

public:
    JZUdpSocket(QObject* parent = nullptr);
    ~JZUdpSocket();

    void init(JZCommUdpConfig info);
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
    JZCommUdpConfig m_info;
    QUdpSocket *m_socket;
};



#endif