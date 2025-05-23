#ifndef JZ_CAMERA_H_
#define JZ_CAMERA_H_

#include <QObject>
#include <opencv2/opencv.hpp>
#include <QSharedPointer>
#include <QDataStream>

enum JZCameraType
{
    Camera_None,
    Camera_File,
    Camera_UVC,
    Camera_Hik,
};

//JZCameraConfig
class JZCameraConfig
{
public:
    JZCameraConfig();

    int type;
    QString name;

    virtual void saveToStream(QDataStream& s) const;
    virtual void loadFromStream(QDataStream& s);
};
typedef QSharedPointer<JZCameraConfig> JZCameraConfigPtr;

class JZCamera : public QObject
{
    Q_OBJECT

public:
    JZCamera(QObject *parent = nullptr);
    virtual ~JZCamera();

    virtual JZCameraType type() = 0;

    virtual bool setConfig(JZCameraConfigPtr config) = 0;
    virtual bool isOpen() = 0;
    virtual bool open() = 0;
    virtual void close() = 0;

    virtual void start() = 0;
    virtual void startOnce() = 0;
    virtual void stop() = 0;

signals:
    void sigFrameReady(cv::Mat mat);
    void sigError();

protected:
    JZCameraConfigPtr m_config;
};

#endif