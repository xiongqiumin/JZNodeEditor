#ifndef JZ_MODULE_CAMERA_H_
#define JZ_MODULE_CAMERA_H_

#include "JZNodeFunction.h"
#include "JZModule.h"
#include "../JZModuleDefine.h"
#include "JZNodeEvent.h"

enum
{
    CameraModule_id = Module_CameraType,
};

enum CameraNode
{
    Node_CameraId = Module_CameraNode,
    Node_CameraInit,
    Node_CameraFrameReady,
};

class JZCameraInitNode : public JZNode
{
public:
    JZCameraInitNode();
    ~JZCameraInitNode();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error);
};

class JZCameraFrameReadyEvent : public JZNodeSignalEvent
{
public:
    JZCameraFrameReadyEvent();
    ~JZCameraFrameReadyEvent();    

    virtual bool JZCameraFrameReadyEvent::compiler(JZNodeCompiler* c, QString& error) override;

    virtual void saveToStream(QDataStream &s) const override;
    virtual void loadFromStream(QDataStream &s) override;
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