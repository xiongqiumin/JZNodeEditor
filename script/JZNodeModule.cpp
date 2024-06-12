#include "JZNodeModule.h"

//JZNodeModule
JZNodeModule::JZNodeModule()
{    
}

JZNodeModule::~JZNodeModule()
{
}

QString JZNodeModule::name()
{
    return m_name;
}

void JZNodeModule::setName(QString name)
{
    m_name = name;
}

QStringList JZNodeModule::depends()
{
    return m_depends;
}

void JZNodeModule::setDepend(QStringList list)
{
    m_depends = list;
}

//JZNodeModuleManager
JZNodeModuleManager::ModuleLoader::ModuleLoader()
{
    count = 0;
    externCount = 0;
}

JZNodeModuleManager *JZNodeModuleManager::instance()
{
    static JZNodeModuleManager inst;
    return &inst;
}

JZNodeModuleManager::JZNodeModuleManager()
{

}

JZNodeModuleManager::~JZNodeModuleManager()
{
    unloadAll();
}

void JZNodeModuleManager::registModule(const JZNodeModule &module)
{
    m_modules.push_back(module);
}

JZNodeModule *JZNodeModuleManager::module(QString name)
{
    for(int i = 0; i < m_modules.size(); i++)
    {
        if(m_modules[i].name() == name)
            return &m_modules[i];
    }

    return nullptr;
}

bool JZNodeModuleManager::load(QString name)
{
    QStringList finished;
    if(!load(name,finished))
        return false;

    m_interfaces[name].externCount++;
    return true;
}

bool JZNodeModuleManager::unload(QString name)
{
    if(!m_interfaces.contains(name))
        return false;

    QStringList finished;
    m_interfaces[name].externCount--;
    return unload(name,finished);
}

void JZNodeModuleManager::unloadAll()
{
    QStringList list = m_interfaces.keys();
    for(int module_idx = 0; module_idx < m_interfaces.size(); module_idx++)
    {
        QString module_name = list[module_idx];
        if(m_interfaces.contains(module_name))
        {
            ModuleLoader &info = m_interfaces[module_name];
            for(int i = 0; i < info.externCount; i++)
                unload(module_name);
        }
    }
    m_interfaces.clear();
}

bool JZNodeModuleManager::load(QString name,QStringList &finished)
{
    if(finished.contains(name))
        return true;
    finished.push_back(name);

    if(m_interfaces.contains(name))
    {
        m_interfaces[name].count++;
        return true;
    }

    JZNodeModule *module = this->module(name);
    if(!module)
        return false;

    auto depends = module->depends();
    for(int i = 0; i < depends.size(); i++)
    {
        if(!load(depends[i],finished))
            return false;
    }

    ModuleLoader info;
    info.loader = new QPluginLoader(name);
    if(!info.loader->load())
    {
        qDebug() << "try load plugin" << name << "failed:" << info.loader->errorString();
        delete info.loader;
        return false;
    }
    
    JZNodeModuleInterface *interface = qobject_cast<JZNodeModuleInterface*>(info.loader->instance());
    interface->regist();
    info.count++;
    info.interface = interface;
    m_interfaces[name] = info;
    return true;
}

bool JZNodeModuleManager::unload(QString name,QStringList &finished)
{
    if(finished.contains(name))
        return true;
    finished.push_back(name);

    if(!m_interfaces.contains(name))
        return false;

    ModuleLoader &info = m_interfaces[name];
    info.count--;
    if(info.count == 0)
    {
        info.interface->unregist();
        delete info.interface;

        JZNodeModule *module = this->module(name);
        auto depends = module->depends();
        for(int i = 0; i < depends.size(); i++)
        {
            unload(depends[i],finished);
        }

        info.loader->unload();
        delete info.loader;

        m_interfaces.remove(name);
    }
    return true;
}