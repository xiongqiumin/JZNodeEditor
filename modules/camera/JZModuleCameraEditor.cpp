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
}