#ifndef JZTCP_CLIENT_H_
#define JZTCP_CLIENT_H_

#include <QTcpSocket>
#include "../JZCommPack.h"

//JZTcpClientInfo
class JZTcpClientInfo
{
public:
    JZTcpClientInfo();

    QString ip;
    int port;

};
QDataStream& operator<<(QDataStream& s, const JZTcpClientInfo& param);
QDataStream& operator>>(QDataStream& s, JZTcpClientInfo& param);

//JZTcpClient
class JZTcpClient : public QObject
{
    Q_OBJECT

public:
    JZTcpClient(QObject *parent = nullptr);
    ~JZTcpClient();

    void init(JZTcpClientInfo info);
    void setFormat(JZCommPackFormat format);

    bool isOpen();
    bool open();
    void close();

    void write(const QByteArray &buffer);
    QByteArray read();

    void writeText(const QString &buffer);
    QString readText();

protected:
    JZTcpClientInfo m_info;
    JZCommPack m_pack;
    QTcpSocket *m_socket;
};



#endif