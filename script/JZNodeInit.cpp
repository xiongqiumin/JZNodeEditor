#include "JZNodeInit.h"
#include "JZNodeFactory.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeDebugPacket.h"
#include "JZNodeObject.h"
#include "JZNodeEngine.h"

void JZNodeInit()
{
    JZNodeType::init();
    JZNodeFactory::instance()->init();
    JZNodeFunctionManager::instance()->init();   
    JZNodeObjectManager::instance()->init();

    JZNetPackManager::instance()->init();
    JZNetPackManager::instance()->registPack(NetPack_debugPacket,createNetPackFunc<JZNodeDebugPacket>);    

    qRegisterMetaType<JZNodeRuntimeError>("JZNodeRuntimeError");
    qRegisterMetaTypeStreamOperators<JZObjectNull>("JZObjectNull");
}
