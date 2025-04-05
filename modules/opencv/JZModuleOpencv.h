#ifndef JZ_MODULE_MODBUS_H_
#define JZ_MODULE_MODBUS_H_

#include "JZModule.h"

class JZModuleOpencv: public JZModule
{
public:
    JZModuleOpencv();
    virtual ~JZModuleOpencv();

    virtual void regist(JZScriptEnvironment *env) override;
    virtual void unregist(JZScriptEnvironment *env) override;        
};


#endif