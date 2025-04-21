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
    newProject(name);

    auto class_item = m_project.getClass("MainWindow");

    JZUiItem *ui_file = class_item->ui();
    QString xml = loadUi("VisionDemo.ui");
    ui_file->setXml(xml);
    m_project.saveItem(ui_file);

    if(name == "VisionDemoHik")
        class_item->addMemberVariable("camera", "JZCameraHik");
    else
        class_item->addMemberVariable("camera", "JZCameraFile");
    class_item->addMemberVariable("yolo", "JZYolo");

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
    auto start_node = script->startNode();

    JZNodeFunction *func_load = new JZNodeFunction();
    JZNodeFunction *func_open = new JZNodeFunction();
    script->addNode(func_load);
    script->addNode(func_open);

    func_load->setFunction("JZYolo::loadNet");
    func_load->setVariable("this.yolo");
    func_open->setFunction("JZCamera::open");
    func_open->setVariable("this.camera");
    
    func_load->setParamInValue(1, "C:/Users/xiong/Desktop/JZNodeEditorTest/data/yolov8n.onnx");
    
    if (class_item->memberVariable("camera", false)->type == "JZCameraFile")
        func_open->setParamInValue(1, "C:/Users/xiong/Desktop/JZNodeEditorTest/data");
    else
        func_open->setParamInValue(1, "192.168.0.150");

    script->addConnect(start_node->flowOutGemo(),func_load->flowInGemo());
    script->addConnect(func_load->flowOutGemo(), func_open->flowInGemo());
}

void SampleVisionDemo::addOnFrameReady()
{
    auto class_item = m_project.getClass("MainWindow");
    auto def = class_item->objectDefine();

    auto start_def = def.initSlotFunction("camera", "sigFrameReady");
    auto script = class_item->addMemberFunction(start_def);
    auto start_node = script->startNode();

    JZNodeParam *param = new JZNodeParam();
    script->addNode(param);
    param->setVariable(script->function().paramIn[1].name);

    JZNodeFunction *func_load = new JZNodeFunction();
    script->addNode(func_load);
    func_load->setFunction("JZYolo::forward");
    func_load->setVariable("this.yolo");

    script->addConnect(start_node->flowOutGemo(), func_load->flowInGemo());
    script->addConnect(param->paramOutGemo(0), func_load->paramInGemo(1));

    JZNodeFunction *func_cvt = new JZNodeFunction();
    script->addNode(func_cvt);
    func_cvt->setFunction("mat2Image");    

    script->addConnect(param->paramOutGemo(0), func_cvt->paramInGemo(0));

    JZNodeFunction *func_set = new JZNodeFunction();
    script->addNode(func_set);
    func_set->setFunction("JZYoloView::setYoloResult");
    func_set->setVariable("this.yoloView");

    script->addConnect(func_cvt->paramOutGemo(0), func_set->paramInGemo(1));
    script->addConnect(func_load->paramOutGemo(0), func_set->paramInGemo(2));
    script->addConnect(func_load->flowOutGemo(), func_set->flowInGemo());
}

void SampleVisionDemo::addBtnClicked()
{
    auto addBtnFunc = [this](QString btn,QString func) {
        auto class_item = m_project.getClass("MainWindow");
        auto def = class_item->objectDefine();

        auto start_def = def.initSlotFunction(btn, "clicked");
        auto script = class_item->addMemberFunction(start_def);
        auto start_node = script->startNode();

        JZNodeFunction *camera_func = new JZNodeFunction();
        script->addNode(camera_func);

        camera_func->setFunction("JZCamera::" + func);
        camera_func->setVariable("this.camera");

        script->addConnect(start_node->flowOutGemo(), camera_func->flowInGemo());
    };
    
    addBtnFunc("btnStartOnce","startOnce");
    addBtnFunc("btnStart", "start");
    addBtnFunc("btnStop", "stop");
}