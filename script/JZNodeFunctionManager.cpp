﻿#include "JZNodeFunctionManager.h"
#include "math.h"
#include "JZNodeBind.h"
#include "JZScriptEnvironment.h"

//JZNodeFunctionManager
JZNodeFunctionManager::JZNodeFunctionManager(JZScriptEnvironment *env)
{
    m_userRegist = false;  
    m_env = env;  
}

JZNodeFunctionManager::~JZNodeFunctionManager()
{
}

JZScriptEnvironment *JZNodeFunctionManager::env()
{
    return m_env;
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

const JZFunctionDefine *JZNodeFunctionManager::function(QString funcName) const
{    
    int index = funcName.indexOf(".");
    if (index >= 0)
    {
        QString base_name = funcName.mid(0, index);
        QString function_name = funcName.mid(index + 1);
        auto meta = m_env->objectManager()->meta(base_name);
        if (meta)
            return meta->function(function_name);
    }
    else
    {
        auto it = m_funcDefine.find(funcName);
        if (it != m_funcDefine.end())
            return &it.value();
    }

    return nullptr;
}

void JZNodeFunctionManager::setUserRegist(bool flag)
{
    m_userRegist = flag;
}

QList<const JZFunctionDefine*> JZNodeFunctionManager::functionList() const
{
    QList<const JZFunctionDefine*>  list;
    auto it = m_funcDefine.begin();
    while(it != m_funcDefine.end())
    {
        list << &it.value();
        it++;
    }
    return list;
}

void JZNodeFunctionManager::registCFunction(const JZFunctionDefine &define, QSharedPointer<CFunction> func)
{
    Q_ASSERT(define.isCFunction);

    JZFunction impl;
    impl.define = define;
    impl.cfunc = func; 
    registFunction(define);
    m_funcImpl[define.fullName()] = impl;
}

void JZNodeFunctionManager::setParam(JZFunctionDefine *def,CFunction *func)
{
    auto env = this->env();

    def->paramIn.clear();
    def->paramOut.clear();
    for (int i = 0; i < func->args.size(); i++)
    {
        QString param_name = "input" + QString::number(i);
        int dataType = env->typeidToType(func->args[i]);
        Q_ASSERT_X(dataType != Type_none,"Unkown typeid",qUtf8Printable(func->args[i]));

        def->paramIn.push_back(env->paramDefine(param_name, dataType));
    }
    if (func->result != typeid(void).name())
    {
        QString param_name = "output";
        int dataType = env->typeidToType(func->result);
        Q_ASSERT_X(dataType != Type_none,"Unkown typeid",qUtf8Printable(func->result));

        def->paramOut.push_back(env->paramDefine(param_name, dataType));
    }
}

void JZNodeFunctionManager::registCFunction(QString fullName,bool isFlow, QSharedPointer<CFunction> cfunc)
{
    JZFunctionDefine define;

    QStringList name_list = fullName.split(".");    
    if (name_list.size() == 1)
    {
        define.name = name_list[0];
    }
    else
    {
        define.className = name_list[0];
        define.name = name_list[1];
    }
    setParam(&define,cfunc.data());
    define.isFlowFunction = isFlow;
    define.isCFunction = true;
    registCFunction(define, cfunc);       
}

void JZNodeFunctionManager::registBuiltInFunction(const JZFunctionDefine &define, QSharedPointer<BuiltInFunction> func)
{
    Q_ASSERT(define.isCFunction);

    JZFunction impl;
    impl.define = define;
    impl.builtIn = func; 
    registFunction(define);
    m_funcImpl[define.fullName()] = impl;
}

void JZNodeFunctionManager::unregistFunction(QString name)
{
    auto it = m_funcDefine.find(name);
    if(it == m_funcDefine.end())
        return;

    m_funcDefine.erase(it);
}

void JZNodeFunctionManager::clearUserReigst()
{
    for(int i = 0; i < m_userFuncs.size(); i++)
    {
        m_funcDefine.remove(m_userFuncs[i]);
        m_funcImpl.remove(m_userFuncs[i]);
    }
    m_userFuncs.clear();
}

void JZNodeFunctionManager::registFunction(const JZFunctionDefine &define)
{
    QString fullName = define.fullName();
    Q_ASSERT_X(!m_funcDefine.contains(fullName),"Error",qUtf8Printable(fullName + " already regist"));
    Q_ASSERT(define.paramIn.size() <= 16 && define.paramOut.size() <= 16);

    m_funcDefine[fullName] = define;
    if(m_userRegist)
        m_userFuncs << fullName;
}

void JZNodeFunctionManager::replaceFunction(const JZFunctionDefine &define)
{   
    Q_ASSERT(m_funcDefine.contains(define.name));
    m_funcDefine[define.fullName()] = define;
}

void JZNodeFunctionManager::registFunctionImpl(JZFunction &impl)
{    
    QString full_name = impl.fullName();
    Q_ASSERT(!m_funcImpl.contains(full_name));

    m_funcImpl[full_name] = impl;
    if (m_userRegist && !m_userFuncs.contains(full_name))
        m_userFuncs << full_name;
}

const JZFunction *JZNodeFunctionManager::functionImpl(QString funcName) const
{
    auto it = m_funcImpl.find(funcName);
    if (it == m_funcImpl.end())
        return nullptr; 

    return &it.value();
}