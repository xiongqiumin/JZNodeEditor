#include "JZNodeDebugPacket.h"

JZNodeDebugPacket::JZNodeDebugPacket()
{
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
    
}

void JZNodeDebugPacket::loadFromStream(QDataStream &s)
{

}

