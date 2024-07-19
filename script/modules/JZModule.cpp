#include "JZModule.h"

//JZModuleInfo
JZModuleInfo::JZModuleInfo()
{
    createFunc = nullptr;
}

//JZModule
JZModule::JZModule()
{
}

JZModule::~JZModule()
{
    m_refCount = 0;
}

const JZModuleInfo &JZModule::info() const
{
    return m_info;
}

void JZModule::addRef()
{
    m_refCount++;
}

void JZModule::release()
{
    m_refCount--;
}

int JZModule::refCount()
{
    return m_refCount;
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

void JZModuleManager::registModule(JZModuleInfo info)
{
    m_moduleInfoList.push_back(info);
}

QStringList JZModuleManager::moduleList()
{
    QStringList list;
    for(int i = 0; i < m_moduleInfoList.size(); i++)
        list << m_moduleInfoList[i].name;
    
    return list;
}

JZModuleInfo *JZModuleManager::moduleInfo(QString name)
{
    for(int i = 0; i < m_moduleInfoList.size(); i++)
    {
        if(m_moduleInfoList[i].name == name)
            return &m_moduleInfoList[i];
    }

    return nullptr;
}

JZModule *JZModuleManager::module(QString name)
{
    for(int i = 0; i < m_moduleList.size(); i++)
    {
        if(m_moduleList[i]->info().name == name)
            return m_moduleList[i];
    }

    return nullptr;
}

bool JZModuleManager::loadModule(QString name)
{
    JZModule *m = module(name);
    if(m)
    {
        m->addRef();
    }
    else
    {
        JZModuleInfo *info = moduleInfo(name);
        if(!info)
            return false;
        if(info->createFunc)
            m = info->createFunc();

        m->addRef();
        for(int i = 0; i < info->depends.size(); i++)
        {
            if(!loadModule(info->depends[i]))
                return false;
        }
        m->regist();
    }
    return true;
}

void JZModuleManager::unloadModule(QString name)
{
    JZModule *m = module(name);
    Q_ASSERT(m);
    
    m->release();
    if(m->refCount() == 0)
    {
        m->unregist();
        auto info = m->info();    
        for(int i = 0; i < info.depends.size(); i++)
            unloadModule(info.depends[i]);
    }
}

