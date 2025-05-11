#include "JZModuleVisionEditor.h"
#include "JZVisionNode.h"
#include "JZEditorGlobal.h"
#include "JZNodeEditorManager.h"

//JZModuleVisionEditorInit
void JZModuleVisionEditorInit()
{
    auto inst = editorManager()->instance();

    auto icon = [](QString name)->QString {
        return ":/Modules/Vision/res/" + name;
    };
    inst->registLogicNode(Node_VisionCropImage, "Vision/图像处理", icon("crop.png"));
    inst->registLogicNode(Node_VisionImageFlip, "Vision/图像处理", icon("flip.png"));
    inst->registLogicNode(Node_VisionImageMorphology, "Vision/图像处理", icon("morphology.png"));
    inst->registLogicNode(Node_VisionImageRotate, "Vision/图像处理", icon("rotate_x.png"));
    inst->registLogicNode(Node_VisionImageSplice, "Vision/图像处理", icon("image_splice.png"));
    inst->registLogicNode(Node_VisionPerspectiveTransform, "Vision/图像处理", icon("perspective.png"));
    inst->registLogicNode(Node_VisionSkeleton, "Vision/图像处理", icon("skeleton.png"));

    inst->registLogicNode(Node_VisionBlobDetector, "Vision/检测识别", icon("blob.png"));
    inst->registLogicNode(Node_VisionBrightnessDetector, "Vision/检测识别", icon("brightness.png"));
    inst->registLogicNode(Node_VisionColorIdentify, "Vision/检测识别", icon("color_r.png"));

    inst->registLogicNode(Node_VisionShapeMatch, "Vision/对位工具", icon("shape_match.png"));
    inst->registLogicNode(Node_VisionTemplateMatch, "Vision/对位工具", icon("match.png"));

    inst->registLogicNode(Node_VisionFindCircle, "Vision/几何工具", icon("find_circle.png"));
    inst->registLogicNode(Node_VisionFindLine, "Vision/几何工具", icon("find_line.png"));
}