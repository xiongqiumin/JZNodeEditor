#ifndef JZ_CAMERA_H_
#define JZ_CAMERA_H_

#include <QObject>
#include <opencv2/opencv.hpp>

class JZCamera : public QObject
{
    Q_OBJECT

public:
    JZCamera(QObject *parent = nullptr);
    virtual ~JZCamera();

    virtual bool open(QString path) = 0;
    virtual void close() = 0;

    virtual void start() = 0;
    virtual void startOnce() = 0;
    virtual void stop() = 0;

signals:
    void sigFrameReady(cv::Mat mat);
    void sigError();

protected:

};

#endif