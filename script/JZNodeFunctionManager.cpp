#include "JZNodeFunctionManager.h"
#include "math.h"
#include "JZNodeBind.h"

//JZNodeFunctionManager
JZNodeFunctionManager *JZNodeFunctionManager::instance()
{
    static JZNodeFunctionManager inst;
    return &inst;
}

JZNodeFunctionManager::JZNodeFunctionManager()
{
    m_userRegist = false;
}

JZNodeFunctionManager::~JZNodeFunctionManager()
{
}

void JZNodeFunctionManager::init()
{
    registCFunction("rand", false, jzbind::createFuncion(rand));
    registCFunction("exp",false,jzbind::createFuncion((double (*)(double))(exp)));
    registCFunction("log",false,jzbind::createFuncion((double (*)(double))(log)));
    registCFunction("log10",false,jzbind::createFuncion((double (*)(double))(log10)));
    registCFunction("pow",false,jzbind::createFuncion((double (*)(double,double))(pow)));
    registCFunction("sqrt",false,jzbind::createFuncion((double (*)(double))(sqrt)));
    registCFunction("ceil",false,jzbind::createFuncion((double (*)(double))(ceil)));
    registCFunction("floor",false,jzbind::createFuncion((double (*)(double))(floor)));
    registCFunction("round",false,jzbind::createFuncion((double (*)(double))(round)));
    registCFunction("fmod",false,jzbind::createFuncion((double (*)(double,double))(fmod)));

    registCFunction("sin",false,jzbind::createFuncion((double (*)(double))(sin)));
    registCFunction("cos",false,jzbind::createFuncion((double (*)(double))(cos)));
    registCFunction("tan",false,jzbind::createFuncion((double (*)(double))(tan)));
    registCFunction("sinh",false,jzbind::createFuncion((double (*)(double))(sinh)));
    registCFunction("cosh",false,jzbind::createFuncion((double (*)(double))(cosh)));
    registCFunction("tanh",false,jzbind::createFuncion((double (*)(double))(tanh)));
    registCFunction("asin",false,jzbind::createFuncion((double (*)(double))(asin)));
    registCFunction("acos",false,jzbind::createFuncion((double (*)(double))(acos)));
    registCFunction("atan",false,jzbind::createFuncion((double (*)(double))(atan)));
    registCFunction("atan2",false,jzbind::createFuncion((double (*)(double,double))(atan2)));
}

const FunctionDefine *JZNodeFunctionManager::function(QString funcName)
{
    auto it = m_funcMap.find(funcName);
    if (it != m_funcMap.end())
        return &it.value();
    else
    {
        int index = funcName.indexOf(".");
        if (index >= 0)
        {
            QString base_name = funcName.mid(0, index);
            QString function_name = funcName.mid(index + 1);
            auto meta = JZNodeObjectManager::instance()->meta(base_name);
            if (meta)
                return meta->function(function_name);
        }        
        return nullptr;
    }
}

void JZNodeFunctionManager::loadLibrary(QString filename)
{    
}

void JZNodeFunctionManager::setUserRegist(bool flag)
{
    m_userRegist = flag;
}

QList<const FunctionDefine*> JZNodeFunctionManager::functionList()
{
    QList<const FunctionDefine*>  list;
    auto it = m_funcMap.begin();
    while(it != m_funcMap.end())
    {
        list << &it.value();
        it++;
    }
    return list;
}

void JZNodeFunctionManager::registCFunction(QString name,bool isFlow, QSharedPointer<CFunction> func)
{
    FunctionDefine define;
    define.name = name;
    define.isCFunction = true;
    define.cfunc = func;
    define.isFlowFunction = isFlow;
    define.updateCFunctionParam();
    registFunction(define);
}

void JZNodeFunctionManager::unregistFunction(QString name)
{
    auto it = m_funcMap.find(name);
    if(it == m_funcMap.end())
        return;

    m_funcMap.erase(it);
}

void JZNodeFunctionManager::clearUserReigst()
{
    for(int i = 0; i < m_userFuncs.size(); i++)
        m_funcMap.remove(m_userFuncs[i]);
}

void JZNodeFunctionManager::registFunction(const FunctionDefine &define)
{
    Q_ASSERT(!m_funcMap.contains(define.name));
    m_funcMap[define.name] = define;    
    if(m_userRegist)
        m_userFuncs << define.name;
}

void JZNodeFunctionManager::replaceFunction(const FunctionDefine &define)
{
    m_funcMap[define.name] = define;
}