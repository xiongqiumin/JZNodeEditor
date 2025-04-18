#include <QEventLoop>
#include <QDebug>
#include <math.h>
#include <QApplication>
#include <QTest>
#include "JZNodeFactory.h"
#include "test_debug.h"

DebugTest::DebugTest()
{
}

void DebugTest::startServer(QString func, QVariantList input)
{
    m_engine.setDebug(true);

    m_server.setEngine(&m_engine);
    if (!m_server.startServer(18888))
    {
        QVERIFY2(false, "start server failded");
    }

    if (!m_client.connectToServer("127.0.0.1", 18888))
    {
        QVERIFY2(false, "connect to server failded");
    }
    QThread::msleep(100);

    JZNodeDebugInfo init_info;
    JZNodeProgramInfo ret;
    bool cmd_ret = m_client.init(init_info, ret);
    QVERIFY(cmd_ret);

    cmd_ret = m_server.waitForAttach(500);
    QVERIFY(cmd_ret);

    callAsync(func, input);
}

void DebugTest::stopServer()
{
    m_client.stop();
    m_server.stopServer();
}

void DebugTest::clearTestCase()
{
    BaseTest::clearTestCase();
    stopServer();
}

void DebugTest::testDebugServer()
{
    QString code = R"(int testWhile(int n) {
        int result = 0;
        int i = 0;
        while(i < n) {
            result = i;
            i = i + 1;
        }
        return result;
    })";

    if(!buildAs(code))
        return;
    dump("debug_testWhile");

    QVariantList in;
    in << 100;
    startServer("testWhile", in);
   
    bool cmd_ret = false;
    for (int i = 0; i < 10; i++)
    {
        cmd_ret = m_client.pause();
        QVERIFY(cmd_ret);

        msleep(10);
        
        cmd_ret = m_client.resume();
        QVERIFY(cmd_ret);

        msleep(50);
    }

    stopServer();

    QVERIFY(m_thread.output.size() == 1);
    QCOMPARE(m_thread.output[0].toInt(), 99);
}


void test_debug(int argc, char *argv[])
{    
    DebugTest s; 
    QTest::qExec(&s,argc,argv);
}
