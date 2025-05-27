#include "test_model.h"
#include "modules/opencv/JZModuleOpencv.h"
#include "modules/model/JZModuleModel.h"

//ModelTest
ModelTest::ModelTest()
{
}

void ModelTest::testYolo()
{
    auto class_item = makeTestClass();
    class_item->addMemberVariable("modelManager", "JZModelManager");

    JZFunctionDefine define = class_item->objectDefine().initMemberFunction("init");
    auto script_init = class_item->addMemberFunction(define);

    JZModelYoloConfig *yolo_config = new JZModelYoloConfig();
    yolo_config->name = "model";
    yolo_config->type = Model_Yolo;
    yolo_config->modelPath = "C:/Users/xiong/Desktop/demo/image/yolo.onnx";
    yolo_config->idPath = "C:/Users/xiong/Desktop/demo/image/yolo.json";

    JZModelManagerConfig config;
    config.modelList << JZModelConfigEnum(yolo_config);

    JZNodeModelInit *node_init = new JZNodeModelInit();
    node_init->setConfig(config);
    script_init->addNode(node_init);
    script_init->addConnect(script_init->startNode()->flowOutGemo(), node_init->flowInGemo());

    if (!build())
        return;
    dump("test_CameraFile");

    QVariantList in, out;
    callMember("init", in, out);
}

void test_model(int argc, char *argv[])
{
    ModelTest s;
    QTest::qExec(&s, argc, argv);
}
