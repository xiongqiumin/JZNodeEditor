#include "test_camera.h"
#include "modules/camera/JZModuleCamera.h"

//CameraTest
CameraTest::CameraTest()
{
}

void CameraTest::testHik()
{
    auto class_item = makeTestClass();
    class_item->addMemberVariable("commManager", "JZCommManager");

    JZFunctionDefine define = class_item->objectDefine().initMemberFunction("init");
    auto script_init = class_item->addMemberFunction(define);

    JZCameraConfig cam_config;
    cam_config.name = "camera";
    cam_config.type = Camera_File;

    JZCameraManagerConfig config;
    config.cameraList << cam_config;

    JZNodeCameraInit *node_init = new JZNodeCameraInit();
    node_init->setConfig(config);
    script_init->addNode(node_init);
    script_init->addConnect(script_init->startNode()->flowOutGemo(), node_init->flowInGemo());

    auto flow = class_item->addFlow("cameraFlow");

    JZNodeCameraReadyEvent *node_frameReady = new JZNodeCameraReadyEvent();
    flow->addNode(node_frameReady);

    if (!build())
        return;

    QVariantList in, out;
    callMember("init", in, out);
}

void test_camera(int argc, char *argv[])
{
    CameraTest s;
    QTest::qExec(&s, argc, argv);
}
