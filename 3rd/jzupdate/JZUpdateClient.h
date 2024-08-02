#ifndef JZ_UPDATE_CLINET_H
#define JZ_UPDATE_CLINET_H

#include <QThread>
#include "JZNetClient.h"
#include "JZUpdatePack.h"

class JZUpdateClient : public QObject
{
	Q_OBJECT

public:    
    JZUpdateClient(QString path);
	~JZUpdateClient();

    bool init(QString ip, int port);
    
    bool checkUpdate();
    bool downloadUpdate();
    bool isDownloadFinish();
    bool dealUpdate();    
    void clearCache();  

    bool isDownload();
    void cancel();

signals:
    void sigProgess(int step, QString tips);
    void sigError(QString error);

protected:
    QString downloadPath();
    QString versionFile();
    QString finishFile();
    QString renameFile();
    void saveVersion();
    void saveFinish();

    QMap<QString, QString> m_localFile;
    QMap<QString, QString> m_remoteFile;
    
    bool m_download;
    JZNetClient *m_client;    
    QString m_path;
};

#endif // UPDATE_H
