#include "JZNodeInit.h"
#include "JZNodeFactory.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeDebugPacket.h"
#include "JZNodeObject.h"
#include "JZNodeEngine.h"
#include "LogManager.h"
#include "sample/ImageModule/ImageModule.h"
#include "3rd/jzupdate/JZUpdatePack.h"

void JZNodeInit()
{
    qRegisterMetaType<JZNodeRuntimeError>("JZNodeRuntimeError");
    qRegisterMetaType<UnitTestResultPtr>("UnitTestResultPtr");
    qRegisterMetaTypeStreamOperators<JZObjectNull>("JZObjectNull");
    qRegisterMetaTypeStreamOperators<JZEnum>("JZEnum");
    qRegisterMetaTypeStreamOperators<JZFunctionPointer>("JZFunctionPointer");
    QMetaType::registerEqualsComparator<JZNodeObjectPtr>();

    JZUpdatePackRegist();
    ImageModule::init();

    JZNodeType::init();
    JZNodeFactory::instance()->init();    
    JZNodeFunctionManager::instance()->init();   
    JZNodeObjectManager::instance()->init();    
    JZNodeEngine::regist();    

    JZNetPackManager::instance()->init();
    JZNetPackManager::instance()->registPack(NetPack_debugPacket,JZNetPackCreate<JZNodeDebugPacket>);

    JZNodeFunctionManager::instance()->setUserRegist(true);
}
