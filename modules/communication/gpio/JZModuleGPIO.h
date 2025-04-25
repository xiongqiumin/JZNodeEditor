#ifndef JZ_MODULE_MODBUS_H_
#define JZ_MODULE_MODBUS_H_

#include "JZNodeFunction.h"
#include "JZModule.h"
#include "jzmodbus/JZModbusParam.h"

class JZModuleGPIO: public JZModule
{        
    
public:
    JZModuleGPIO();
    virtual ~JZModuleGPIO();
    
    virtual void regist(JZScriptEnvironment *env) override;
    virtual void unregist(JZScriptEnvironment *env) override;
};

#endif