#ifndef JZ_MODULE_CAMERA_H_
#define JZ_MODULE_CAMERA_H_

#include "JZNodeFunction.h"
#include "JZModule.h"

enum
{
    CameraModule_id = 16000,
};

enum CameraNode
{
    Node_CameraId = 1000,
    Node_CameraInit,
};

class JZCameraInitNode : public JZNode
{
public:
    JZCameraInitNode();
    ~JZCameraInitNode();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error);
};

class JZModuleCamera: public JZModule
{            
public:
    JZModuleCamera();
    virtual ~JZModuleCamera();
    
    virtual void regist(JZScriptEnvironment *env) override;
    virtual void unregist(JZScriptEnvironment *env) override;

protected:

};

void JZModuleCameraNodeInit();

#endif