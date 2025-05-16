#include <QEventLoop>
#include <QDebug>
#include <math.h>
#include <QApplication>
#include <QTest>
#include <QPointer>
#include <QScopeGuard>
#include "test_vision.h"
#include "modules/vision/JZVisionNode.h"
#include "modules/vision/JZVisonWindow.h"

//VisionTest
VisionTest::VisionTest()
{
}

void VisionTest::testTemplateMatch()
{
    auto class_item = makeTestClass();

    JZFunctionDefine func_test = class_item->objectDefine().initMemberFunction("testTemplate");

    JZScriptItem *script = class_item->addMemberFunction(func_test);
    auto start = script->startNode();

    JZNodeFunction* node_imread = new JZNodeFunction();
    node_imread->setFunction("imread");
    script->addNode(node_imread);

    QString file_path = qApp->applicationDirPath() + "/data/test.png";
    node_imread->setParamInValue(0, file_path);

    JZTemplateConfig temp_config;
    temp_config.templatePath = file_path;
    
    JZNodeVisionTemplateMatch* node_match = new JZNodeVisionTemplateMatch();
    node_match->setConfig(temp_config);
    script->addNode(node_match);
    script->addConnect(node_imread->paramOutGemo(0), node_match->paramInGemo(0));
    script->addConnect(start->flowOutGemo(0), node_match->flowInGemo());

    if (!build())
        return;

    dump("vision_testTemplateMatch");

    QVariantList in, out;
    callMember("testTemplate", in, out);
}

void VisionTest::testBrightnessDetector()
{
    JZFunctionDefine define;
    define.name = "testBrightnessDetector";
    auto script = m_file->addFunction(define);

    auto start = script->startNode();

    JZNodeFunction* node_imread = new JZNodeFunction();
    node_imread->setFunction("imread");
    script->addNode(node_imread);

    QString file_path = qApp->applicationDirPath() + "/data/test.png";
    node_imread->setParamInValue(0, file_path);

    JZNodeVisionBrightnessDetector* node_br = new JZNodeVisionBrightnessDetector();
    script->addNode(node_br);
    script->addConnect(node_imread->paramOutGemo(0), node_br->paramInGemo(0));
    script->addConnect(start->flowOutGemo(0), node_br->flowInGemo());

    if (!build())
        return;

    dump("vision_testBrightnessDetector");

    QVariantList in, out;
    call("testBrightnessDetector", in, out);
}

void VisionTest::testColorIdentify()
{
    JZFunctionDefine define;
    define.name = "testColorIdentify";
    auto script = m_file->addFunction(define);

    auto start = script->startNode();

    JZNodeFunction* node_imread = new JZNodeFunction();
    node_imread->setFunction("imread");
    script->addNode(node_imread);

    JZNodeVisionColorIdentify* node_color = new JZNodeVisionColorIdentify();
    script->addNode(node_color);
    script->addConnect(node_imread->paramOutGemo(0), node_color->paramInGemo(0));
    script->addConnect(node_imread->paramOutGemo(0), node_color->paramInGemo(1));
    script->addConnect(start->flowOutGemo(0), node_color->flowInGemo());
}

void VisionTest::testVisonWindow()
{
    JZVisonWindowConfig cfg;

    JZVisonWindow w;
    w.init(cfg);
    w.show();

    QTest::qWaitFor([&w]()->bool {
        return !w.isVisible();
    }, 100000);
}

void test_vision(int argc, char *argv[])
{    
    VisionTest s;
    QTest::qExec(&s,argc,argv);
}
