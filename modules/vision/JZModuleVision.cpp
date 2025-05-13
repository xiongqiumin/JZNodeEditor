#include <opencv2/opencv.hpp>
#include <QBuffer>
#include "JZModuleVision.h"
#include "JZScriptEnvironment.h"
#include "JZNodeBind.h"
#include "JZVisionWidget.h"

using namespace cv;

//JZModuleVision
JZModuleVision::JZModuleVision()
{        
    m_name = "vision";
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
    cls_camera_view.regist();

    auto func_inst = env->functionManager();
   
    func_inst->registCFunction("JZVisionCropImage", true, jzbind::createFuncion(JZVisionCropImage));
    func_inst->registCFunction("JZVisionImageFlip", true, jzbind::createFuncion(JZVisionImageFlip));
    func_inst->registCFunction("JZVisionImageMorphology", true, jzbind::createFuncion(JZVisionImageMorphology));
    func_inst->registCFunction("JZVisionImageRotate", true, jzbind::createFuncion(JZVisionImageRotate));
    func_inst->registCFunction("JZVisionImageSplice", true, jzbind::createFuncion(JZVisionImageSplice));
    func_inst->registCFunction("JZVisionPerspectiveTransform", true, jzbind::createFuncion(JZVisionPerspectiveTransform));
    func_inst->registCFunction("JZVisionSkeleton", true, jzbind::createFuncion(JZVisionSkeleton));

    func_inst->registCFunction("JZVisionBlobDetector", true, jzbind::createFuncion(JZVisionBlobDetector));
    func_inst->registCFunction("JZVisionBrightnessDetector", true, jzbind::createFuncion(JZVisionBrightnessDetector));
    func_inst->registCFunction("JZVisionColorIdentify", true, jzbind::createFuncion(JZVisionColorIdentify));

    func_inst->registCFunction("JZVisionShapeMatch", true, jzbind::createFuncion(JZVisionShapeMatch));
    func_inst->registCFunction("JZVisionTemplateMatch", true, jzbind::createFuncion(JZVisionTemplateMatch));

    func_inst->registCFunction("JZVisionFindCircle", true, jzbind::createFuncion(JZVisionFindCircle));
    func_inst->registCFunction("JZVisionFindLine", true, jzbind::createFuncion(JZVisionFindLine));

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