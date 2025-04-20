#ifndef JZ_MODULE_COMM_H_
#define JZ_MODULE_COMM_H_

#include "JZModule.h"
#include "JZCommNode.h"

class JZModuleComm: public JZModule
{        
    
public:
    JZModuleComm();
    virtual ~JZModuleComm();
    
    virtual void regist(JZScriptEnvironment *env) override;
    virtual void unregist(JZScriptEnvironment *env) override;
};

void JZModuleCommNodeInit();

#endif