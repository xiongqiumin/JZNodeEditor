#ifndef JZ_TCP_SERVER_H_
#define JZ_TCP_SERVER_H_

#include <QTcpServer>
#include <QSharedPointer>
#include "../JZCommPack.h"
#include "../JZComm.h"

//JZCommTcpServerConfig
class JZCommTcpServerConfig : public JZCommConfig
{
public:
    JZCommTcpServerConfig();
    
    virtual void saveToStream(QDataStream& s) const;
    virtual void loadFromStream(QDataStream& s);

    QString ip;
    int port;
    JZCommPackFormat packFormat;
};

//JZTcpServer
class JZTcpServer : public JZCommObject
{
    Q_OBJECT
    
public:
    JZTcpServer(QObject *parent = nullptr);
    ~JZTcpServer();

    virtual bool isOpen();
    virtual bool open();
    virtual void close();

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

    bool m_stopServer;
    int m_netId;
    QTcpServer* m_server;
    QMap<int, ClientPtr> m_tcpClients;
};

#endif