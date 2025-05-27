#ifndef JZ_CAMERA_UVC_H_
#define JZ_CAMERA_UVC_H_

#include "JZCamera.h"

//JZCameraUVC
class JZCameraUvcConfig : public JZCameraConfig
{
public:
    JZCameraUvcConfig();

    QString path;

    virtual void saveToStream(QDataStream& s) const override;
    virtual void loadFromStream(QDataStream& s) override;
};

//JZCameraUVC
class JZCameraUVC : public JZCamera
{
    Q_OBJECT

public:
    JZCameraUVC(QObject *parent = nullptr);
    ~JZCameraUVC();

    virtual JZCameraType type() override;

    virtual bool setConfig(JZCameraConfigEnum config) override;
    virtual bool isOpen() override;
    virtual bool open() override;
    virtual void close() override;

    virtual void start() override;
    virtual void startOnce() override;
    virtual void stop() override;

protected:

};

#endif