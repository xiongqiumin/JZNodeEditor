#include "JZModuleVisionApp.h"
#include "JZScriptEnvironment.h"
#include "JZNodeBind.h"
#include "modules/JZModuleDefine.h"
#include "modules/JZModuleDebug.h"
#include "mainwindow.h"
#include "JZVisionAppNode.h"

void JZVisionAppCameraInit(JZCameraManager* inst, const QByteArray& buffer)
{
}

void JZVisionAppCommInit(JZCommManager* inst, const QByteArray& buffer)
{
}

void JZVisionAppModelInit(JZModelManager* inst, const QByteArray& buffer)
{
}

//JZVisionApplication
JZVisionApplication::JZVisionApplication()
{
    m_window = nullptr;
}

JZVisionApplication::~JZVisionApplication()
{
}

void JZVisionApplication::setMainWindow(MainWindow* window)
{
    m_window = window;
}

JZModelManager* JZVisionApplication::modelManager()
{
    return m_window->modelManager();
}

JZCameraManager* JZVisionApplication::cameraManager()
{
    return m_window->cameraManager();
}

JZCommManager* JZVisionApplication::commManager()
{
    return m_window->commManager();
}

JZPaddleOCR* JZVisionApplication::getOCR()
{
    return m_window->getOCR();
}

JZBarCode* JZVisionApplication::getBarCode()
{
    return m_window->getBarCode();
}

JZQRCode* JZVisionApplication::getQrCode()
{
    return m_window->getQrCode();
}

//JZVisionImageDebug
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
    
    func_inst->replaceCFunction("JZCameraInit", true, jzbind::createFuncion(JZVisionAppCameraInit));
    func_inst->replaceCFunction("JZCommInit",true, jzbind::createFuncion(JZVisionAppCommInit));
    func_inst->replaceCFunction("JZModelInit",true, jzbind::createFuncion(JZVisionAppModelInit));

    int cls_id = Module_VisionAppType;
    jzbind::ClassBind<JZVisionApplication> cls_app(cls_id++, "JZVisionApplication", "QObject");
    cls_app.defPropertyFunc("cameraManager", &JZVisionApplication::cameraManager);
    cls_app.defPropertyFunc("modelManager", &JZVisionApplication::modelManager);
    cls_app.defPropertyFunc("commManager", &JZVisionApplication::commManager);
    cls_app.def("getBarCode", false, &JZVisionApplication::getBarCode, CFunction::Reference);
    cls_app.def("getQrCode", false, &JZVisionApplication::getQrCode, CFunction::Reference);

    cls_app.def("getOCR", false, &JZVisionApplication::getOCR, CFunction::Reference);
    cls_app.regist();

    auto node_factory = env->nodeFactory();
    node_factory->registNode(Node_visionAppParam, createJZNode<JZNodeVisionParam>);
    node_factory->registNode(Node_visionAppBarCode, createJZNode<JZNodeVisionAppBarCode>);
    node_factory->registNode(Node_visionAppQrCode, createJZNode<JZNodeVisionAppQrCode>);
    node_factory->registNode(Node_visionAppOCR, createJZNode<JZNodeVisionAppOCR>);
}

void JZModuleVisionApp::unregist(JZScriptEnvironment* env)
{
}