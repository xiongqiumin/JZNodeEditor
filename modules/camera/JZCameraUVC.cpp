#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include "JZCameraUVC.h"

using namespace cv;

JZCameraUvcConfig::JZCameraUvcConfig()
{
    type = Camera_UVC;
}

void JZCameraUvcConfig::saveToStream(QDataStream& s) const
{
}


void JZCameraUvcConfig::loadFromStream(QDataStream& s)
{
}

//JZCameraUVC
JZCameraUVC::JZCameraUVC(QObject *parent)
    :JZCamera(parent)
{
}

JZCameraUVC::~JZCameraUVC()
{
}

JZCameraType JZCameraUVC::type()
{
    return Camera_UVC;
}

bool JZCameraUVC::isOpen()
{
    return false;
}

bool JZCameraUVC::setConfig(JZCameraConfigPtr config)
{
    m_config = config;
    return true;
}

bool JZCameraUVC::open()
{
    return false;
}

void JZCameraUVC::close()
{
}

void JZCameraUVC::start()
{
}

void JZCameraUVC::startOnce()
{
}

void JZCameraUVC::stop()
{
}