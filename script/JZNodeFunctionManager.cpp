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
}

JZNodeFunctionManager::~JZNodeFunctionManager()
{
    for(int i = 0; i < m_cfuncs.size(); i++)
        delete m_cfuncs[i];
    m_cfuncs.clear();
}

int JZNodeFunctionManager::idToType(QString id)
{
    if(id == typeid(int).name())
        return Type_int;
    else if(id == typeid(int).name())
        return Type_double;
    else if(id == typeid(QString).name())
        return Type_string;

    return Type_unknown;
}

void JZNodeFunctionManager::init()
{    
    registCFunction("exp",jzbind::createFuncion((double (*)(double))(exp)));
    registCFunction("log",jzbind::createFuncion((double (*)(double))(log)));
    registCFunction("log10",jzbind::createFuncion((double (*)(double))(log10)));
    registCFunction("pow",jzbind::createFuncion((double (*)(double,double))(pow)));
    registCFunction("sqrt",jzbind::createFuncion((double (*)(double))(sqrt)));
    registCFunction("ceil",jzbind::createFuncion((double (*)(double))(ceil)));
    registCFunction("floor",jzbind::createFuncion((double (*)(double))(floor)));
    registCFunction("round",jzbind::createFuncion((double (*)(double))(round)));
    registCFunction("fmod",jzbind::createFuncion((double (*)(double,double))(fmod)));

    registCFunction("sin",jzbind::createFuncion((double (*)(double))(sin)));
    registCFunction("cos",jzbind::createFuncion((double (*)(double))(cos)));
    registCFunction("tan",jzbind::createFuncion((double (*)(double))(tan)));
    registCFunction("sinh",jzbind::createFuncion((double (*)(double))(sinh)));
    registCFunction("cosh",jzbind::createFuncion((double (*)(double))(cosh)));
    registCFunction("tanh",jzbind::createFuncion((double (*)(double))(tanh)));
    registCFunction("asin",jzbind::createFuncion((double (*)(double))(asin)));
    registCFunction("acos",jzbind::createFuncion((double (*)(double))(acos)));
    registCFunction("atan",jzbind::createFuncion((double (*)(double))(atan)));
    registCFunction("atan2",jzbind::createFuncion((double (*)(double,double))(atan2)));
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

void JZNodeFunctionManager::registCFunction(QString name,CFunction *func)
{
    m_cfuncs.push_back(func);

    FunctionDefine define;
    define.name = name;
    define.isCFunction = true;
    define.cfunc = func;

    for(int i = 0; i < func->args.size(); i++)
    {
        JZNodePin prop;
        prop.setFlag(Prop_param | Prop_in);
        prop.setDataType({idToType(func->args[i])});
        define.paramIn.push_back(prop);
    }    
    if(func->result != typeid(void).name())
    {
        JZNodePin prop_out;
        prop_out.setFlag(Prop_param | Prop_out);
        prop_out.setDataType({idToType(func->result)});
        define.paramOut.push_back(prop_out);    
    }
    registFunction(define);
}

void JZNodeFunctionManager::registFunction(const FunctionDefine &define)
{
    Q_ASSERT(!m_funcMap.contains(define.name));
    m_funcMap[define.name] = define;
}
