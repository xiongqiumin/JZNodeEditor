#ifndef JZ_TCP_SERVER_H_
#define JZ_TCP_SERVER_H_

#include <QTcpServer>
#include <QSharedPointer>
#include "../JZCommPack.h"

//JZTcpServerInfo
class JZTcpServerInfo
{
public:
    JZTcpServerInfo();

    QString ip;
    int port;
};
QDataStream& operator<<(QDataStream& s, const JZTcpServerInfo& param);
QDataStream& operator>>(QDataStream& s, JZTcpServerInfo& param);

//JZTcpServer
class JZTcpServer : public QObject
{
    Q_OBJECT
    
public:
    JZTcpServer(QObject *parent = nullptr);
    ~JZTcpServer();

    void init(JZTcpServerInfo info);
    void setCommFormat(JZCommPackFormat format);

    bool isOpen();
    bool startServer();
    void stopServer();

    void closeConnect(int netId);
    bool isConnect(int netId);

    bool sendPack(int netId, const QByteArray &pack);
    bool sendPackExclude(int netId, const QByteArray& pack);
    bool sendPackToAll(const QByteArray& pack);

signals:
    void sigNewConnect(int netId);
    void sigDisConnect(int netId);
    void sigNetPackRecv(int netId, QByteArray body);

protected slots:
    void onNewConnect();
    void onReadyRead();
    void onDisconnected();

protected:
    struct Client{
        JZCommPack pack;
        QTcpSocket* socket;
    };
    typedef QSharedPointer<Client> ClientPtr;

    JZTcpServerInfo m_info;
    JZCommPackFormat m_packFormat;

    bool m_stopServer;
    int m_netId;
    QTcpServer* m_server;
    QMap<int, ClientPtr> m_tcpClients;
};

#endif