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

//JZModuleCamera
JZModuleCamera::JZModuleCamera()
{    
    m_name = "modbus";
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
    cls_camera.def("setConfig", true, &JZCamera::setConfig);
    cls_camera.def("config", true, &JZCamera::config);
    cls_camera.defSingle("sigFrameReady", &JZCamera::sigFrameReady);
    cls_camera.regist();

    jzbind::ClassBind<JZCameraFile> cls_camera_file(cls_id++, "JZCameraFile", "JZCamera");
    cls_camera_file.regist();

    jzbind::ClassBind<JZCameraHik> cls_camera_hik(cls_id++, "JZCameraHik", "JZCamera");
    cls_camera_hik.regist();

    auto func = env->functionManager();

    env->nodeFactory()->registNode(Node_CameraInit, createJZNode<JZNodeCameraInit>);
    env->nodeFactory()->registNode(Node_CameraFrameReady, createJZNode<JZNodeCameraReadyEvent>);
}

void JZModuleCamera::unregist(JZScriptEnvironment *env)
{

}
