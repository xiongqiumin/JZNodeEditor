#include <inttypes.h>
#include "JZMotion.h"
#include "modules/opencv/CvToQt.h"

//JZMotionConfigEnum
QDataStream& operator<<(QDataStream& s, const JZMotionConfigEnum& config)
{
    JZModuleConfigFactory<JZMotionConfig>::instance()->saveToStream(s, config);
    return s;
}

QDataStream& operator >> (QDataStream& s, JZMotionConfigEnum& config)
{
    JZModuleConfigFactory<JZMotionConfig>::instance()->loadFromStream(s, config);
    return s;
}

//JZMotionConfig
JZMotionConfig::JZMotionConfig()
{
    type = Motion_None;
}

void JZMotionConfig::saveToStream(QDataStream& s) const
{
    s << name;
    s << type;
}

void JZMotionConfig::loadFromStream(QDataStream& s)
{
    s >> name;
    s >> type;
}

//JZMotion
JZMotion::JZMotion(QObject *parent)
    :QObject(parent)
{
}

JZMotion::~JZMotion()
{
}

QString JZMotion::name() const
{
    return m_config->name;
}

const JZMotionConfigEnum &JZMotion::config()
{
    return m_config;
}

void JZMotion::setConfig(JZMotionConfigEnum config)
{
    m_config = config;
}
