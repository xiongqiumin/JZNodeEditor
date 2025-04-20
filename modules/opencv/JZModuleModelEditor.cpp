#include "JZNodeFactory.h"
#include "JZModuleModelEditor.h"
#include "JZModelNode.h"
#include "JZEditorGlobal.h"

void JZModuleModelEditorInit()
{
    auto inst = editorManager()->instance();

    JZLogicNode logic;
    logic.nodeType = Node_ModelInit;
    logic.path = "模型";
    inst->registLogicNode(logic);


    JZLogicNode logic_set;
    logic_set.nodeType = Node_ModelSetting;
    logic_set.path = "模型";
    inst->registLogicNode(logic_set);
}