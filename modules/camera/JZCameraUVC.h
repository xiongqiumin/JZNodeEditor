#ifndef JZ_CAMERA_UVC_H_
#define JZ_CAMERA_UVC_H_

#include "JZCamera.h"

class JZCameraUVC : public JZCamera
{
    Q_OBJECT

public:
    JZCameraUVC(QObject *parent = nullptr);
    ~JZCameraUVC();

    virtual JZCameraType type() override;
    virtual bool isOpen() override;
    virtual bool open(QString path) override;
    virtual void close() override;

    virtual QString config() override;
    virtual bool setConfig(const QString &config) override;

    virtual void start() override;
    virtual void startOnce() override;
    virtual void stop() override;

protected:

};

#endif