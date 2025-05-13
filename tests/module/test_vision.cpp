#include <QEventLoop>
#include <QDebug>
#include <math.h>
#include <QApplication>
#include <QTest>
#include <QPointer>
#include <QScopeGuard>
#include "test_vision.h"
#include "modules/vision/JZVisionNode.h"

//VisionTest
VisionTest::VisionTest()
{
}

void VisionTest::testTemplateMatch()
{
    JZFunctionDefine func_test;
    func_test.name = "testTemplate";

    JZScriptItem *script = m_file->addFunction(func_test);

    JZNodeFunction* node_imread = new JZNodeFunction();
    node_imread->setFunction("imread");
    script->addNode(node_imread);
}

void test_vision(int argc, char *argv[])
{    
    VisionTest s;
    QTest::qExec(&s,argc,argv);
}
