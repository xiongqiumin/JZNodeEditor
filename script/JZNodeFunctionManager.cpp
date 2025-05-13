#include "JZNodeFunctionManager.h"
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
    registCFunction("loge",false,jzbind::createFuncion((double (*)(double))(log)));
    registCFunction("log2", false, jzbind::createFuncion((double (*)(double))(log2)));
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
    auto coor = JZFunctionHelper::splitFunction(funcName);

    if (!coor.className.isEmpty())
    {
        auto meta = m_env->objectManager()->meta(coor.className);
        if (meta)
            return meta->function(coor.name);
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

QStringList JZNodeFunctionManager::functionList() const
{    
    return m_funcDefine.keys();
}

JZFunctionDefine* JZNodeFunctionManager::registCFunction(const JZFunctionDefine &define, QSharedPointer<CFunction> func)
{
    Q_ASSERT(define.isCFunction);

    JZFunction impl;
    impl.define = define;
    impl.cfunc = func; 
    if(define.className.isEmpty())
        registFunction(define);
    
    m_funcImpl[define.fullName()] = impl;
    return &m_funcDefine[define.fullName()];
}

void JZNodeFunctionManager::setParam(JZFunctionDefine *def,CFunction *func)
{
    auto env = this->env();

    def->paramIn.clear();
    def->paramOut.clear();
    for (int i = 0; i < func->args.size(); i++)
    {
        QString param_name = "input" + QString::number(i);
        int dataType = env->ctypeidToType(func->args[i]);
        Q_ASSERT_X(dataType != Type_none,"Unkown typeid",qUtf8Printable(func->args[i]));

        def->paramIn.push_back(env->paramDefine(param_name, dataType));
    }
    if (func->result != typeid(void).name())
    {
        QString param_name = "output";
        int dataType = env->ctypeidToType(func->result);
        Q_ASSERT_X(dataType != Type_none,"Unkown typeid",qUtf8Printable(func->result));

        def->paramOut.push_back(env->paramDefine(param_name, dataType));
    }
}

JZFunctionDefine *JZNodeFunctionManager::registCFunction(QString fullName,bool isFlow, QSharedPointer<CFunction> cfunc)
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
    return registCFunction(define, cfunc);       
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
    m_funcImpl.remove(name);
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
    Q_ASSERT(define.className.isEmpty());
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

void JZNodeFunctionManager::replaceFunctionImpl(JZFunction &impl)
{
    m_funcImpl[impl.fullName()] = impl;
}

const JZFunction *JZNodeFunctionManager::functionImpl(QString funcName) const
{
    auto it = m_funcImpl.find(funcName);
    if (it == m_funcImpl.end())
        return nullptr; 

    return &it.value();
}