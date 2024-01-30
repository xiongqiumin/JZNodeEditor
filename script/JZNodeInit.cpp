#include "JZNodeInit.h"
#include "JZNodeFactory.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeDebugPacket.h"
#include "JZNodeObject.h"
#include "JZNodeEngine.h"
#include "LogManager.h"

void JZNodeInit()
{
    qRegisterMetaType<JZNodeRuntimeError>("JZNodeRuntimeError");
    qRegisterMetaTypeStreamOperators<JZObjectNull>("JZObjectNull");

    LogManager::instance()->init();
    LogManager::instance()->addModule(Log_Compiler,"Compiler");
    LogManager::instance()->addModule(Log_Runtime,"Runtime");

    JZNodeType::init();
    JZNodeFactory::instance()->init();    
    JZNodeFunctionManager::instance()->init();   
    JZNodeObjectManager::instance()->init();    
    JZNodeEngine::regist();    

    JZNetPackManager::instance()->init();
    JZNetPackManager::instance()->registPack(NetPack_debugPacket,createNetPackFunc<JZNodeDebugPacket>);

    JZNodeFunctionManager::instance()->setUserRegist(true);
}
