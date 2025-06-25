#include <QEventLoop>
#include <QDebug>
#include <math.h>
#include <QApplication>
#include <QTest>
#include <QEventLoop>
#include "JZNodeFactory.h"
#include "test_co.h"
#include "modules/communication/JZModuleComm.h"
#include "JZNodeUtils.h"

CoTest::CoTest()
{
}

void CoTest::testModbus()
{
    JZCoCoroutinePtr co_server = jzco_spawn([] {
        JZModbusConnetInfo conn;
        conn.modbusType = Modbus_tcpServer;

        JZModbusServer server;
        server.initMapping(100, 100, 100, 100);
        server.mapping()->start_registers = 40000;

        server.initConn(conn);
        server.start();

        jzco_sleep(5000);
     });

    JZCoCoroutinePtr co_client = jzco_spawn([] {
        JZCommModbusTcpClientConfig* cfg = new JZCommModbusTcpClientConfig();

        JZCommManagerConfig comm_config;
        cfg->type = Comm_ModbusTcpClient;
        cfg->conn.modbusType = Modbus_tcpClient;
        cfg->name = "modbus";
        comm_config.commList << JZCommConfigEnum(cfg);

        JZCommManager manager;
        JZCommInit(&manager, JZNodeUtils::toBuffer(comm_config));

        int addr = 40000;
        int function = Function_Register;
        QString dataType = "double";

        JZVariantAny in_any;
        JZVariantAny ret_any;

        in_any.variant = QVariant::fromValue(0.6);
        JZCommModbusWrite(&manager, "modbus", function, dataType, addr, in_any);
        ret_any = JZCommModbusRead(&manager, "modbus", function, dataType, addr);
    });

    jzco_waitCoroutine({ co_server,co_client });
}

void CoTest::testLoop()
{
    if (!build())
        return;

    QList<JZCoCoroutinePtr> co_list;
    for (int i = 0; i < 20; i++)
    {
        JZCoCoroutinePtr co_ptr = JZCoCoroutinePtr(new JZEngineCoroutine(&m_engine));
        co_ptr->setTask([this] {
            QVariantList in, out;
            in << 1000;
            call("sleep", in, out);
        });
        co_list << co_ptr;
        jzco_spawn(co_ptr);
    }

    QElapsedTimer t;
    t.start();
    jzco_waitCoroutine(co_list);
    QVERIFY(t.elapsed() < 1100);

    t.restart();
    for (int i = 0; i < 3; i++)
    {
        QVariantList in, out;
        in << 1000;
        call("sleep", in, out);
    }
    QVERIFY(t.elapsed() > 3000);
}


void test_co(int argc, char *argv[])
{    
    CoTest s; 
    QTest::qExec(&s,argc,argv);
}
