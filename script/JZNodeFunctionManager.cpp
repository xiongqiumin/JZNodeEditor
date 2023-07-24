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
    qDeleteAll(m_cfuncs);
    qDeleteAll(m_csingles);
}

void JZNodeFunctionManager::init()
{
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
        return nullptr;    
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

void JZNodeFunctionManager::registCFunction(QString name,bool isFlow,CFunction *func)
{
    m_cfuncs.push_back(func);

    FunctionDefine define;
    define.name = name;
    define.isCFunction = true;
    define.cfunc = func;
    define.isFlowFunction = isFlow;

    for(int i = 0; i < func->args.size(); i++)
    {
        JZParamDefine prop;
        prop.name = "input" + QString::number(i);
        prop.dataType = JZNodeType::typeidToType(func->args[i]);
        Q_ASSERT(prop.dataType != Type_none);

        define.paramIn.push_back(prop);
    }    
    if(func->result != typeid(void).name())
    {
        JZParamDefine prop;
        prop.name = "output";
        prop.dataType = JZNodeType::typeidToType(func->result);
        Q_ASSERT(prop.dataType != Type_none);

        define.paramOut.push_back(prop);
    }
    registFunction(define);
}

void JZNodeFunctionManager::unregistFunction(QString name)
{
    auto it = m_funcMap.find(name);
    if(it == m_funcMap.end())
        return;

    if(it->isCFunction)
    {
        m_cfuncs.removeAll(it->cfunc);
        delete it->cfunc;
    }
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

void JZNodeFunctionManager::registCSingle(CSingle *single)
{
    m_csingles.push_back(single);
}

void JZNodeFunctionManager::unregistCSingle(CSingle *single)
{
    m_csingles.removeAll(single);
}
