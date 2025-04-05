#include <QPluginLoader>
#include <QDir>
#include <QApplication>
#include <QSysInfo>
#include "JZModule.h"
#include "JZNodeFunctionManager.h"

//JZModule
JZModule::JZModule()
{ 
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
