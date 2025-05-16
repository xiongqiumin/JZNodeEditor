#ifndef JZ_MODULE_MOTION_H_
#define JZ_MODULE_MOTION_H_

#include "JZModule.h"
#include "../JZModuleDefine.h"
#include "JZMotionNode.h"

class JZModuleMotion: public JZModule
{
public:
    JZModuleMotion();
    virtual ~JZModuleMotion();

    virtual void regist(JZScriptEnvironment *env) override;
    virtual void unregist(JZScriptEnvironment *env) override;        
};

#endif