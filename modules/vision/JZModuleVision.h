#ifndef JZ_MODULE_VISION_H_
#define JZ_MODULE_VISION_H_

#include "JZModule.h"
#include "../JZModuleDefine.h"
#include "JZVisionNode.h"

class JZModuleVision: public JZModule
{
public:
    JZModuleVision();
    virtual ~JZModuleVision();

    virtual void regist(JZScriptEnvironment *env) override;
    virtual void unregist(JZScriptEnvironment *env) override;        

    JZNodeObjectWidgetFactory m_visionWindowFactory;
};

#endif