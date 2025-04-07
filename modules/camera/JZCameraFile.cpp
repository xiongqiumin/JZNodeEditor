#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include "JZCameraFile.h"

JZCameraFile::JZCameraFile(QObject *parent)
    :JZCamera(parent)
{
    connect(m_timer,&QTimer::timeout,this,&JZCameraFile::onReadTimer);
}

JZCameraFile::~JZCameraFile()
{
}

bool JZCameraFile::open(QString path)
{
    QDir dir(path);
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

    return true;
}

void JZCameraFile::close()
{
}

void JZCameraFile::start()
{
    m_timer->start(100);
}

cv::Mat JZCameraFile::startOnce()
{
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