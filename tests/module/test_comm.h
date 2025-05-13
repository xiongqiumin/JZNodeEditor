#ifndef TEST_COMM_H_
#define TEST_COMM_H_

#include <QObject>
#include <QThread>
#include "../script/test_base.h"

class TcpThread : public QThread
{
    Q_OBJECT

public:
    TcpThread();

protected slots:
    void onPackRecv(int netId, const QByteArray& buffer);

protected:
    virtual void run() override;
};

class QNetworkDatagram;
class UdpThread : public QThread
{
    Q_OBJECT

public:
    UdpThread();

protected slots:
    void onPackRecv(const QNetworkDatagram& data);

protected:
    virtual void run() override;
};

class CommTest : public BaseTest
{
    Q_OBJECT

public:
    CommTest();

private slots:
    void testModbusClientCpp();
    void testModbusClient();
    void testTcpCpp();
    void testUdpCpp();
    void testComCpp();

protected:
   

};

void test_comm(int argc, char *argv[]);

#endif
