#include "JZNodeFactory.h"
#include "JZModuleCameraEditor.h"
#include "JZModuleCamera.h"
#include "JZEditorGlobal.h"

void JZCameraEditorInit()
{
    auto inst = editorManager()->instance();

    JZLogicNode logic;
    logic.nodeType = Node_CameraInit;
    logic.path = "相机";
    inst->registLogicNode(logic);

    JZLogicNode logic_frameReady;
    logic_frameReady.nodeType = Node_CameraFrameReady;
    logic_frameReady.path = "相机";
    inst->registLogicNode(logic_frameReady);
}