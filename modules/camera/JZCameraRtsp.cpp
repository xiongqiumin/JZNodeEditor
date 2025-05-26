#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include "JZCameraRtsp.h"
#include "jzProfiler/JZTx.h"

using namespace cv;

JZCameraRtspConfig::JZCameraRtspConfig()
{
    type = Camera_Rtsp;
}

void JZCameraRtspConfig::saveToStream(QDataStream& s) const
{
    JZCameraConfig::saveToStream(s);
    s << path;
}

void JZCameraRtspConfig::loadFromStream(QDataStream& s)
{
    JZCameraConfig::loadFromStream(s);
    s >> path;
}

//RtspThread
void RtspThread::run()
{
    cv::VideoCapture capture;
        
    int timeout = 5;
    capture.set(cv::CAP_PROP_OPEN_TIMEOUT_MSEC, int(timeout * 1000));
    capture.set(cv::CAP_PROP_READ_TIMEOUT_MSEC, int(timeout * 1000));
    if (!capture.open(qPrintable(config.path),cv::CAP_FFMPEG))
        return;

    while(!isInterruptionRequested())
    {
        Mat mat;
        if (!capture.read(mat))
            break;

        sigFrameReady(mat);
        msleep(10);
    }
}

//JZCameraRtsp
JZCameraRtsp::JZCameraRtsp(QObject *parent)
    :JZCamera(parent)
{
    m_delete = false;
    m_timer = new QTimer(this);        

    connect(m_timer, &QTimer::timeout, this, &JZCameraRtsp::onReadTimer);
    connect(&m_rtspThread, &RtspThread::finished, this, &JZCameraRtsp::onThreadFinish);
    connect(&m_rtspThread, &RtspThread::sigFrameReady, this, &JZCameraRtsp::onFrameReady);
}

JZCameraRtsp::~JZCameraRtsp()
{    
    close();
    if (m_rtspThread.isRunning())        
        m_rtspThread.wait();    
}

JZCameraType JZCameraRtsp::type()
{
    return Camera_Rtsp;
}

bool JZCameraRtsp::isOpen()
{
    return m_rtspThread.isRunning();
}

bool JZCameraRtsp::setConfig(JZCameraConfigPtr config)
{
    m_config = config;
    return true;
}

bool JZCameraRtsp::open()
{
    if (m_rtspThread.isRunning())
        return false;

    auto config = dynamic_cast<JZCameraRtspConfig*>(m_config.data());
    m_rtspThread.config = *config;
    m_rtspThread.start();
    return true;
}

void JZCameraRtsp::close()
{    
    if (m_rtspThread.isRunning())
        m_rtspThread.requestInterruption();
}

void JZCameraRtsp::start()
{    
    m_timer->start(500);
}

void JZCameraRtsp::startOnce()
{
    cv::Mat mat;
    if (!readFrame(mat))
        return;

    emit sigFrameReady(mat);
}

void JZCameraRtsp::stop()
{
    m_timer->stop();    
}

void JZCameraRtsp::stopAndDelete()
{
    if (m_rtspThread.isRunning())
    {
        m_rtspThread.disconnect();
        disconnect();
        close();
        m_delete = true;
    }
    else
    {
        deleteLater();
    }
}

bool JZCameraRtsp::readFrame(cv::Mat &mat)
{    
    if (m_frame.empty())
        return false;
    
    mat = m_frame;
    m_frame = Mat();
    return true;    
}

void JZCameraRtsp::onFrameReady(cv::Mat mat)
{
    m_frame = mat;
}

void JZCameraRtsp::onThreadFinish()
{
    if (m_delete)
        deleteLater();
}

void JZCameraRtsp::onReadTimer()
{
    JZTX_FUNCTION
    cv::Mat mat;
    if (!readFrame(mat))
        return;    

    emit sigFrameReady(mat);
}