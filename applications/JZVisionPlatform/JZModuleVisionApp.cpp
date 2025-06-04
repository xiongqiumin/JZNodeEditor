#include "JZModuleVisionApp.h"
#include "JZScriptEnvironment.h"
#include "JZNodeBind.h"
#include "modules/JZModuleDefine.h"
#include "modules/JZModuleDebug.h"
#include "mainwindow.h"

class JZVisionImageDebug : public BuiltInFunction
{
public:
    virtual void call(JZNodeEngine *engine) override
    {
        g_visionWindow->imageDebug();
    }
};

//JZModuleVisionApp
JZModuleVisionApp::JZModuleVisionApp()
{
    m_name = "visionApp";

    JZModuleDebugManager::instance()->setNodeDebug("JZVisionImageDebug");
}

JZModuleVisionApp::~JZModuleVisionApp()
{
}

void JZModuleVisionApp::regist(JZScriptEnvironment* env)
{
    auto func_inst = env->functionManager();

    JZFunctionDefine image_debug;
    image_debug.name = "JZVisionImageDebug";
    image_debug.isCFunction = true;
    image_debug.paramIn.push_back(env->paramDefine("args", Type_args));
    auto format_func = BuiltInFunctionPtr(new JZVisionImageDebug());
    func_inst->registBuiltInFunction(image_debug, format_func);

    int cls_id = Module_VisionAppType;
    jzbind::ClassBind<MainWindow> cls_mainwindow(cls_id++, "JZVisionPlatform", "QMainWindow");
    cls_mainwindow.defPropertyFunc("cameraManager", &MainWindow::cameraManager);
    cls_mainwindow.defPropertyFunc("modelManager", &MainWindow::modelManager);
    cls_mainwindow.defPropertyFunc("commManager", &MainWindow::commManager);
    cls_mainwindow.regist();
}

void JZModuleVisionApp::unregist(JZScriptEnvironment* env)
{
}