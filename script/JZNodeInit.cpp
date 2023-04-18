#include "JZNodeInit.h"
#include "JZNodeFactory.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeDebugPacket.h"

void JZNodeInit()
{
    JZNodeFactory::instance()->init();
    JZNodeFunctionManager::instance()->init();   
    JZNetPackManager::instance()->init();
    JZNetPackManager::instance()->registPack(NetPack_debugPacket,createNetPackFunc<JZNodeDebugPacket>);
}
