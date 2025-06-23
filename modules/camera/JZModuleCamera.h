#ifndef JZ_MODULE_CAMERA_H_
#define JZ_MODULE_CAMERA_H_

#include "../JZModuleDefine.h"
#include "JZNodeFunction.h"
#include "JZModule.h"
#include "JZCameraNode.h"
#include "JZCameraManager.h"

class JZModuleCamera: public JZModule
{            
public:
    JZModuleCamera();
    virtual ~JZModuleCamera();
    
    virtual void regist(JZScriptEnvironment *env) override;
    virtual void unregist(JZScriptEnvironment *env) override;

    std::function<JZCameraManagerConfig()> cameraConfig();

protected:

};

#endif