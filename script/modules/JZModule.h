#ifndef JZ_MODULE_H_
#define JZ_MODULE_H_

#include "JZNodeObject.h"

class JZModule;
typedef JZModule *(*JZModuleCreateFunc)();

template<class T>
JZModule *createModule()
{
    return new T();
}

class JZModuleInfo
{
public:
    JZModuleInfo();

    QString name;
    QStringList depends;
    JZModuleCreateFunc createFunc;
};

class JZModule
{
public:
    JZModule();
    virtual ~JZModule();

    const JZModuleInfo &info() const;

    void addRef();
    void release();
    int refCount();

    virtual void regist() = 0;
    virtual void unregist() = 0;

protected:
    JZModuleInfo m_info;
    int m_refCount;
};

class JZModuleManager
{
public:
    static JZModuleManager *instance();

    void registModule(JZModuleInfo info);

    QStringList moduleList();
    JZModuleInfo *moduleInfo(QString name);
    JZModule *module(QString name);

    bool loadModule(QString name);
    void unloadModule(QString name);

protected:
    JZModuleManager();
    ~JZModuleManager();

    QList<JZModuleInfo> m_moduleInfoList;
    QList<JZModule*> m_moduleList;
};


#endif