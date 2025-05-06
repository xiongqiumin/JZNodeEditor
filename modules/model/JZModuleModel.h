#ifndef JZ_MODULE_MODEL_H_
#define JZ_MODULE_MODEL_H_

#include "JZModule.h"
#include "../JZModuleDefine.h"
#include "JZModelNode.h"

class JZModuleModel: public JZModule
{
public:
    JZModuleModel();
    virtual ~JZModuleModel();

    virtual void regist(JZScriptEnvironment *env) override;
    virtual void unregist(JZScriptEnvironment *env) override;        
};

#endif