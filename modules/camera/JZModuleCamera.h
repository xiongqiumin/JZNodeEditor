#ifndef JZ_MODULE_CAMERA_H_
#define JZ_MODULE_CAMERA_H_

#include "JZNodeFunction.h"
#include "JZModule.h"
#include "../JZModuleDefine.h"

enum
{
    CameraModule_id = Module_CameraType,
};

enum CameraNode
{
    Node_CameraId = Module_CameraNode,
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

#endif