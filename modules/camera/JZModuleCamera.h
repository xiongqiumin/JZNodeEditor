#ifndef JZ_MODULE_MODBUS_H_
#define JZ_MODULE_MODBUS_H_

#include "JZNodeFunction.h"
#include "JZModule.h"

class JZModuleCamera: public JZModule
{            
public:
    JZModuleCamera();
    virtual ~JZModuleCamera();
    
    virtual void regist(JZScriptEnvironment *env) override;
    virtual void unregist(JZScriptEnvironment *env) override;

protected:

};

#endif