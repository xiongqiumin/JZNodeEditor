#include "JZModuleCommEditor.h"
#include "JZModuleComm.h"
#include "JZEditorGlobal.h"

void JZModuleCommEditorInit()
{
    auto inst = editorManager()->instance();

    JZLogicNode logic;
    logic.nodeType = Node_CommInit;
    logic.path = "通信";
    inst->registLogicNode(logic);
}