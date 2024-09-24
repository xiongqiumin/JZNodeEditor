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

//JZModuleStatic
void JZModuleStatic::init(QString name, QStringList classList, QStringList functionList, QStringList depends)
{
    m_name = name;
    m_classList = classList;
    m_functionList = functionList;
    m_depends = depends;
}

void JZModuleStatic::regist(JZScriptEnvironment *env)
{
}

void JZModuleStatic::unregist(JZScriptEnvironment *env)
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
    QString plugin_path = qApp->applicationDirPath() + "/modules";
    QString old_cur_path = QDir::currentPath();
    QDir::setCurrent(plugin_path);

    QString osType = QSysInfo::productType();
    QString librarySuffix;
    if (osType == "windows") {
        librarySuffix = ".dll";
    }
    else if (osType == "macos") {
        librarySuffix = ".dylib";
    }
    else {
        librarySuffix = ".so";
    }
    
    QDir dir(plugin_path);
    QStringList list = dir.entryList(QDir::Files);
    for (int i = 0; i < list.size(); i++)
    {
        if (!list[i].endsWith(librarySuffix) || !list[i].startsWith("Module"))
            continue;

        QString module_path = plugin_path + "/" + list[i];
        QPluginLoader pluginLoader(module_path);        
        QObject* plugin = pluginLoader.instance();
        if (plugin) {
            auto name = plugin->metaObject()->className();
            JZModule* pInterface = qobject_cast<JZModule*>(plugin);
            if (pInterface)
                addModule(pInterface);
        }
        else
        {            
            qDebug() << "load plugin" << module_path <<  "failed.";
        }
    }

    QDir::setCurrent(old_cur_path);
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
