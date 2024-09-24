#ifndef JZ_MODULE_H_
#define JZ_MODULE_H_

#include "JZNodeObject.h"

#define JZModulePluginInterface_iid "JZModulePlugin.Interface"

class JZScriptEnvironment;
class JZCORE_EXPORT JZModule
{
public:
    JZModule();
    virtual ~JZModule();        

    virtual void regist(JZScriptEnvironment *env) = 0;
    virtual void unregist(JZScriptEnvironment *env) = 0;

    QString name() const;
    QStringList classList() const;
    QStringList functionList() const;
    QStringList depends() const;

protected:    
    QString m_name;
    QStringList m_classList;
    QStringList m_functionList;
    QStringList m_depends;    
};
Q_DECLARE_INTERFACE(JZModule, JZModulePluginInterface_iid);

class JZModuleStatic : public JZModule
{
public:
    void init(QString name,QStringList classList, QStringList functionList, QStringList depends);

protected:
    virtual void regist(JZScriptEnvironment *env);
    virtual void unregist(JZScriptEnvironment *env);
};

class JZModuleManager
{
public:
    static JZModuleManager *instance();
    
    void init();
    void addModule(JZModule *module);

    QStringList moduleList();    
    JZModule *module(QString name);    

protected:
    JZModuleManager();
    ~JZModuleManager();
    
    QList<JZModule*> m_moduleList;
};


#endif