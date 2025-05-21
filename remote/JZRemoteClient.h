#ifndef JZ_REMOTE_CLIENT_H_
#define JZ_REMOTE_CLIENT_H_

#include <QObject>
#include "3rd/JZCommon/jzNet/JZNetClient.h"
#include "JZRemotePacket.h"

class JZRemoteClient : public QObject
{
    Q_OBJECT

public:
    JZRemoteClient();
    ~JZRemoteClient();

    bool connectToServer(QString ip,int port);
    void disconnectFromServer();
    bool isConnect();

    bool startProgram(JZNodeProgram *program);
    bool stopProgram();

signals:
    void sigNetError();
    void sigConnect();
    void sigDisConnect();       

protected slots:    
    void onConnect();
	void onDisConnect();
	void onNetPackRecv(JZNetPackPtr ptr);    

protected:        
    JZNetClient m_client;    
};


#endif
