#include <QApplication>
#include <QDir>
#include "VisionWindow.h"
#include "modules/vision/JZVisionUiItem.h"
#include "modules/vision/JZVisionWindow.h"
#include "modules/camera/JZCameraNode.h"
#include "modules/communication/JZCommNode.h"
#include "modules/model/JZModelNode.h"

SampleVisionWindow::SampleVisionWindow()
{        
    QFileInfo info(__FILE__);
    m_root = info.path();
}

SampleVisionWindow::~SampleVisionWindow()
{

}

void SampleVisionWindow::initProject(QString name)
{
    newProject(name,"vision");

    auto class_item = m_project.getClass("MainWindow");           

    addInit();
    addOnFrameReady();

    saveProject();
}

void SampleVisionWindow::initCameraHik()
{
    initProject("VisionWindowHik");
}

void SampleVisionWindow::initCameraFile()
{
    initProject("VisionWindowFile");
}

void SampleVisionWindow::initCameraRtsp()
{
    initProject("VisionWindowRtsp");
}

void SampleVisionWindow::addInit()
{
    auto class_item = m_project.getClass("MainWindow");
    auto script = class_item->memberFunction("init");
    class_item->addMemberVariable(JZParamDefine("cameraManager", "JZCameraManager"));
    class_item->addMemberVariable(JZParamDefine("commManager", "JZCommManager"));
    class_item->addMemberVariable(JZParamDefine("modelManager", "JZModelManager"));

    auto start_node = script->startNode();
    JZNodeCameraInit *cam_init = dynamic_cast<JZNodeCameraInit*>(script->findNodeByType(Node_CameraInit)[0]);
    JZNodeCommInit *comm_init = dynamic_cast<JZNodeCommInit*>(script->findNodeByType(Node_CommInit)[0]);
    JZNodeModelInit *model_init = dynamic_cast<JZNodeModelInit*>(script->findNodeByType(Node_ModelInit)[0]);

    JZCameraManagerConfig cam_config;
    JZCameraConfig *cfg = nullptr;
    if (m_name == "VisionDemoHik")
    {
        JZCameraHikConfig *cfg_hik = new JZCameraHikConfig();
        cfg_hik->name = "camera";
        cfg_hik->type = Camera_Hik;
        cfg_hik->path = "192.168.0.150";
        cfg_hik->exposureTime = 4000;
        cfg_hik->gain = 0;
        cfg = cfg_hik;
    }
    else if(m_name == "VisionWindowFile")
    {
        JZCameraFileConfig *cfg_file = new JZCameraFileConfig();
        cfg_file->name = "camera";
        cfg_file->path = "C:/Users/xiong/Desktop/JZNodeEditorTest/data";
        cfg = cfg_file;
    }
    else if (m_name == "VisionWindowRtsp")
    {
        JZCameraRtspConfig *cfg_file = new JZCameraRtspConfig();
        cfg_file->name = "camera";
        cfg_file->path = "rtsp://admin:123456HK@192.168.0.64:554/Streaming/Channels/101";
        cfg = cfg_file;
    }
    cam_config.cameraList << JZCameraConfigPtr(cfg);
    cam_init->setConfig(cam_config);

    JZModbusConnetInfo conn;
    conn.modbusType = Modbus_rtuClient;

    JZCommManagerConfig comm_mangare_config;

    JZCommConfig comm_cfg;
    JZCommModbusClientConfig *modbus = new JZCommModbusClientConfig();
    modbus->conn = conn;
    modbus->name = "modbus";

    comm_mangare_config.commList << JZCommConfigPtr(modbus);
    comm_init->setConfig(comm_mangare_config);

    JZModelManagerConfig model_config;
    JZModelYoloConfig *model = new JZModelYoloConfig();
    model->modelPath = "C:/Users/xiong/Desktop/JZNodeEditorTest/data/yolov8n.onnx";
    model->idPath = "C:/Users/xiong/Desktop/JZNodeEditorTest/data/yolov8n.json";

    model_config.modelList << JZModelConfigPtr(model);
    model_init->setConfig(model_config);
}

void SampleVisionWindow::addOnFrameReady()
{
    auto class_item = m_project.getClass("MainWindow");
    auto flow_script = class_item->flow("flow");

    JZNodeCameraReadyEvent *cam_ready = new JZNodeCameraReadyEvent();
    flow_script->addNode(cam_ready);

    JZNodeModelForward *model_forward = new JZNodeModelForward();
    model_forward->setModel("yolo");
    flow_script->addNode(model_forward);
    flow_script->addConnect(cam_ready->flowOutGemo(), model_forward->flowInGemo());
    flow_script->addConnect(cam_ready->paramOutGemo(0), model_forward->paramInGemo(1));
/*
    //set result
    JZNodeFunction *func_cvt = new JZNodeFunction();
    flow_script->addNode(func_cvt);
    func_cvt->setFunction("mat2Image");

    flow_script->addConnect(cam_ready->paramOutGemo(0), func_cvt->paramInGemo(0));

    JZNodeFunction *func_set = new JZNodeFunction();
    flow_script->addNode(func_set);
    func_set->setFunction("JZYoloView::setYoloResult");
    func_set->setVariable("this.yoloView");

    flow_script->addConnect(func_cvt->paramOutGemo(0), func_set->paramInGemo(1));
    flow_script->addConnect(model_forward->paramOutGemo(0), func_set->paramInGemo(2));
    flow_script->addConnect(model_forward->flowOutGemo(), func_set->flowInGemo());

    JZNodeDisplay *display = new JZNodeDisplay();
    display->addInput();
    flow_script->addNode(display);    

    flow_script->addConnect(cam_ready->paramOutGemo(0), display->paramInGemo(0));
    flow_script->addConnect(model_forward->paramOutGemo(0), display->paramInGemo(1));

    //if result > 0
    JZNodeIf *node_if = new JZNodeIf();
    node_if->addElsePin();
    flow_script->addNode(node_if);

    JZNodeFunction *function = new JZNodeFunction();
    flow_script->addNode(function);    
    function->setFunction("QList<JZYoloResult>::size");    
    flow_script->addConnect(model_forward->paramOutGemo(0), function->paramInGemo(0));

    JZNodeGT *node_gt = new JZNodeGT();
    flow_script->addNode(node_gt);

    flow_script->addConnect(function->paramOutGemo(0), node_gt->paramInGemo(0));
    node_gt->setParamInValue(1, "0");

    flow_script->addConnect(node_gt->paramOutGemo(0), node_if->paramInGemo(0));
    flow_script->addConnect(func_set->flowOutGemo(), node_if->flowInGemo());

    JZNodeModbusWrite *write_true = new JZNodeModbusWrite();
    JZNodeModbusWrite *write_false = new JZNodeModbusWrite();
    flow_script->addNode(write_true);
    flow_script->addNode(write_false);
    write_true->setName("modbus");
    write_false->setName("modbus");
    write_true->setValue("1");
    write_false->setValue("0");

    flow_script->addConnect(node_if->subFlowOutGemo(0), write_true->flowInGemo());
    flow_script->addConnect(node_if->subFlowOutGemo(1), write_false->flowInGemo());
*/
}