#ifndef JZ_CAMERA_NODE_H_
#define JZ_CAMERA_NODE_H_

#include "JZNodeEvent.h"
#include "../JZModuleDefine.h"
#include "JZCameraManager.h"

enum CameraNode
{
    Node_CameraId = Module_CameraNode,
    Node_CameraInit,
    Node_CameraFrameReady,
};

class JZNodeCameraInit : public JZNode
{
public:
    JZNodeCameraInit();
    ~JZNodeCameraInit();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error);

    void setConfig(const JZCameraManagerConfig &config);
    JZCameraManagerConfig config();

protected:
    JZCameraManagerConfig m_config;
};

class JZNodeCameraReadyEvent : public JZNodeSignalEvent
{
public:
    JZNodeCameraReadyEvent();
    ~JZNodeCameraReadyEvent();    

    virtual bool JZNodeCameraReadyEvent::compiler(JZNodeCompiler* c, QString& error) override;

    virtual void saveToStream(QDataStream &s) const override;
    virtual void loadFromStream(QDataStream &s) override;

    QString m_camera;
};

#endif