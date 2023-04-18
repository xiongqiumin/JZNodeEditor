#ifndef JZNODE_DEBUG_PACKET_H_
#define JZNODE_DEBUG_PACKET_H_

#include "JZNetPack.h"

const int NetPack_debugPacket = NetPack_user;

class JZNodeDebugPacket : public JZNetPack 
{
public:
    JZNodeDebugPacket();
    ~JZNodeDebugPacket();

    virtual int type() const;
	virtual void saveToStream(QDataStream &s) const;
	virtual void loadFromStream(QDataStream &s);    
};

#endif