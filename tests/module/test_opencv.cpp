#include <QEventLoop>
#include <QDebug>
#include <math.h>
#include <QApplication>
#include <QTest>
#include <QPointer>
#include <QScopeGuard>
#include "test_opencv.h"
#include "modules/opencv/JZOpencvNode.h"

//OpencvTest
OpencvTest::OpencvTest()
{
}

void OpencvTest::testTemplateMatch()
{
    JZFunctionDefine func_test;
    func_test.name = "testTemplate";

    JZScriptItem *script = m_file->addFunction(func_test);

    JZNodeFunction* node_imread = new JZNodeFunction();
    node_imread->setFunction("imread");
    script->addNode(node_imread);

    JZNodeTemplateMatch* node_match = new JZNodeTemplateMatch();
    script->addNode(node_match);
    script->addConnect(node_imread->paramOutGemo(0), node_match->paramInGemo(0));
}

void test_opencv(int argc, char *argv[])
{    
    OpencvTest s;
    QTest::qExec(&s,argc,argv);
}
