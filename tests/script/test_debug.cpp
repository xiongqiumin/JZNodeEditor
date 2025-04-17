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

void DebugTest::cleanup()
{
    stopServer();
}

void DebugTest::testDebugServer()
{
    QString code = R"(int testFor(int n) {       
        int result = 0;
        for (int i = 0; i < n; i++) {
            result = i;
        }
        return result;        
    })";

    if(!buildAs(code))
        return;

    QVariantList in;
    in << 1000000;
    startServer("testFor", in);
   
    bool cmd_ret = false;
    for (int i = 0; i < 10; i++)
    {
        cmd_ret = m_client.pause();
        QVERIFY(cmd_ret);

        msleep(100);
        
        cmd_ret = m_client.resume();
        QVERIFY(cmd_ret);
    }

    stopServer();
}


void test_debug(int argc, char *argv[])
{    
    DebugTest s; 
    QTest::qExec(&s,argc,argv);
}
