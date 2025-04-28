#include <QEventLoop>
#include <QDebug>
#include <math.h>
#include <QApplication>
#include <QTest>
#include <QPointer>
#include <QScopeGuard>
#include "JZNodeFactory.h"
#include "test_modbus.h"
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

//ModbusTest
ModbusTest::ModbusTest()
{
}

void ModbusTest::testClientCpp()
{
    JZCommConfig comm_config;
    JZCommModbusInfo modbus;
    modbus.conn.modbusType = Modbus_tcpClient;
    modbus.name = "modbus";
    comm_config.modbusClient << modbus;

    JZCommManager manager;
    JZCommInit(&manager, JZNodeUtils::toBuffer(comm_config));

    ModbusThread t;
    t.start();
    QTest::qWait(200);
    auto cleanup = qScopeGuard([&t]{ 
        t.quit();
        t.wait();
    });

    QJsonObject param;
    param["addr"] = 40000;
    param["function"] = Function_Register;
    param["dataType"] = "double";
            
    JZVariantAny ret_any;

    JZCommModbusWrite(&manager, "modbus", param, JZVariantAny::fromValue<int32_t>(-1));
    ret_any = JZCommModbusRead(&manager, "modbus", param);
    QCOMPARE(ret_any.variant.toInt(), -1);

    JZCommModbusWrite(&manager, "modbus", param, JZVariantAny::fromValue<uint32_t>(-1));
    ret_any = JZCommModbusRead(&manager, "modbus", param);
    QCOMPARE(ret_any.variant.toUInt(), -1);


    JZCommModbusWrite(&manager, "modbus", param, JZVariantAny::fromValue(0.6));
    ret_any = JZCommModbusRead(&manager, "modbus", param);
    QCOMPARE(ret_any.variant.toDouble(),0.6);       
}

void ModbusTest::testClient()
{
    auto class_item = makeTestClass();
    class_item->addMemberVariable("commManager", "JZCommManager");

    JZFunctionDefine define = class_item->objectDefine().initMemberFunction("testFunction");    
    class_item->addMemberFunction(define);

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
    auto cleanup = qScopeGuard([&t] {
        t.quit();
        t.wait();
    });

    QVariantList in, out;
    for (int i = 0; i < 50; i++)
        callMember("testFunction", in, out);   
}

void test_modbus(int argc, char *argv[])
{    
    ModbusTest s; 
    QTest::qExec(&s,argc,argv);
}
