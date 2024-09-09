#include "JZNodeInit.h"
#include "JZNodeFactory.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeDebugPacket.h"
#include "JZNodeObject.h"
#include "JZNodeEngine.h"
#include "LogManager.h"
#include "sample/ImageModule/ImageModule.h"
#include "3rd/jzupdate/JZUpdatePack.h"
#include "script/modules/JZModuleModbus.h"
#include "3rd/qcustomplot/JZPlotConfg.h"
#include "JZNodeVariableBind.h"

void JZNodeInit()
{
    qRegisterMetaType<JZNodeRuntimeError>("JZNodeRuntimeError");
    qRegisterMetaType<UnitTestResultPtr>("UnitTestResultPtr");
    qRegisterMetaTypeStreamOperators<JZObjectNull>("JZObjectNull");
    qRegisterMetaTypeStreamOperators<JZEnum>("JZEnum");
    qRegisterMetaTypeStreamOperators<JZFunctionPointer>("JZFunctionPointer");
    QMetaType::registerEqualsComparator<JZNodeObjectPtr>();

    JZUpdatePackRegist();    

    JZNodeType::init();
    JZNodeFactory::instance()->init();    
    JZNodeFunctionManager::instance()->init();   
    JZNodeObjectManager::instance()->init();    
    JZNodeEngine::regist();    

    JZNetPackManager::instance()->init();
    JZNetPackManager::instance()->registPack(NetPack_debugPacket,JZNetPackCreate<JZNodeDebugPacket>);

    ImageModule::init();
    InitModbus();
    InitCustomPlot();

    JZNodeFunctionManager::instance()->setUserRegist(true);
    JZNodeObjectManager::instance()->setUserRegist(true);

    BindManager::instance()->init();
}
