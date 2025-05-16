#ifndef JZ_CAMERA_UVC_H_
#define JZ_CAMERA_UVC_H_

#include "JZCamera.h"

//JZCameraUVC
class JZCameraUvcConfig
{
public:
    QString path;
};
QDataStream& operator<<(QDataStream& s, const JZCameraUvcConfig& param);
QDataStream& operator>>(QDataStream& s, JZCameraUvcConfig& param);

//JZCameraUVC
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

    virtual void start() override;
    virtual void startOnce() override;
    virtual void stop() override;

protected:

};

#endif