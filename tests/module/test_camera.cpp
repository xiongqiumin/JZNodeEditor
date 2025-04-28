#include "test_camera.h"
#include "modules/camera/JZModuleCamera.h"

//CameraTest
CameraTest::CameraTest()
{
}

void CameraTest::testFile()
{
    auto class_item = makeTestClass();
    class_item->addMemberVariable("cameraManager", "JZCameraManager");

    JZFunctionDefine define = class_item->objectDefine().initMemberFunction("init");
    auto script_init = class_item->addMemberFunction(define);

    JZCameraConfig cam_config;
    cam_config.name = "camera";
    cam_config.type = Camera_File;
    cam_config.path = "C:/Users/xiong/Desktop/demo/image";

    JZCameraManagerConfig config;
    config.cameraList << cam_config;

    JZNodeCameraInit *node_init = new JZNodeCameraInit();
    node_init->setConfig(config);
    script_init->addNode(node_init);
    script_init->addConnect(script_init->startNode()->flowOutGemo(), node_init->flowInGemo());

    JZNodeCameraStart* camera_start = new JZNodeCameraStart();
    script_init->addNode(camera_start);
    script_init->addConnect(node_init->flowOutGemo(), camera_start->flowInGemo());

    auto flow = class_item->addFlow("cameraFlow");

    JZNodeCameraReadyEvent *node_frameReady = new JZNodeCameraReadyEvent();
    flow->addNode(node_frameReady);

    JZNodeFunction* test_node = new JZNodeFunction();
    flow->addNode(test_node);
    test_node->setFunction("JZTestLambda");
    flow->addConnect(node_frameReady->flowOutGemo(), test_node->flowInGemo());

    if (!build())
        return;
    dump("test_CameraFile");

    QVariantList in, out;
    callMember("init", in, out);

    bool isRecv = false;
    g_testFunc = [&isRecv] {
        isRecv = true;
    };

    bool wait_ret = QTest::qWaitFor([&isRecv]()->bool {
        return isRecv;
    });
    QVERIFY(wait_ret);
}

void test_camera(int argc, char *argv[])
{
    CameraTest s;
    QTest::qExec(&s, argc, argv);
}
