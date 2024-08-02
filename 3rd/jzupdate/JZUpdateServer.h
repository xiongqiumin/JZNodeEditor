#ifndef JZ_UPDATE_SERVER_H
#define JZ_UPDATE_SERVER_H

#include <QFile>
#include <QSharedPointer>
#include "JZNetServer.h"
#include "JZUpdatePack.h"

class JZUpdateServer : public QObject
{
	Q_OBJECT

public:
    JZUpdateServer();
	~JZUpdateServer();

	void init(QString path,int port);	

protected slots:
    void onConnect(int id);
    void onDisconnect(int id);
    void onPacketRecv(int id, JZNetPackPtr ptr);

protected:
    struct ClientInfo
    {        
        int offset;
        QSharedPointer<QFile> file;
    };    

    QString m_path;
    QMap<QString, QString> m_fileMap;
    QMap<int, ClientInfo> m_client;
    JZNetServer m_server;
};

#endif // UPDATE_H
