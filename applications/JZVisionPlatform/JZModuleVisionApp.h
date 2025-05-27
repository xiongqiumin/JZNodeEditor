#ifndef JZ_MODEL_VISION_APP_H_
#define JZ_MODEL_VISION_APP_H_

#include "JZModule.h"

class JZModuleVisionApp : public JZModule
{
public:
    JZModuleVisionApp();
    virtual ~JZModuleVisionApp();

    virtual void regist(JZScriptEnvironment* env) override;
    virtual void unregist(JZScriptEnvironment* env) override;
};

#endif // !JZ_MODEL_VISION_APP_H_