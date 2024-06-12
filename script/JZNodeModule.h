#ifndef JZNODE_MODULE_H_
#define JZNODE_MODULE_H_

#include <QPluginLoader>

class JZNodeModule
{
public:
    JZNodeModule();
    ~JZNodeModule();

    QString name();
    void setName(QString name);

    QStringList depends();
    void setDepend(QStringList list);

protected:
    QString m_name;
    QStringList m_depends;
};

class JZNodeModuleInterface
{
public:
    virtual ~JZNodeModuleInterface(){}
    virtual bool regist() = 0;
    virtual bool unregist() = 0;    
};
Q_DECLARE_INTERFACE(JZNodeModuleInterface, "JZNode.ModuleInterface")

class JZNodeModuleManager
{
public:
    static JZNodeModuleManager *instance();

    void registModule(const JZNodeModule &module);
    bool load(QString name);
    bool unload(QString name);
    void unloadAll();

protected: 
    struct ModuleLoader
    {
        ModuleLoader();

        JZNodeModuleInterface *interface;
        QPluginLoader *loader;
        int count;
        int externCount;
    };
    
    JZNodeModuleManager();
    ~JZNodeModuleManager();

    JZNodeModule *module(QString name);
    bool load(QString name,QStringList &finished);
    bool unload(QString name,QStringList &finished);

    QList<JZNodeModule> m_modules;
    QMap<QString,ModuleLoader> m_interfaces;
};

#endif