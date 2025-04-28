#ifndef JZ_CAMERA_H_
#define JZ_CAMERA_H_

#include <QObject>
#include <opencv2/opencv.hpp>

enum JZCameraType
{
    Camera_None,
    Camera_File,
    Camera_Hik,
};

class JZCamera : public QObject
{
    Q_OBJECT

public:
    JZCamera(QObject *parent = nullptr);
    virtual ~JZCamera();

    virtual JZCameraType type() = 0;
    virtual bool isOpen() = 0;
    virtual bool open(QString path) = 0;
    virtual void close() = 0;

    virtual QString config() = 0;
    virtual bool setConfig(const QString &config) = 0;

    virtual void start() = 0;
    virtual void startOnce() = 0;
    virtual void stop() = 0;

signals:
    void sigFrameReady(cv::Mat mat);
    void sigError();

protected:

};

#endif