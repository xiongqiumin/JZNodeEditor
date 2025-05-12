#include <QDebug>
#include <QMetaObject>
#include "JZNodeFactory.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeDebugPacket.h"
#include "JZNodeObject.h"
#include "JZNodeEngine.h"
#include "LogManager.h"
#include "mvvm/JZNodeVariableBind.h"
#include "JZNodeBuildThread.h"
#include "JZModule.h"
#include "JZNodeInit.h"
#include "modules/opencv/JZModuleOpencv.h"
#include "modules/camera/JZModuleCamera.h"
#include "modules/communication/JZModuleComm.h"
#include "modules/model/JZModuleModel.h"
#include "modules/vision/JZModuleVision.h"
#include "LogManager.h"
#include "runtime/JZWidgetBind.h"
#include "JZScriptUnitTest.h"

QDebug operator<<(QDebug dbg, const JZNodeObjectPointer ptr)
{
    Q_ASSERT(ptr.object());
    dbg << JZNodeType::debugString(ptr.object());
    return dbg;
}

void JZNodeInit()
{
    qRegisterMetaType<JZNodeRuntimeError>("JZNodeRuntimeError");    
    qRegisterMetaType<JZNodeBuildResultPtr>("JZNodeBuildResultPtr");
    qRegisterMetaTypeStreamOperators<JZNodeObjectNull>("JZNodeObjectNull");
    qRegisterMetaTypeStreamOperators<JZEnum>("JZEnum");
    qRegisterMetaTypeStreamOperators<JZFunctionPointer>("JZFunctionPointer");
    
    QMetaType::registerDebugStreamOperator<JZNodeObjectPointer>();
    QMetaType::registerEqualsComparator<JZNodeObjectPointer>();    

    JZNodeType::init();
    JZProjectInit();
    JZNodeEngine::regist();

    JZNetPackManager::instance()->init();
    JZNetPackManager::instance()->registPack(NetPack_debugPacket,JZNetPackCreate<JZNodeDebugPacket>);              

    JZBindManager::instance()->init();    
    
    auto module_inst = JZModuleManager::instance();
    module_inst->addModule(new JZModuleComm());
    module_inst->addModule(new JZModuleOpencv());
    module_inst->addModule(new JZModuleCamera());
    module_inst->addModule(new JZModuleModel());
    module_inst->addModule(new JZModuleVision());
    module_inst->initModules();
}
