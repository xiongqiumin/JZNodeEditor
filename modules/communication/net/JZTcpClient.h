#ifndef JZTCP_CLIENT_H_
#define JZTCP_CLIENT_H_

#include <QTcpSocket>
#include "../JZCommPack.h"
#include "../JZComm.h"

//JZCommTcpClientConfig
class JZCommTcpClientConfig : public JZCommConfig
{
public:
    JZCommTcpClientConfig();

    virtual void saveToStream(QDataStream& s) const;
    virtual void loadFromStream(QDataStream& s);

    QString ip;
    int port;
    JZCommPackFormat format;
};

//JZTcpClient
class JZTcpClient : public JZCommObject
{
    Q_OBJECT

public:
    JZTcpClient(QObject *parent = nullptr);
    ~JZTcpClient();

    virtual bool isOpen() override;
    virtual bool open() override;
    virtual void close() override;

    void write(const QByteArray &buffer);
    QByteArray read();

    void writeText(const QString &buffer);
    QString readText();

protected:
    JZCommPack m_pack;
    QTcpSocket *m_socket;
};



#endif