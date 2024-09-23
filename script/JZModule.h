#ifndef JZ_MODULE_H_
#define JZ_MODULE_H_

#include "JZNodeObject.h"

#define JZModulePluginInterface_iid "JZModulePlugin.Interface"

class JZCORE_EXPORT JZModule
{
public:
    JZModule();
    virtual ~JZModule();        

    QString name() const;
    QStringList classList() const;
    QStringList functionList() const;
    QStringList depends() const;

    void addRef();
    void release();
    int refCount();
    void unload();

protected:
    virtual void regist() = 0;
    virtual void unregist() = 0;

    QString m_name;
    QStringList m_classList;
    QStringList m_functionList;
    QStringList m_depends;
    int m_refCount;
};
Q_DECLARE_INTERFACE(JZModule, JZModulePluginInterface_iid);

class JZModuleStatic : public JZModule
{
public:
    void init(QString name,QStringList classList, QStringList functionList, QStringList depends);

protected:
    virtual void regist();
    virtual void unregist();
};

class JZModuleManager
{
public:
    static JZModuleManager *instance();
    
    void init();
    void addModule(JZModule *module);

    QStringList moduleList();    
    JZModule *module(QString name);

    bool loadModule(QString name);
    void unloadModule(QString name);
    void unloadAllModule();

protected:
    JZModuleManager();
    ~JZModuleManager();
    
    QList<JZModule*> m_moduleList;
};


#endif