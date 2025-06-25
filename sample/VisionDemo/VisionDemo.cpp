#include <QApplication>
#include <QDir>
#include "VisionDemo.h"
#include "JZNodeBuilder.h"
#include "JZNodeVM.h"
#include "UiCommon.h"
#include "JZNodeFunction.h"
#include "JZNodeValue.h"
#include "JZUiItem.h"
#include "JZNodeView.h"
#include "JZNodeUtils.h"
#include "JZContainer.h"
#include "modules/camera/JZCameraNode.h"
#include "modules/communication/JZCommNode.h"
#include "modules/model/JZModelNode.h"

SampleVisionDemo::SampleVisionDemo()
{        
    QFileInfo info(__FILE__);
    m_root = info.path();
}

SampleVisionDemo::~SampleVisionDemo()
{

}

void SampleVisionDemo::initProject(QString name)
{
    newProject(name,"ui");

    auto class_item = m_project.getClass("MainWindow");
    
    JZUiItem *ui_file = dynamic_cast<JZUiItem*>(class_item->ui());
    QString xml = loadUi("VisionDemo.ui");
    ui_file->setXml(xml);
    m_project.saveItem(ui_file);

    addInit();
    addOnFrameReady();
    addBtnClicked();

    saveProject();
}

void SampleVisionDemo::initCameraHik()
{
    initProject("VisionDemoHik");
}

void SampleVisionDemo::initCameraFile()
{
    initProject("VisionDemo");
}

void SampleVisionDemo::addInit()
{
    auto class_item = m_project.getClass("MainWindow");
    auto script = class_item->memberFunction("init");
    class_item->addMemberVariable(JZParamDefine("cameraManager", "JZCameraManager"));
    class_item->addMemberVariable(JZParamDefine("commManager", "JZCommManager"));
    class_item->addMemberVariable(JZParamDefine("modelManager","JZModelManager"));

    auto start_node = script->startNode();
    JZNodeCameraInit *cam_init = new JZNodeCameraInit();
    JZNodeCommInit *comm_init = new JZNodeCommInit();
    JZNodeModelInit *model_init = new JZNodeModelInit();
    script->addNode(cam_init);
    script->addNode(comm_init);
    script->addNode(model_init);
    script->addConnect(start_node->flowOutGemo(), cam_init->flowInGemo());
    script->addConnect(cam_init->flowOutGemo(), comm_init->flowInGemo());
    script->addConnect(comm_init->flowOutGemo(), model_init->flowInGemo());

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
    else
    {
        JZCameraFileConfig *cfg_file = new JZCameraFileConfig();
        cfg_file->name = "camera";
        cfg_file->path = "C:/Users/xiong/Desktop/JZNodeEditorTest/data";
        cfg = cfg_file;
    }
    cam_config.cameraList << JZCameraConfigEnum(cfg);
    cam_init->setConfig(cam_config);

    JZModbusConnetInfo conn;
    conn.modbusType = Modbus_rtuClient;

    JZCommManagerConfig comm_mangare_config;

    JZCommConfig comm_cfg;
    JZCommModbusTcpClientConfig*modbus = new JZCommModbusTcpClientConfig();
    modbus->conn = conn;
    modbus->name = "modbus";

    comm_mangare_config.commList << JZCommConfigEnum(modbus);
    comm_init->setConfig(comm_mangare_config);

    JZModelManagerConfig model_config;
    JZModelYoloConfig *model = new JZModelYoloConfig();    
    model->modelPath = "C:/Users/xiong/Desktop/JZNodeEditorTest/data/yolov8n.onnx";
    model->idPath = "C:/Users/xiong/Desktop/JZNodeEditorTest/data/yolov8n.json";

    model_config.modelList << JZModelConfigEnum(model);
    model_init->setConfig(model_config);
}

void SampleVisionDemo::addOnFrameReady()
{
    auto class_item = m_project.getClass("MainWindow");
    auto flow_script = class_item->addFlow("flow");

    JZNodeCameraReadyEvent *cam_ready = new JZNodeCameraReadyEvent();
    flow_script->addNode(cam_ready);

    JZNodeModelForward *model_forward = new JZNodeModelForward();
    model_forward->setModel("yolo");
    flow_script->addNode(model_forward);
    flow_script->addConnect(cam_ready->flowOutGemo(), model_forward->flowInGemo());
    flow_script->addConnect(cam_ready->paramOutGemo(0), model_forward->paramInGemo(1));

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
/*
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

void SampleVisionDemo::addBtnClicked()
{
    auto addBtnFunc = [this](QString btn,JZNode* node) {
        auto class_item = m_project.getClass("MainWindow");
        auto def = class_item->objectDefine();

        auto start_def = def.initSlotFunction(btn, "clicked");
        auto script = class_item->addMemberFunction(start_def);
        auto start_node = script->startNode();
        script->addNode(node);

        script->addConnect(start_node->flowOutGemo(), node->flowInGemo());
    };
    
    addBtnFunc("btnStartOnce",new JZNodeCameraStartOnce());
    addBtnFunc("btnStart", new JZNodeCameraStart());
    addBtnFunc("btnStop", new JZNodeCameraStop());
}