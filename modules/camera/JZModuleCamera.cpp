#include <QSerialPort>
#include <QPushButton>
#include "JZModuleCamera.h"
#include "JZCamera.h"
#include "JZCameraFile.h"
#include "JZScriptEnvironment.h"
#include "JZNodeBind.h"

enum 
{
    CameraModule_id = 16000,
};

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
    int cls_id = CameraModule_id;

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
}

void JZModuleCamera::unregist(JZScriptEnvironment *env)
{

}