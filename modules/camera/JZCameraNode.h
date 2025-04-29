#ifndef JZ_CAMERA_NODE_H_
#define JZ_CAMERA_NODE_H_

#include "JZNodeEvent.h"
#include "../JZModuleDefine.h"
#include "JZCameraManager.h"

enum CameraNode
{
    Node_CameraId = Module_CameraNode,
    Node_CameraInit,
    Node_CameraStart,
    Node_CameraStartOnce,
    Node_CameraStop,
    Node_CameraSetting,
    Node_CameraFrameReady,
};

class JZNodeCameraInit : public JZNode
{
public:
    JZNodeCameraInit();
    ~JZNodeCameraInit();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error);
    void saveToStream(QDataStream& s) const;
    void loadFromStream(QDataStream& s);

    void setConfig(const JZCameraManagerConfig &config);
    JZCameraManagerConfig config();

protected:
    JZCameraManagerConfig m_config;
};

//JZCameraNode
class JZCameraNode : public JZNode
{
public:
    JZCameraNode();
    ~JZCameraNode();

    virtual bool compiler(JZNodeCompiler* c, QString& error) override;

    virtual void saveToStream(QDataStream& s) const override;
    virtual void loadFromStream(QDataStream& s) override;

    void setCamera(QString name);
    QString camera();

protected:
    QString m_camera;
    QString m_name;
    QString m_function;
};

//JZNodeCameraStart
class JZNodeCameraStart : public JZCameraNode
{
public:
    JZNodeCameraStart();

protected:
};

//JZNodeCameraStartOnce
class JZNodeCameraStartOnce : public JZCameraNode
{
public:
    JZNodeCameraStartOnce();

protected:
};

//JZNodeCameraStop
class JZNodeCameraStop : public JZCameraNode
{
public:
    JZNodeCameraStop();

protected:
};

//JZNodeCameraSetting
class JZNodeCameraSetting : public JZCameraNode
{
public:
    JZNodeCameraSetting();

protected:
};

//JZNodeCameraReadyEvent
class JZNodeCameraReadyEvent : public JZNodeSignalEvent
{
public:
    JZNodeCameraReadyEvent();
    ~JZNodeCameraReadyEvent();    

    virtual bool compiler(JZNodeCompiler* c, QString& error) override;

    virtual void saveToStream(QDataStream &s) const override;
    virtual void loadFromStream(QDataStream &s) override;

    void setCamera(QString name);
    QString camera();

protected:
    QString m_camera;
};

#endif