#ifndef JZ_CAMERA_RTSP_H_
#define JZ_CAMERA_RTSP_H_

#include <QThread>
#include "JZCamera.h"

class JZCameraRtspConfig : public JZCameraConfig
{
public:
    JZCameraRtspConfig();
    QString path;

    virtual void saveToStream(QDataStream& s) const override;
    virtual void loadFromStream(QDataStream& s) override;
};

class RtspThread : public QThread
{
    Q_OBJECT

public: 
    JZCameraRtspConfig config;

signals:
    void sigFrameReady(cv::Mat mat);

protected:
    virtual void run() override;
};

class JZCameraRtsp : public JZCamera
{
    Q_OBJECT

public:
    JZCameraRtsp(QObject *parent = nullptr);
    ~JZCameraRtsp();

    virtual JZCameraType type() override;

    virtual bool setConfig(JZCameraConfigEnum config) override;
    virtual bool isOpen() override;
    virtual bool open() override;
    virtual void close() override;

    virtual void start() override;
    virtual void startOnce() override;
    virtual void stop() override;

    void stopAndDelete();

protected slots:
    void onReadTimer();
    void onFrameReady(cv::Mat mat);
    void onThreadFinish();

protected:
    RtspThread m_rtspThread;
    bool readFrame(cv::Mat &mat);
    
    bool m_delete;
    QTimer *m_timer;    
    cv::Mat m_frame;
};

#endif