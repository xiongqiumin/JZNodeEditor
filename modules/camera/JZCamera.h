#ifndef JZ_CAMERA_H_
#define JZ_CAMERA_H_

#include <QObject>
#include <opencv/opecv2.hpp>

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
    void sigTrigger(cv::Mat mat);
    void sigError();

protected:

};

#endif