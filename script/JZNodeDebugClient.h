#ifndef JZNODE_DEBUG_CLIENT_H_
#define JZNODE_DEBUG_CLIENT_H_

#include <QObject>
#include "JZNetClient.h"
#include "JZNodeDebugPacket.h"

class JZNodeDebugClient : public QObject
{
    Q_OBJECT

public:
    JZNodeDebugClient();
    ~JZNodeDebugClient();

    bool pause();
    bool resume();
    bool stop();

protected slots:    
    void onConnect();
	void onDisConnect();
	void onNetPackRecv(JZNetPackPtr ptr);    

protected:
    bool send();

    JZNetClient m_client;
};


#endif