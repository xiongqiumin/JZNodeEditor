#include "test_model.h"
#include "modules/opencv/JZModuleOpencv.h"
#include "modules/model/JZModuleModel.h"
#include "modules/model/JZYolo.h"

//ModelTest
ModelTest::ModelTest()
{
}

void ModelTest::testYolo()
{
    JZModelConfigEnum config = JZModelConfigEnum(new JZModelYoloConfig());
    JZModelYoloConfig *yolo_cfg = (JZModelYoloConfig*)config.data();
    yolo_cfg->backend = Model_BackendCpu;
    yolo_cfg->modelPath = "C:/Users/xiong/Desktop/JZNodeEditorTest/data/yolov8n.onnx";
    yolo_cfg->idPath = "C:/Users/xiong/Desktop/JZNodeEditorTest/data/yolov8n.json";

    JZYolo yolo_cpu, yolo_tensor_rt;
    yolo_cpu.setConfig(config);

    yolo_cfg->backend = Model_BackendTensorRT;
    yolo_cfg->modelPath = "C:/Users/xiong/Desktop/JZNodeEditorTest/data/yolov8n.engine";
    yolo_tensor_rt.setConfig(config);

    bool ret = yolo_cpu.init();
    QVERIFY(ret);

    ret = yolo_tensor_rt.init();
    QVERIFY(ret);

    cv::Mat mat = imread("C:/Users/xiong/Desktop/JZNodeEditorTest/data/111111111.jpg");

    m_benchmark.setTestSec(10);

    JZBENCHMARK(yolo_cpu)
    {
        yolo_cpu.forward(mat);
    }

    JZBENCHMARK(yolo_tensor_rt)
    {
        yolo_tensor_rt.forward(mat);
    }
    
    m_benchmark.report();
}

void test_model(int argc, char *argv[])
{
    ModelTest s;
    QTest::qExec(&s, argc, argv);
}
