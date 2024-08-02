#include <QThread>
#include <QScopeGuard>
#include <QApplication>
#include "JZUpdateServer.h"
#include "JZUpdateClient.h"
#include "JZUpdateDialog.h"

class ServerThread : public QThread
{
public:
    virtual void run() override
    {
        QString path = qApp->applicationDirPath() + "/test/update/server";
        JZUpdateServer server;
        server.init(path,8888);

        exec();
    }
};

void test_update()
{
    JZUpdatePackRegist();

    ServerThread t;
    t.start();

    auto cleanup = qScopeGuard([&t] {
        t.quit();
        t.wait();
    });
    
    QString path = qApp->applicationDirPath() + "/test/update/client";
    JZUpdateClient client(path);
    if (!client.init("127.0.0.1", 8888))
    {
        qDebug() << "connect server failed";
        return;
    }
    
    if (!client.checkUpdate())
    {
        qDebug() << "no need update";
        return;
    }
    
    JZUpdateDialog dialog;
    dialog.setClient(&client);
    if (dialog.exec() != QDialog::Accepted)    
        return;
           
    if(client.isDownloadFinish())
        client.dealUpdate();
    client.clearCache();
}