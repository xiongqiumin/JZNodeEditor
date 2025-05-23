#include <QDataStream>
#include "JZCamera.h"

//JZCameraConfig
JZCameraConfig::JZCameraConfig()
{
    type = Camera_None;
    name = "camera";
}

void JZCameraConfig::saveToStream(QDataStream& s) const
{
    s << type << name;
}

void JZCameraConfig::loadFromStream(QDataStream& s)
{
    s >> type >> name;
}

//JZCamera
JZCamera::JZCamera(QObject *parent)
    :QObject(parent)
{
}

JZCamera::~JZCamera()
{
}