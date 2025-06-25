#include <QEventLoop>
#include <QDebug>
#include <math.h>
#include <QApplication>
#include <QTest>
#include <QPointer>
#include <QScopeGuard>
#include "JZNodeFactory.h"
#include "test_comm.h"
#include "modules/communication/JZModuleComm.h"
#include "JZNodeUtils.h"

class ModbusThread : public QThread
{
    virtual void run() override
    {
        JZModbusConnetInfo conn;
        conn.modbusType = Modbus_tcpServer;

        JZModbusServer server;
        server.initMapping(100, 100, 100, 100);
        server.mapping()->start_registers = 40000;

        server.initConn(conn);
        server.start();

        exec();
    }
};

//TcpThread
TcpThread::TcpThread()
{
    moveToThread(this);
}

void TcpThread::run()
{
    JZTcpServer server;
    connect(&server, &JZTcpServer::sigNetPackRecv, this, &TcpThread::onPackRecv);

    JZCommTcpServerConfig *info = new JZCommTcpServerConfig();
    info->port = 5000;
    server.setConfig(JZCommConfigEnum(info));
    server.open();

    exec();
}
 
void TcpThread::onPackRecv(int netId, const QByteArray& buffer)
{
    JZTcpServer* server = qobject_cast<JZTcpServer*>(sender());
    server->sendPack(netId,buffer);
}

//UdpThread
UdpThread::UdpThread()
{
    moveToThread(this);
}

void UdpThread::run()
{
    JZUdpSocket server;
    connect(&server, &JZUdpSocket::sigDataRecv, this, &UdpThread::onPackRecv);

    JZCommUdpConfig info;
    info.port = 15000;
    server.init(info);
    server.open();

    exec();
}

void UdpThread::onPackRecv(const QNetworkDatagram& data)
{
    JZUdpSocket* server = qobject_cast<JZUdpSocket*>(sender());
    server->write(data.data(), data.senderAddress().toString(), data.senderPort());
}

//CommTest
CommTest::CommTest()
{
}

void CommTest::testModbusClientCpp()
{
    JZCommModbusTcpClientConfig*cfg = new JZCommModbusTcpClientConfig();

    JZCommManagerConfig comm_config;    
    cfg->type = Comm_ModbusTcpClient;
    cfg->conn.modbusType = Modbus_tcpClient;
    cfg->name = "modbus";
    comm_config.commList << JZCommConfigEnum(cfg);

    JZCommManager manager;
    JZCommInit(&manager, JZNodeUtils::toBuffer(comm_config));

    ModbusThread t;
    t.start();
    QTest::qWait(200);
    auto cleanup = qScopeGuard([&t]{ 
        t.quit();
        t.wait();
    });

    int addr = 40000;
    int function = Function_Register;
    QString dataType = "double";
            
    JZVariantAny in_any;
    JZVariantAny ret_any;

    in_any.variant = QVariant::fromValue(0.6);
    JZCommModbusWrite(&manager, "modbus", function, dataType, addr, in_any);
    ret_any = JZCommModbusRead(&manager, "modbus", function, dataType, addr);
    QCOMPARE(ret_any.variant.toDouble(),0.6);       
}

void CommTest::testModbusClient()
{
    auto class_item = makeTestClass();
    class_item->addMemberVariable("commManager", "JZCommManager");

    JZFunctionDefine define = class_item->objectDefine().initMemberFunction("testFunction");    
    class_item->addMemberFunction(define);

    auto script = class_item->memberFunction("testFunction");
    auto start = script->startNode();

    JZCommModbusTcpClientConfig* cfg = new JZCommModbusTcpClientConfig();
    cfg->type = Comm_ModbusTcpClient;
    cfg->conn.modbusType = Modbus_tcpClient;
    cfg->name = "modbus";

    JZCommManagerConfig comm_config;
    comm_config.commList << JZCommConfigEnum(cfg);

    JZNodeCommInit* comm_init = new JZNodeCommInit();
    script->addNode(comm_init);
    comm_init->setConfig(comm_config);
    script->addConnect(start->flowOutGemo(), comm_init->flowInGemo());

    JZNodeModbusWrite *modbus_write = new JZNodeModbusWrite();
    modbus_write->setName("modbus");
    modbus_write->setParamInValue(2, "100");
    script->addNode(modbus_write);
    script->addConnect(comm_init->flowOutGemo(), modbus_write->flowInGemo());

    JZNodeModbusRead *modbus_read = new JZNodeModbusRead();
    modbus_read->setName("modbus");
    script->addNode(modbus_read);
    script->addConnect(modbus_write->flowOutGemo(), modbus_read->flowInGemo());

    if (!build())
        return;
    dump("modbus_testClient");

    ModbusThread t;
    t.start();
    QTest::qWait(200);
    auto cleanup = qScopeGuard([&t] {
        t.quit();
        t.wait();
    });

    QVariantList in, out;
    for (int i = 0; i < 50; i++)
        callMember("testFunction", in, out);   
}

void CommTest::testTcpCpp()
{
    TcpThread t;
    t.start();
    QTest::qWait(200);
    auto cleanup = qScopeGuard([&t] {
        t.quit();
        t.wait();
        });

    JZCommTcpClientConfig* cfg = new JZCommTcpClientConfig();
    cfg->type = Comm_TcpClient;
    cfg->name = "tcpClient";
    cfg->port = 5000;

    JZCommManagerConfig comm_config;
    comm_config.commList << JZCommConfigEnum(cfg);

    JZCommManager manager;
    JZCommInit(&manager, JZNodeUtils::toBuffer(comm_config));

    QByteArray send("123456");
    JZCommTcpWrite(&manager, "tcpClient", send);
    QByteArray recv = JZCommTcpRead(&manager, "tcpClient");

    QCOMPARE(send, recv);
}

void CommTest::testUdpCpp()
{
    UdpThread t;
    t.start();
    QTest::qWait(200);
    auto cleanup = qScopeGuard([&t] {
        t.quit();
        t.wait();
        });

    JZCommUdpConfig* cfg = new JZCommUdpConfig();
    cfg->type = Comm_Udp;
    cfg->name = "udpClient";
    
    JZCommManagerConfig comm_config;
    comm_config.commList << JZCommConfigEnum(cfg);

    JZCommManager manager;
    JZCommInit(&manager, JZNodeUtils::toBuffer(comm_config));

    QByteArray send("123456");
    JZCommUdpWrite(&manager, "udpClient", send, "127.0.0.1", 15000);
    QByteArray recv = JZCommUdpRead(&manager, "udpClient");

    QCOMPARE(send, recv);
}

void CommTest::testComCpp()
{
}

void test_comm(int argc, char *argv[])
{    
    CommTest s; 
    QTest::qExec(&s,argc,argv);
}
