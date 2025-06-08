#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include "JZCameraFile.h"

using namespace cv;

JZCameraFileConfig::JZCameraFileConfig()
{
    type = Camera_File;
    delay = 500;
}

void JZCameraFileConfig::saveToStream(QDataStream& s) const
{
    JZCameraConfig::saveToStream(s);
    s << path;
    s << delay;
}

void JZCameraFileConfig::loadFromStream(QDataStream& s)
{
    JZCameraConfig::loadFromStream(s);
    s >> path;
    s >> delay;
}

//JZCameraFile
JZCameraFile::JZCameraFile(QObject *parent)
    :JZCamera(parent)
{
    m_timer = new QTimer();
    m_fileIndex = 0;
    connect(m_timer,&QTimer::timeout,this,&JZCameraFile::onReadTimer);
}

JZCameraFile::~JZCameraFile()
{
}

JZCameraType JZCameraFile::type()
{
    return Camera_File;
}

bool JZCameraFile::isOpen()
{
    return m_fileList.size() != 0;
}

bool JZCameraFile::setConfig(JZCameraConfigEnum config)
{
    m_config = config;
    return true;
}

bool JZCameraFile::open()
{
    auto config = dynamic_cast<JZCameraFileConfig*>(m_config.data());

    QDir dir(config->path);
    if(!dir.exists())
        return false;
    
    QStringList filters;
    filters << "*.bmp" << "*.jpg" << "*.jpeg" << "*.png";
    
    auto list = dir.entryInfoList(filters, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot,QDir::Name);
    if(list.size() == 0)
        return false;

    m_fileList.clear();
    for(int i = 0; i < list.size(); i++)
        m_fileList.push_back(list[i].filePath());

    m_fileIndex = 0;
    return true;
}

void JZCameraFile::close()
{
    stop();
    m_fileList.clear();
}

void JZCameraFile::start()
{
    if (m_fileList.size() == 0)
        return;

    auto config = dynamic_cast<JZCameraFileConfig*>(m_config.data());
    m_timer->start(config->delay);
}

void JZCameraFile::startOnce()
{
    m_timer->stop();
    cv::Mat mat = readFrame();
    emit sigFrameReady(mat);
}

void JZCameraFile::stop()
{
    m_timer->stop();
}

cv::Mat JZCameraFile::readFrame()
{
    QString path = m_fileList[m_fileIndex];    
    m_fileIndex = (m_fileIndex + 1)%m_fileList.size();
    cv::Mat mat = imread(path.toLocal8Bit().data());
    return mat;
}

void JZCameraFile::onReadTimer()
{
    cv::Mat mat = readFrame();
    emit sigFrameReady(mat);
}