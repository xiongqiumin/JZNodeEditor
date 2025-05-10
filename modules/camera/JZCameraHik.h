#ifndef JZ_CAMERA_HIK_H_
#define JZ_CAMERA_HIK_H_

#include "JZCamera.h"

class JZCamerHikConfig
{
public:
    enum {        
        TRIGGER_SOURCE_LINE0 = 0,
        TRIGGER_SOURCE_LINE1 = 1,
        TRIGGER_SOURCE_LINE2 = 2,
        TRIGGER_SOURCE_LINE3 = 3,
        TRIGGER_SOURCE_COUNTER0 = 4,

        TRIGGER_SOURCE_SOFTWARE = 7,
        TRIGGER_SOURCE_FrequencyConverter = 8,
    };

    enum {
        TRIGGER_MODE_OFF,
        TRIGGER_MODE_ON,
    };

    enum {
        GAIN_MODE_OFF = 0, 
        GAIN_MODE_ONCE = 1,  
        GAIN_MODE_CONTINUOUS,
    };

    enum {
        EXPOSURE_AUTO_MODE_OFF,
        EXPOSURE_AUTO_MODE_ONCE,
        EXPOSURE_AUTO_MODE_CONTINUOUS,
    };

    JZCamerHikConfig();

    QString path;
    int triggerSource;
    int triggerMode;

    int gainMode;
    double gain;

    int exposureMode;    
    double exposureTime;
};
QDataStream &operator<<(QDataStream &s, const JZCamerHikConfig &param);
QDataStream &operator>>(QDataStream &s, JZCamerHikConfig &param);

class JZCameraHik : public JZCamera
{
public:
    JZCameraHik();
    ~JZCameraHik();

    virtual JZCameraType type() override;
    virtual bool isOpen() override;
    virtual bool open(QString path) override;
    virtual void close() override;

    virtual void start() override;
    virtual void startOnce() override;
    virtual void stop() override;

    void setConfig(JZCamerHikConfig config);

protected:    
    bool CommandExecute(QString command);
    void GrabbingThread();
    void startGrabbing();
        
    QString errorString(int code);

    bool m_isStartGrabbing;
    void *m_hDevHandle;
    QThread *m_thread;
};



#endif // !JZ_CAMERA_HIK_H_
