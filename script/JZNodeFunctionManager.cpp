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

const JZFunctionDefine *JZNodeFunctionManager::function(QString funcName)
{
    auto it = m_funcMap.find(funcName);
    if (it != m_funcMap.end())
        return &it->funcDefine;
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

QList<const JZFunctionDefine*> JZNodeFunctionManager::functionList()
{
    QList<const JZFunctionDefine*>  list;
    auto it = m_funcMap.begin();
    while(it != m_funcMap.end())
    {
        list << &it->funcDefine;
        it++;
    }
    return list;
}

void JZNodeFunctionManager::registCFunction(QString fullName,bool isFlow, QSharedPointer<CFunction> cfunc)
{
    QStringList name_list = fullName.split(".");

    JZFunction define;
    if (name_list.size() == 1)
    {
        define.name = name_list[0];
    }
    else
    {
        define.className = name_list[0];
        define.name = name_list[1];
    }
    define.cfunc = cfunc;
    define.flow = isFlow;
    
    for (int i = 0; i < cfunc->args.size(); i++)
    {
        QString name = "input" + QString::number(i);
        int dataType = JZNodeType::typeidToType(cfunc->args[i]);
        Q_ASSERT(dataType != Type_none);

        define.paramIn.push_back(JZParam(name, dataType));
    }
    if (cfunc->result != typeid(void).name())
    {
        QString name = "output";
        int dataType = JZNodeType::typeidToType(cfunc->result);
        Q_ASSERT(dataType != Type_none);

        define.paramOut.push_back(JZParam(name, dataType));
    }

    registFunctionImpl(define);
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

void JZNodeFunctionManager::registFunction(const JZFunctionDefine &define)
{
    QString fullName = define.fullName();
    Q_ASSERT(!m_funcMap.contains(fullName));

    m_funcMap[fullName].funcDefine = define;
    if(m_userRegist)
        m_userFuncs << fullName;
}

void JZNodeFunctionManager::replaceFunction(const JZFunctionDefine &define)
{
    m_funcMap[define.fullName()].funcDefine = define;
}

void JZNodeFunctionManager::registFunctionImpl(JZFunction &impl)
{    
    m_funcMap[impl.fullName()].funcDefine = impl.define();
    m_funcMap[impl.fullName()].funcImpl = impl;
}

const JZFunction *JZNodeFunctionManager::functionImpl(QString funcName)
{
    auto it = m_funcMap.find(funcName);
    if (it == m_funcMap.end())
        return nullptr; 

    return &it->funcImpl;
}