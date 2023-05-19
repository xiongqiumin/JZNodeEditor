#ifndef JZNODE_DEBUG_PACKET_H_
#define JZNODE_DEBUG_PACKET_H_

#include "JZNetPack.h"

const int NetPack_debugPacket = NetPack_user;

enum{
    Cmd_none,
    Cmd_addBreakPoint,
    Cmd_removeBreakPoint,       
    Cmd_clearBreakPoint,
    Cmd_heart,
    Cmd_pause,
    Cmd_resume,    
    Cmd_stepIn,
    Cmd_stepOver,
    Cmd_stepOut, 
    Cmd_breakTrigger,
};


class JZNodeDebugPacket : public JZNetPack 
{
public:
    JZNodeDebugPacket();
    ~JZNodeDebugPacket();

    virtual int type() const;
	virtual void saveToStream(QDataStream &s) const;
	virtual void loadFromStream(QDataStream &s);    

    int cmd;
    QVariantMap params;
};

#endif