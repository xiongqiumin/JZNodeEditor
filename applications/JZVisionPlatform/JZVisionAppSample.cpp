#include <QApplication>
#include "JZVisionAppSample.h"
#include "modules/camera/JZCameraNode.h"
#include "modules/communication/JZCommNode.h"
#include "modules/model/JZModelNode.h"
#include "modules/JZModuleCompiler.h"
#include "JZEditorUtils.h"
#include "JZNodeFlow.h"
#include "JZNodeFunction.h"
#include "JZNodeOperator.h"
#include "JZVisionAppLayout.h"

JZVisionAppSample::JZVisionAppSample()
{
    m_project = nullptr;
}


void JZVisionAppSample::updateInit()
{
    auto class_item = m_project->getClass("MyVisionApp");
    auto script = class_item->memberFunction("init");

    auto* camera_init = dynamic_cast<JZNodeCameraInit*>(getInitNode(script,Node_CameraInit));
    auto* model_init = dynamic_cast<JZNodeModelInit*>(getInitNode(script, Node_ModelInit));
    auto* comm_init = dynamic_cast<JZNodeCommInit*>(getInitNode(script, Node_CommInit));

    //camera_init
    JZCameraManagerConfig cam_config;
    JZCameraConfig* cfg = nullptr;
    
    JZCameraFileConfig* cfg_file = new JZCameraFileConfig();
    cfg_file->name = "camera";
    cfg_file->path = qApp->applicationDirPath() + "/data";
    cfg = cfg_file;
    cam_config.cameraList << JZCameraConfigEnum(cfg);
    camera_init->setConfig(cam_config);

    //comm_init
    JZModbusConnetInfo conn;
    conn.modbusType = Modbus_tcpClient;

    JZCommManagerConfig comm_mangare_config;

    JZCommConfig comm_cfg;
    JZCommModbusTcpClientConfig* modbus = new JZCommModbusTcpClientConfig();
    modbus->conn = conn;
    modbus->name = "modbus";

    comm_mangare_config.commList << JZCommConfigEnum(modbus);
    comm_init->setConfig(comm_mangare_config);

    //model_init
    JZModelManagerConfig model_config;
    JZModelYoloConfig* model = new JZModelYoloConfig();
    model->modelPath = qApp->applicationDirPath() + "/model/yolo/yolov8n.onnx";
    model->idPath = qApp->applicationDirPath() + "/model/yolo/yolov8n.json";

    model_config.modelList << JZModelConfigEnum(model);
    model_init->setConfig(model_config);

}

void JZVisionAppSample::create(JZProject *project)
{
    m_project = project;
    updateInit();    

    auto class_item = m_project->getClass("MyVisionApp");
    auto flow_script = class_item->addFlow("flow");

    JZNodeCameraReadyEvent* cam_ready = new JZNodeCameraReadyEvent();
    flow_script->addNode(cam_ready);

    JZNodeModelForward* model_forward = new JZNodeModelForward();
    model_forward->setModel("yolo");
    flow_script->addNode(model_forward);
    flow_script->addConnect(cam_ready->flowOutGemo(), model_forward->flowInGemo());
    flow_script->addConnect(cam_ready->paramOutGemo(0), model_forward->paramInGemo(1));
   
    //if result > 0
    JZNodeIf *node_if = new JZNodeIf();
    node_if->addElsePin();
    flow_script->addNode(node_if);

    JZNodeContainerFunction *func_size = new JZNodeContainerFunction();
    func_size->setForceFlow(true);
    flow_script->addNode(func_size);
    func_size->setFunction("QList<T>::size");
    flow_script->addConnect(model_forward->paramOutGemo(0), func_size->paramInGemo(0));
    flow_script->addConnect(model_forward->flowOutGemo(0), func_size->flowInGemo());

    JZNodeGT *node_gt = new JZNodeGT();
    node_gt->setFlow(true);
    flow_script->addNode(node_gt);

    flow_script->addConnect(func_size->paramOutGemo(0), node_gt->paramInGemo(0));
    flow_script->addConnect(func_size->flowOutGemo(0), node_gt->flowInGemo());
    node_gt->setParamInValue(1, "0");

    flow_script->addConnect(node_gt->paramOutGemo(0), node_if->paramInGemo(0));
    flow_script->addConnect(node_gt->flowOutGemo(), node_if->flowInGemo());

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
        
}

void JZVisionAppSample::save(QString path)
{
    JZVisionAppLayout layout;
    layout.layout(m_project);

    m_project->saveAs(path);
    m_project->saveAllItem();
    m_project->save();
}