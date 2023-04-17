#ifndef JZNODE_DEBUG_PACKET_H_
#define JZNODE_DEBUG_PACKET_H_


#include <QDataStream>

class JZNodeDebugPacket
{
public:
    JZNodeDebugPacket();
    ~JZNodeDebugPacket();

protected:
    friend QDataStream &operator<<(QDataStream &s, const JZNodeDebugPacket &param);
    friend QDataStream &operator>>(QDataStream &s, JZNodeDebugPacket &param);    
};
QDataStream &operator<<(QDataStream &s, const JZNodeDebugPacket &param);
QDataStream &operator>>(QDataStream &s, JZNodeDebugPacket &param);

#endif