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
#include "JZNodeParamDelegate.h"
#include "JZModule.h"

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
    InitJZProject();

    JZNetPackManager::instance()->init();
    JZNetPackManager::instance()->registPack(NetPack_debugPacket,JZNetPackCreate<JZNodeDebugPacket>);
       
    InitCustomPlot();
    InitParamDelegate();    

    BindManager::instance()->init();    
    JZModuleManager::instance()->init();
}
