#include "JZNodeInit.h"
#include "JZNodeFactory.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeDebugPacket.h"
#include "JZNodeObject.h"

void JZNodeInit()
{
    JZNodeFactory::instance()->init();
    JZNodeFunctionManager::instance()->init();   
    JZNodeObjectManager::instance()->init();

    JZNetPackManager::instance()->init();
    JZNetPackManager::instance()->registPack(NetPack_debugPacket,createNetPackFunc<JZNodeDebugPacket>);

    QMetaType::registerDebugStreamOperator<JZNodeObjectPtr>();
}
