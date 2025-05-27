#ifndef JZ_MODULE_LOG_H_
#define JZ_MODULE_LOG_H_

#include "JZModule.h"
#include "../JZModuleDefine.h"

class JZModuleLog : public JZModule
{
public:
    JZModuleLog();
    virtual ~JZModuleLog();

    virtual void regist(JZScriptEnvironment *env) override;
    virtual void unregist(JZScriptEnvironment *env) override;
};

#endif