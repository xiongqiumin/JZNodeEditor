#ifndef JZ_TCP_SERVER_H_
#define JZ_TCP_SERVER_H_

#include <QTcpServer>

class JZTcpServer : public QTcpServer
{
    Q_OBJECT
    
public:
    JZTcpServer();
    ~JZTcpServer();







};

#endif