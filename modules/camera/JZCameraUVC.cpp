#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include "JZCameraUVC.h"

using namespace cv;

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

bool JZCameraUVC::open(QString path)
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