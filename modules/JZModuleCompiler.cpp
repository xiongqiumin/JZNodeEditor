#include "JZModuleCompiler.h"

JZNode *getInitNode(JZScriptItem *item,int type)
{
    auto cls_item = item->getClassItem();
    if (!cls_item)
        return nullptr;
    
    auto init_script = cls_item->memberFunction("init");
    auto node_list = init_script->findNodeByType(type);
    Q_ASSERT(node_list.size() == 1);

    return node_list[0];
}