#include "JZModuleLogEditor.h"
#include "JZEditorGlobal.h"

JZLogRecordItem::JZLogRecordItem(JZNode *node)
    :JZNodeGraphItem(node)
{

}

void JZModuleLogEditorInit()
{    
    auto inst = editorManager()->instance();

    inst->registLogicNode(Node_logEvent, "日志", QString(), CreateJZNodeGraphItem<JZLogRecordItem>);
}