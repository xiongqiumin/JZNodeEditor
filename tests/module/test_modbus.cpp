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

    m_project.registType();

    if (!build())
        return;
    dump("modbus_testClient");
  
    bool result = QTest::qWaitFor([this]()->bool 
    { 
        return true;
    }, 1000);

    m_engine.deinit();
    QTest::qWait(4000);
}

void test_modbus(int argc, char *argv[])
{    
    ModbusTest s; 
    QTest::qExec(&s,argc,argv);
}
