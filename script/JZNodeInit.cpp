#include "JZNodeInit.h"
#include "JZNodeFactory.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeDebugPacket.h"
#include "JZNodeObject.h"
#include "JZNodeEngine.h"
#include "LogManager.h"
#include "3rd/jzupdate/JZUpdatePack.h"
#include "3rd/qcustomplot/JZPlotConfg.h"
#include "JZNodeVariableBind.h"

#include "modules/JZModuleModbus.h"
#include "sample/ImageModule/ImageModule.h"
#include "JZNodeEditorWidget.h"

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
   
    InitEditorWidget();
    InitCustomPlot();

    JZNodeFunctionManager::instance()->setUserRegist(true);
    JZNodeObjectManager::instance()->setUserRegist(true);

    BindManager::instance()->init();

    //load module
    auto module_modbus = new JZModuleModbus();
    auto module_image = new ModuleImageSample();
    JZModuleManager::instance()->addModule(module_modbus);
    JZModuleManager::instance()->addModule(module_image);
}
