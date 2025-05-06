#ifndef JZTCP_CLIENT_H_
#define JZTCP_CLIENT_H_

#include <QTcpSocket>
#include "../JZCommPack.h"

class JZTcpClient : public QObject
{
    Q_OBJECT

public:
    JZTcpClient(QObject *parent = nullptr);
    ~JZTcpClient();

    void connectToHost(QString ip, int port);
    void disconnectFromHost();

    void write(const QByteArray &buffer);
    QByteArray read();

    void writeText(const QString &buffer);
    QString readText();

protected:
    JZCommPack m_pack;
    QTcpSocket *m_socket;
};



#endif