#include <QTest>
#include <QFile>
#include <QTextStream>
#include "test_unitTest.h"
#include "JZNodeBuilder.h"
#include "JZNodeUtils.h"
#include "JZScriptUnitTest.h"
#include "modules/camera/JZCameraNode.h"
#include "modules/camera/JZCameraFile.h"

TestUnitTest::TestUnitTest()
{
    
}

void TestUnitTest::dumpUnit(JZScriptUnitTest* unit,QString name)
{
    QString path = qApp->applicationDirPath() + "/dump/" + name;
    unit->dump(path);
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

    unit.setScript(add_script);
    JZScriptItemDepend *ptr = unit.depend();
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

    unit.setScript(flow);
    JZScriptItemDepend *ptr = unit.depend();

    JZNodeTimerEvent* event = new JZNodeTimerEvent();
    event->setTimeOut(100);
    flow->addNode(event);

    bool ret = unit.run();
    QVERIFY(ret);

    dumpUnit(&unit, "unit_testTimerEvent");
}

void TestUnitTest::testCameraEvent()
{
    auto class_item = makeTestClass();
    class_item->addMemberVariable("cameraManager", "JZCameraManager");

    //init func
    JZFunctionDefine define = class_item->objectDefine().initMemberFunction("init");
    auto script_init = class_item->addMemberFunction(define);

    JZCameraFileConfig *cam_config = new JZCameraFileConfig();
    cam_config->name = "camera";
    cam_config->type = Camera_File;
    cam_config->path = "C:/Users/xiong/Desktop/demo/image";

    JZCameraManagerConfig config;
    config.cameraList << JZCameraConfigEnum(cam_config);

    JZNodeCameraInit* node_init = new JZNodeCameraInit();
    node_init->setConfig(config);
    script_init->addNode(node_init);
    script_init->addConnect(script_init->startNode()->flowOutGemo(), node_init->flowInGemo());

    //onFrame
    auto flow = class_item->addFlow("onFrame");

    JZScriptUnitTest unit;
    unit.setProject(&m_project);

    JZNodeCameraReadyEvent* event = new JZNodeCameraReadyEvent();
    flow->addNode(event);

    unit.setScript(flow);
    JZScriptItemDepend* ptr = unit.depend();
    QVERIFY2(!ptr->isError(), qUtf8Printable(ptr->error));
    
    bool ret = unit.run();
    dumpUnit(&unit, "unit_testCameraEvent");

    QVERIFY(ret);
}

void test_unitTest(int argc, char *argv[])
{
    TestUnitTest test;
    QTest::qExec(&test,argc,argv);
}