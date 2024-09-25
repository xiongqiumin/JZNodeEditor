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
#include "3rd/jzupdate/JZUpdatePack.h"

QDebug operator<<(QDebug dbg, const JZNodeObjectPtr ptr)
{
    Q_ASSERT(ptr.object());
    dbg << JZNodeType::debugString(ptr.object());
    return dbg;
}

void JZNodeInit()
{
    qRegisterMetaType<JZNodeRuntimeError>("JZNodeRuntimeError");
    qRegisterMetaType<UnitTestResultPtr>("UnitTestResultPtr");
    qRegisterMetaTypeStreamOperators<JZObjectNull>("JZObjectNull");
    qRegisterMetaTypeStreamOperators<JZEnum>("JZEnum");
    qRegisterMetaTypeStreamOperators<JZFunctionPointer>("JZFunctionPointer");
    
    QMetaType::registerDebugStreamOperator<JZNodeObjectPtr>();
    QMetaType::registerEqualsComparator<JZNodeObjectPtr>();

    JZUpdatePackRegist();    

    JZNodeType::init();
    JZNodeFactory::instance()->init();
    InitJZProject();
    JZNodeEngine::regist();

    JZNetPackManager::instance()->init();
    JZNetPackManager::instance()->registPack(NetPack_debugPacket,JZNetPackCreate<JZNodeDebugPacket>);          

    BindManager::instance()->init();    
    JZModuleManager::instance()->init();
}
