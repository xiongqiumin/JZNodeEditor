#include <opencv2/opencv.hpp>
#include <QBuffer>
#include "JZModuleVision.h"
#include "JZScriptEnvironment.h"

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