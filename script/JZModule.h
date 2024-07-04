#ifndef JZ_MODULE_H_
#define JZ_MODULE_H_

#include "JZNodeObject.h"

class JZContext;
class JZModule
{
public:
    JZModule();
    ~JZModule();

    QString name();
    QStringList depends();

    void regist();
};

class JZModuleManager
{
public:
    static JZModuleManager *instance();

protected:
    JZModuleManager(/* args */);
    ~JZModuleManager();
};


#endif