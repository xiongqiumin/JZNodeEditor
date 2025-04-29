#include <QTest>
#include <QFile>
#include <QTextStream>
#include "test_unitTest.h"
#include "JZNodeBuilder.h"
#include "JZNodeUtils.h"
#include "JZScriptUnitTest.h"
#include "modules/camera/JZCameraNode.h"

TestUnitTest::TestUnitTest()
{
    
}

void TestUnitTest::testHello()
{
    QString code = R"(int add(int a,int b)
        {
            return a + pow(2,4) + b;
        }
    )";    

    if(!buildAs(code))
        return;
    m_engine.deinit();

    JZScriptItem *add_script = m_file->getFunction("add");

    JZScriptUnitTest unit;
    unit.setProject(&m_project);

    JZScriptItemDepend *ptr = unit.genDepend(add_script);
    ptr->functionList[0].value = 800.0;
    ptr->input << 1 << 2;

    bool ret = unit.run();
    QVERIFY(ret);    
    QCOMPARE(ptr->output[0].toInt(), 803);
}

void TestUnitTest::testTimerEvent()
{
    auto class_item = makeTestClass();
    auto flow = class_item->addFlow("onTimer");

    JZScriptUnitTest unit;
    unit.setProject(&m_project);

    JZScriptItemDepend* ptr = unit.genDepend(flow);

    JZNodeTimerEvent* event = new JZNodeTimerEvent();
    event->setTimeOut(100);
    flow->addNode(event);

    bool ret = unit.run();
    QVERIFY(ret);
}

void TestUnitTest::testCameraEvent()
{
    auto class_item = makeTestClass();
    class_item->addMemberVariable("cameraManager", "JZCameraManager");

    auto flow = class_item->addFlow("onFrame");

    JZScriptUnitTest unit;
    unit.setProject(&m_project);

    JZScriptItemDepend* ptr = unit.genDepend(flow);

    JZNodeCameraReadyEvent* event = new JZNodeCameraReadyEvent();
    flow->addNode(event);

    bool ret = unit.run();
    QVERIFY(ret);
}

void test_unitTest(int argc, char *argv[])
{
    TestUnitTest test;
    QTest::qExec(&test,argc,argv);
}