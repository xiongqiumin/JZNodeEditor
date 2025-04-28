#include "JZNodeFactory.h"
#include "JZModuleCameraEditor.h"
#include "JZModuleCamera.h"
#include "JZEditorGlobal.h"
#include "JZCameraNode.h"

void JZCameraEditorInit()
{
    auto inst = editorManager()->instance();

    inst->registLogicNode(Node_CameraInit,"相机");
    inst->registLogicNode(Node_CameraStart,"相机");
    inst->registLogicNode(Node_CameraStartOnce,"相机");
    inst->registLogicNode(Node_CameraStop,"相机");
    inst->registLogicNode(Node_CameraSetting,"相机");
    inst->registLogicNode(Node_CameraFrameReady,"相机");
}