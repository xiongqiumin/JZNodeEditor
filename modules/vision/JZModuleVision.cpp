#include <opencv2/opencv.hpp>
#include <QBuffer>
#include "JZModuleVision.h"
#include "JZScriptEnvironment.h"
#include "JZNodeBind.h"
#include "JZVisionWidget.h"
#include "JZVisionWindow.h"
#include "JZVisionUiItem.h"

using namespace cv;

//JZModuleVision
JZModuleVision::JZModuleVision()
{        
    m_name = "vision";

    m_visionWindowFactory.creator = [](JZNodeObject *obj)
    {
        JZVisionWindowConfig config;

        QByteArray buffer = obj->meta()->widgetDefine.buffer;
        QDataStream s(buffer);
        s >> config;

        JZVisionWindow *w = JZObjectCast<JZVisionWindow>(obj);
        w->init(config);
    };

    JZProjectItemManager::instance()->registItem(ProjectItem_visionUi, createJZProjectItem<JZVisionUiItem>);
}

JZModuleVision::~JZModuleVision()
{
}

void JZModuleVision::regist(JZScriptEnvironment *env)
{    
    int cls_id = Module_VisionType;

    jzbind::ClassBind<JZCameraListWidget> cls_camera_list(cls_id++, "JZCameraListWidget", "QWidget");
    cls_camera_list.regist();

    jzbind::ClassBind<JZCameraViewWidget> cls_camera_view(cls_id++, "JZCameraViewWidget", "QWidget");
    cls_camera_view.def("label", false, &JZCameraViewWidget::label, CFunction::Reference);
    cls_camera_view.regist();

    jzbind::ClassBind<JZVisionWindow> cls_vision_window(cls_id++, "JZVisionWindow", "QMainWindow");
    cls_vision_window.defPropertyFunc("cameraManager", &JZVisionWindow::cameraManager);
    cls_vision_window.defPropertyFunc("commManager", &JZVisionWindow::commManager);
    cls_vision_window.defPropertyFunc("modelManager", &JZVisionWindow::modelManager);
    cls_vision_window.defPropertyFunc("cameraView", &JZVisionWindow::cameraView);
    cls_vision_window.def("initView", true, &JZVisionWindow::initView);
    cls_vision_window.regist();

    jzbind::ClassBind<JZTemplateMatch> cls_template_match(cls_id++, "JZTemplateMatch", "QObject");
    cls_template_match.def("match", true, &JZTemplateMatch::match);
    cls_template_match.regist();

    jzbind::ClassBind<BrightnessDetectorResult> cls_birghtness_ret(cls_id++, "BrightnessDetectorResult");
    cls_birghtness_ret.setValueType(true);
    cls_birghtness_ret.defProperty("cast", &BrightnessDetectorResult::cast);
    cls_birghtness_ret.defProperty("da", &BrightnessDetectorResult::da);
    cls_birghtness_ret.regist();

    auto func_inst = env->functionManager();
    func_inst->registCFunction("JZTemplateMatchInit", true, jzbind::createFuncion(JZTemplateMatchInit));
   
    func_inst->registCFunction("JZVisionCropImage", true, jzbind::createFuncion(JZVisionCropImage));
    func_inst->registCFunction("JZVisionImageFlip", true, jzbind::createFuncion(JZVisionImageFlip));
    func_inst->registCFunction("JZVisionImageMorphology", true, jzbind::createFuncion(JZVisionImageMorphology));
    func_inst->registCFunction("JZVisionPerspectiveTransform", true, jzbind::createFuncion(JZVisionPerspectiveTransform));
    func_inst->registCFunction("JZVisionSkeleton", true, jzbind::createFuncion(JZVisionSkeleton));

    //func_inst->registCFunction("JZVisionBlobDetector", true, jzbind::createFuncion(JZVisionBlobDetector));
    func_inst->registCFunction("JZVisionBrightnessDetector", true, jzbind::createFuncion(JZVisionBrightnessDetector));
    func_inst->registCFunction("JZVisionColorIdentify", true, jzbind::createFuncion(JZVisionColorIdentify));

    func_inst->registCFunction("JZVisionFindCircle", true, jzbind::createFuncion(JZVisionFindCircle));
    func_inst->registCFunction("JZVisionFindLine", true, jzbind::createFuncion(JZVisionFindLine));

    auto obj_inst = env->objectManager();
    obj_inst->registWidgetFactory(Widget_Vision, m_visionWindowFactory);

    //node
    auto node_inst = env->nodeFactory();

    node_inst->registNode(Node_VisionCropImage, createJZNode<JZNodeVisionCropImage>);

    node_inst->registNode(Node_VisionImageFlip, createJZNode<JZNodeVisionImageFlip>);
    node_inst->registNode(Node_VisionImageMorphology, createJZNode<JZNodeVisionImageMorphology>);
    node_inst->registNode(Node_VisionImageRotate, createJZNode<JZNodeVisionImageRotate>);
    node_inst->registNode(Node_VisionImageSplice, createJZNode<JZNodeVisionImageSplice>);
    node_inst->registNode(Node_VisionPerspectiveTransform, createJZNode<JZNodeVisionPerspectiveTransform>);
    node_inst->registNode(Node_VisionSkeleton, createJZNode<JZNodeVisionSkeleton>);

    node_inst->registNode(Node_VisionBlobDetector, createJZNode<JZNodeVisionBlobDetector>);
    node_inst->registNode(Node_VisionBrightnessDetector, createJZNode<JZNodeVisionBrightnessDetector>);
    node_inst->registNode(Node_VisionColorIdentify, createJZNode<JZNodeVisionColorIdentify>);

    node_inst->registNode(Node_VisionShapeMatch, createJZNode<JZNodeVisionShapeMatch>);
    node_inst->registNode(Node_VisionTemplateMatch, createJZNode<JZNodeVisionTemplateMatch>);

    node_inst->registNode(Node_VisionFindCircle, createJZNode<JZNodeVisionFindCircle>);
    node_inst->registNode(Node_VisionFindLine, createJZNode<JZNodeVisionFindLine>);
}

void JZModuleVision::unregist(JZScriptEnvironment *env)
{
}