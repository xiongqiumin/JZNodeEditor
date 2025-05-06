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
    JZCommConfig cfg;

    JZCommManagerConfig comm_config;    
    cfg.commType = Comm_ModbusTcpClient;
    cfg.modbus.conn.modbusType = Modbus_tcpClient;
    cfg.name = "modbus";
    comm_config.commList << cfg;

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

void ModbusTest::testClient()
{
    auto class_item = makeTestClass();
    class_item->addMemberVariable("commManager", "JZCommManager");

    JZFunctionDefine define = class_item->objectDefine().initMemberFunction("testFunction");    
    class_item->addMemberFunction(define);

    auto script = class_item->memberFunction("testFunction");
    auto start = script->startNode();

    JZCommConfig cfg;
    cfg.commType = Comm_ModbusTcpClient;

    JZCommModbusInfo modbus;
    cfg.modbus.conn.modbusType = Modbus_tcpClient;
    cfg.name = "modbus";

    JZCommManagerConfig comm_config;
    comm_config.commList << cfg;

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
