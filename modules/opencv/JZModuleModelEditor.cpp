#include "JZNodeFactory.h"
#include "JZModuleModelEditor.h"
#include "JZModelNode.h"
#include "JZEditorGlobal.h"

void JZModuleModelEditorInit()
{
    auto inst = editorManager()->instance();

    JZLogicNode logic;
    logic.nodeType = Node_ModelForward;
    logic.path = "模型/推理";
    inst->registLogicNode(logic);
}