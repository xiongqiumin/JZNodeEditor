#include "JZNodeDebugPacket.h"
#include <QDataStream>

//JZNodeDebugPacket
JZNodeDebugPacket::JZNodeDebugPacket()
{
    cmd = Cmd_none;
}

JZNodeDebugPacket::~JZNodeDebugPacket()
{
}

int JZNodeDebugPacket::type() const
{
    return NetPack_debugPacket;
}

void JZNodeDebugPacket::saveToStream(QDataStream &s) const
{
    s << cmd << params;        
}

void JZNodeDebugPacket::loadFromStream(QDataStream &s)
{
    s >> cmd >> params;
}

//DebugInfo
JZNodeDebugInfo::JZNodeDebugInfo()
{
}

QDataStream &operator<<(QDataStream &s, const JZNodeDebugInfo &param)
{
    s << param.breakPoints;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeDebugInfo &param)
{
    s >> param.breakPoints;
    return s;
}
