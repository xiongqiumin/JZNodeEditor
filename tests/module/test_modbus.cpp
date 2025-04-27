#include <QEventLoop>
#include <QDebug>
#include <math.h>
#include <QApplication>
#include <QTest>
#include <QPointer>
#include "JZNodeFactory.h"
#include "test_modbus.h"
#include "modules/communication/JZModuleComm.h"

ModbusTest::ModbusTest()
{
}

void ModbusTest::testClient()
{
    auto class_item = makeTestClass();
    class_item->addMemberVariable("commManager", "JZCommManager");

    auto script = class_item->memberFunction("testFunction");
    auto start = script->startNode();

    JZNodeCommInit* comm_init = new JZNodeCommInit();
    script->addNode(comm_init);
    script->addConnect(start->flowOutGemo(), comm_init->flowInGemo());

    JZNodeModbusWrite *modbus_write = new JZNodeModbusWrite();
    script->addNode(modbus_write);
    script->addConnect(comm_init->flowOutGemo(), modbus_write->flowInGemo());

    JZNodeModbusRead *modbus_read = new JZNodeModbusRead();
    script->addNode(modbus_read);
    script->addConnect(modbus_write->flowOutGemo(), modbus_read->flowInGemo());

    if (!build())
        return;
    dump("modbus_testClient");
      
    QVariantList in, out;    
    callMember("testFunction",in,out);
}

void test_modbus(int argc, char *argv[])
{    
    ModbusTest s; 
    QTest::qExec(&s,argc,argv);
}
