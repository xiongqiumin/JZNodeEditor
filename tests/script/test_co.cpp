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

void CoTest::testException()
{
    if (!build())
        return;

    g_testFunc = [] {
        auto co = g_engine->currentCo();
        if (co->id != 0)
        {
            jzco_sleep(1000);
            throw std::runtime_error("test execption");
        }
        else
        {
            jzco_sleep(4000);
            printf("co finished\n");
        }
    };

    m_engine.blockSignals(true);
    QList<JZCoCoroutinePtr> co_list;
    for (int i = 0; i < 5; i++)
    {
        JZCoCoroutinePtr co_ptr = JZCoCoroutinePtr(new JZEngineCoroutine(&m_engine));
        co_ptr->setTask([this] {
            QVariantList in, out;
            call("JZTestLambda", in, out);
        });
        co_list << co_ptr;
        jzco_spawn(co_ptr);
    }
    
    jzco_waitCoroutine(co_list);
    m_engine.blockSignals(false);
}

void CoTest::testABSwitch()
{
    if (!build())
        return;

    JZCoCoroutinePtr co1 = JZCoCoroutinePtr(new JZEngineCoroutine(&m_engine));
    JZCoCoroutinePtr co2 = JZCoCoroutinePtr(new JZEngineCoroutine(&m_engine));

    auto task1 = co1.data();
    auto task2 = co2.data();
    co1->setTask([task2] {
        for (int i = 0; i < 5; i++)
        {
            g_scheduler->resume(task2);
            jzco_sleep(200);
            qDebug() << "task1" << i;
        }
    });
    co2->setTask([task1] {
        for (int i = 0; i < 5; i++)
        {
            g_scheduler->resume(task1);
            jzco_sleep(200);
            qDebug() << "task2" << i;
        }
    });

    jzco_spawn(co1);
    jzco_spawn(co2);

    jzco_waitCoroutine({ co1 ,co2 });
}

void CoTest::testThread()
{
    JZCoCoroutinePtr co1 = JZCoCoroutinePtr(new JZCoCoroutine());
    JZCoCoroutinePtr co2 = JZCoCoroutinePtr(new JZCoCoroutine());

    co1->setTask([] {
        QThread::msleep(2000);
    });
    co2->setTask([] {
        jzco_waitThread([] {
            QThread::msleep(2000);
        });
    });

    jzco_spawn(co1);
    jzco_spawn(co2);

    jzco_waitCoroutine({ co1 ,co2 });
}

void test_co(QStringList testcase)
{   
    CallArg call = genCallUnitArg(testcase);

    CoTest s; 
    QTest::qExec(&s, call.argc, call.argv.data());
}
