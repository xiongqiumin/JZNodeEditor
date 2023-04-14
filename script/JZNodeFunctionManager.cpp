#include "JZNodeFunctionManager.h"

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
}

void JZNodeFunctionManager::init()
{    
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
    if (it != m_funcMap.end())
    {
        list << &it.value();
        it++;
    }
    return list;
}

void JZNodeFunctionManager::registFunction(const FunctionDefine &define)
{
    Q_ASSERT(!m_funcMap.contains(define.name));
    m_funcMap[define.name] = define;
}

void JZNodeFunctionManager::callCFunction(const FunctionDefine *def, const QVariantList &paramIn, QVariantList &paramOut)
{    
    int type = def->functionType;
    void *ptr = def->pointer;
    switch (type)
    {
    case Func_ii_i:
    {
        int p0 = paramIn[0].value<int>();
        int p1 = paramIn[1].value<int>();
        int ret = ((int (*)(int, int))ptr)(p0, p1);
        paramOut.push_back(ret);
        break;
    }
    default:
        break;
    }
}
