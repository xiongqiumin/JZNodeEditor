#include "JZModuleVisionApp.h"
#include "JZScriptEnvironment.h"
#include "JZNodeBind.h"
#include "modules/JZModuleDefine.h"
#include "mainwindow.h"

//JZModuleVisionApp
JZModuleVisionApp::JZModuleVisionApp()
{
    m_name = "visionApp";
}

JZModuleVisionApp::~JZModuleVisionApp()
{
}

void JZModuleVisionApp::regist(JZScriptEnvironment* env)
{
    int cls_id = Module_VisionAppType;

    jzbind::ClassBind<MainWindow> cls_mainwindow(cls_id++, "JZVisionPlatform");
    cls_mainwindow.defPropertyFunc("cameraManager", &MainWindow::cameraManager);
    cls_mainwindow.defPropertyFunc("modelManager", &MainWindow::modelManager);
    cls_mainwindow.defPropertyFunc("commManager", &MainWindow::commManager);
    cls_mainwindow.regist();

    
}

void JZModuleVisionApp::unregist(JZScriptEnvironment* env)
{
}