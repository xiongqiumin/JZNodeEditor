#ifndef JZNODE_DEBUG_CLIENT_H_
#define JZNODE_DEBUG_CLIENT_H_

#include <QObject>
#include <QTcpClient>
#include "JZNodeDebugPacket.h"

class JZNodeDebugClient : public QObject
{
    Q_OBJECT

public:
    JZNodeDebugClient();
    ~JZNodeDebugClient();

protected:
    bool waitPacket(int id,JZNodeDebugPacket &recv);
    void sendPacket(JZNodeDebugPacket packet);
};


#endif