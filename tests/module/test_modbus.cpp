#include <QEventLoop>
#include <QDebug>
#include <math.h>
#include <QApplication>
#include <QTest>
#include <QPointer>
#include "JZNodeFactory.h"
#include "test_modbus.h"
#include "modules/communication/JZModuleComm.h"

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

//ModbusTest
ModbusTest::ModbusTest()
{
}

void ModbusTest::testClient()
{
    auto class_item = makeTestClass();
    class_item->addMemberVariable("commManager", "JZCommManager");

    auto script = class_item->memberFunction("testFunction");
    auto start = script->startNode();

    JZCommConfig comm_config;
    JZCommModbusInfo modbus;
    modbus.conn.modbusType = Modbus_tcpClient;
    modbus.name = "modbus";
    comm_config.modbusClient << modbus;

    JZNodeCommInit* comm_init = new JZNodeCommInit();
    script->addNode(comm_init);
    comm_init->setConfig(comm_config);
    script->addConnect(start->flowOutGemo(), comm_init->flowInGemo());

    JZNodeModbusWrite *modbus_write = new JZNodeModbusWrite();
    modbus_write->setClient("modbus");
    modbus_write->setParamInValue(1, "100");
    script->addNode(modbus_write);
    script->addConnect(comm_init->flowOutGemo(), modbus_write->flowInGemo());

    JZNodeModbusRead *modbus_read = new JZNodeModbusRead();
    modbus_read->setClient("modbus");
    script->addNode(modbus_read);
    script->addConnect(modbus_write->flowOutGemo(), modbus_read->flowInGemo());

    if (!build())
        return;
    dump("modbus_testClient");

    ModbusThread t;
    t.start();
    QTest::qWait(200);

    QVariantList in, out;    
    for(int i = 0; i < 10; i++)
        callMember("testFunction",in,out);

    QTest::qWait(1000);
    t.quit();
    t.wait();
}

void test_modbus(int argc, char *argv[])
{    
    ModbusTest s; 
    QTest::qExec(&s,argc,argv);
}
