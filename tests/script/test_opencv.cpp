#include <QTest>
#include <math.h>
#include <functional>
#include "test_opencv.h"


//OpencvTest
OpencvTest::OpencvTest()
{

}

void OpencvTest::testCamera()
{   
    m_project.addGlobalVariable("camera", "JZCameraFile");

    auto env = m_project.environment();
    auto meta = env->meta("JZCameraFile");

    JZFunctionDefine func = JZSlotFunctionDefine("camera", meta->signal("sigFrameReady"));
    JZScriptFile* main_file = m_project.mainFile();
    JZScriptItem* slot = main_file->addFunction(func);
    auto slot_start = slot->startNode();

    JZNodePrint* node_print = new JZNodePrint();
    node_print->setParamInValue(0, "hello");
    slot->addNode(node_print);
    slot->addConnect(slot_start->flowOutGemo(), node_print->flowInGemo());

    //main
    JZScriptItem* main = m_project.mainFunction();
    auto start = main->startNode();

    JZNodeFunction* function_open = new JZNodeFunction();
    JZNodeFunction* function_start = new JZNodeFunction();
    main->addNode(function_open);
    main->addNode(function_start);

    function_open->setFunction(meta->function("open"));
    function_open->setVariable("camera");
    function_open->setParamInValue(1, "D:/work/qt/JZNodeEditor/Resources/icons");

    function_start->setFunction(meta->function("start"));
    function_start->setVariable("camera");

    main->addConnect(start->flowOutGemo(), function_open->flowInGemo());
    main->addConnect(function_open->flowOutGemo(), function_start->flowInGemo());

    if (!build())
        return;

    QVariantList in, out;
    m_engine.call("main", in, out);
}

void test_opencv(int argc, char *argv[])
{
    OpencvTest test;
    QTest::qExec(&test,argc,argv);
}