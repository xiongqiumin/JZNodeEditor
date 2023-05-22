#include "JZNodeDebugPacket.h"
#include <QDataStream>

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
