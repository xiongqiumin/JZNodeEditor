#ifndef JZ_CAMERA_HIK_H_
#define JZ_CAMERA_HIK_H_

#include "JZCamera.h"

class JZCameraHik : public JZCamera
{
public:
    JZCameraHik();
    ~JZCameraHik();

    virtual bool isOpen() override;
    virtual bool open(QString path) override;
    virtual void close() override;

    virtual QString config() override;
    virtual bool setConfig(const QString &config) override;

    virtual void start() override;
    virtual void startOnce() override;
    virtual void stop() override;

protected:    
    bool CommandExecute(QString command);
    void GrabbingThread();
    void startGrabbing();
    
    double GetExposureTime();               // ch:设置曝光时间 | en:Set Exposure Time
    bool SetExposureTime(double time);
    double GetGain();                       // ch:设置增益 | en:Set Gain
    bool SetGain(double gain);

    bool m_isStartGrabbing;
    void *m_hDevHandle;
    QThread *m_thread;
};



#endif // !JZ_CAMERA_HIK_H_
