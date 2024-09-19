#include "JZModule.h"

//JZModule
JZModule::JZModule()
{
    m_refCount = 0;
}

JZModule::~JZModule()
{    
}

QString JZModule::name() const
{
    return m_name;
}

QStringList JZModule::classList() const
{
    return m_classList;
}

QStringList JZModule::functionList() const
{
    return m_functionList;
}

QStringList JZModule::depends() const
{
    return m_depends;
}

void JZModule::addRef()
{
    m_refCount++;
    if (m_refCount == 1)
        regist();
}

void JZModule::release()
{
    m_refCount--;
    if (m_refCount == 0)
        unregist();
}

int JZModule::refCount()
{
    return m_refCount;
}

void JZModule::unload()
{
    if (m_refCount > 0)
        unregist();
    m_refCount = 0;
}

//JZModuleStatic
void JZModuleStatic::init(QString name, QStringList classList, QStringList functionList, QStringList depends)
{
    m_name = name;
    m_classList = classList;
    m_functionList = functionList;
    m_depends = depends;
}

void JZModuleStatic::regist()
{
}

void JZModuleStatic::unregist()
{
}

//JZModuleManager
JZModuleManager *JZModuleManager::instance()
{
    static JZModuleManager inst;
    return &inst;
}

JZModuleManager::JZModuleManager()
{

}

JZModuleManager::~JZModuleManager()
{
    qDeleteAll(m_moduleList);
}

void JZModuleManager::init()
{

}

void JZModuleManager::addModule(JZModule *module)
{
    m_moduleList.push_back(module);
}

void JZModuleManager::unloadAllModule()
{
    for (int i = 0; i < m_moduleList.size(); i++)
    {
        m_moduleList[i]->unload();
    }
}

QStringList JZModuleManager::moduleList()
{
    QStringList list;
    for(int i = 0; i < m_moduleList.size(); i++)
        list << m_moduleList[i]->name();
    
    return list;
}


JZModule *JZModuleManager::module(QString name)
{
    for(int i = 0; i < m_moduleList.size(); i++)
    {
        if(m_moduleList[i]->name() == name)
            return m_moduleList[i];
    }

    return nullptr;
}

bool JZModuleManager::loadModule(QString name)
{
    JZModule *m = module(name);    
    if (!m)
        return false;
    
    m->addRef();    
    auto depends = m->depends();
    for(int i = 0; i < depends.size(); i++)
    {
        if(!loadModule(depends[i]))
            return false;
    }        
    return true;
}

void JZModuleManager::unloadModule(QString name)
{
    JZModule *m = module(name);
    Q_ASSERT(m);      

    auto depends = m->depends();
    for(int i = 0; i < depends.size(); i++)
        unloadModule(depends[i]);
}

