#include <QSerialPort>
#include <QPushButton>
#include "JZModuleCamera.h"
#include "JZCamera.h"
#include "JZCameraFile.h"
#include "JZCameraHik.h"
#include "JZScriptEnvironment.h"
#include "JZNodeBind.h"
#include "JZNodeFactory.h"
#include "JZNodeCompiler.h"
#include "JZCameraNode.h"
#include "JZCameraManager.h"
#include "JZCameraUnitTest.h"
#include "../JZModuleConfigFactory.h"

//JZModuleCamera
JZModuleCamera::JZModuleCamera()
{    
    m_name = "camera";

    JZScriptUnitTestManager::instance()->regist(new JZNodeCameraVistor());

    auto config_inst = JZModuleConfigFactory<JZCameraConfig>::instance();
    config_inst->regist(Camera_File, JZModuleConfigCreator<JZCameraFileConfig>);
    config_inst->regist(Camera_Hik, JZModuleConfigCreator<JZCameraHikConfig>);
    config_inst->regist(Camera_UVC, JZModuleConfigCreator<JZCameraUvcConfig>);
}

JZModuleCamera::~JZModuleCamera()
{
}

void JZModuleCamera::regist(JZScriptEnvironment *env)
{
    int cls_id = Module_CameraType;

    jzbind::ClassBind<JZCamera> cls_camera(cls_id++, "JZCamera", "QObject");
    cls_camera.def("open", true, &JZCamera::open);
    cls_camera.def("close", true, &JZCamera::close);
    cls_camera.def("start",true,&JZCamera::start);
    cls_camera.def("startOnce", true, &JZCamera::startOnce);
    cls_camera.def("stop", true, &JZCamera::stop);
    cls_camera.defSingle("sigFrameReady", &JZCamera::sigFrameReady);
    cls_camera.regist();

    jzbind::ClassBind<JZCameraFile> cls_camera_file(cls_id++, "JZCameraFile", "JZCamera");
    cls_camera_file.regist();

    jzbind::ClassBind<JZCameraHik> cls_camera_hik(cls_id++, "JZCameraHik", "JZCamera");
    cls_camera_hik.regist();

    jzbind::ClassBind<JZCameraManager> cls_camera_manager(cls_id++, "JZCameraManager", "QObject");
    cls_camera_manager.regist();

    auto func_inst = env->functionManager();
    func_inst->registCFunction("JZCameraInit", true, jzbind::createFuncion(JZCameraInit));
    func_inst->registCFunction("JZCameraConnect", true, jzbind::createFuncion(JZCameraConnect));
    func_inst->registCFunction("JZCameraStart", true, jzbind::createFuncion(JZCameraStart));
    func_inst->registCFunction("JZCameraStartOnce", true, jzbind::createFuncion(JZCameraStartOnce));
    func_inst->registCFunction("JZCameraStop", true, jzbind::createFuncion(JZCameraStop));

    env->nodeFactory()->registNode(Node_CameraInit, createJZNode<JZNodeCameraInit>);
    env->nodeFactory()->registNode(Node_CameraStart, createJZNode<JZNodeCameraStart>);
    env->nodeFactory()->registNode(Node_CameraStartOnce, createJZNode<JZNodeCameraStartOnce>);
    env->nodeFactory()->registNode(Node_CameraStop, createJZNode<JZNodeCameraStop>);
    env->nodeFactory()->registNode(Node_CameraSetting, createJZNode<JZNodeCameraSetting>);
    env->nodeFactory()->registNode(Node_CameraFrameReady, createJZNode<JZNodeCameraReadyEvent>);
}

void JZModuleCamera::unregist(JZScriptEnvironment *env)
{

}
