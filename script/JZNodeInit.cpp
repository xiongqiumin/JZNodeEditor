#include <QDebug>
#include <QMetaObject>
#include "JZNodeFactory.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeDebugPacket.h"
#include "JZNodeObject.h"
#include "JZNodeEngine.h"
#include "LogManager.h"
#include "JZNodeVariableBind.h"
#include "JZModule.h"
#include "JZNodeInit.h"
#include "modules/opencv/JZModuleOpencv.h"
#include "modules/camera/JZModuleCamera.h"
#include "modules/modbus/JZModuleModbus.h"
#include "LogManager.h"

QDebug operator<<(QDebug dbg, const JZNodeObjectHolder ptr)
{
    Q_ASSERT(ptr.object());
    dbg << JZNodeType::debugString(ptr.object());
    return dbg;
}

void JZNodeInit()
{
    LogManagerInit();

    qRegisterMetaType<JZNodeRuntimeError>("JZNodeRuntimeError");
    qRegisterMetaType<UnitTestResultPtr>("UnitTestResultPtr");
    qRegisterMetaTypeStreamOperators<JZObjectNull>("JZObjectNull");
    qRegisterMetaTypeStreamOperators<JZEnum>("JZEnum");
    qRegisterMetaTypeStreamOperators<JZFunctionPointer>("JZFunctionPointer");
    
    QMetaType::registerDebugStreamOperator<JZNodeObjectHolder>();
    QMetaType::registerEqualsComparator<JZNodeObjectHolder>();    

    JZNodeType::init();
    JZNodeFactory::instance()->init();
    JZProjectInit();
    JZNodeEngine::regist();

    JZNetPackManager::instance()->init();
    JZNetPackManager::instance()->registPack(NetPack_debugPacket,JZNetPackCreate<JZNodeDebugPacket>);          

    BindManager::instance()->init();    

    auto module_inst = JZModuleManager::instance();
    module_inst->addModule(new JZModuleOpencv());
    module_inst->addModule(new JZModuleCamera());
    //module_inst->addModule(new JZModuleModbus());
    module_inst->initModules();
}
