#ifndef JZNODE_DEBUG_PACKET_H_
#define JZNODE_DEBUG_PACKET_H_

#include "JZNetPack.h"

const int NetPack_debugPacket = NetPack_user;

enum{
    Cmd_none,
    Cmd_addBreakPoint,
    Cmd_removeBreakPoint,       
    Cmd_clearBreakPoint,    
    Cmd_detach,
    Cmd_pause,
    Cmd_resume,    
    Cmd_stop,
    Cmd_stepIn,
    Cmd_stepOver,
    Cmd_stepOut, 
    Cmd_breakTrigger,
    Cmd_runtimeInfo,
    Cmd_runtimeError,
    Cmd_getVariable,
    Cmd_setVariable,
    Cmd_log,
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
    QVariantList params;
};

template<class T>
QByteArray netDataPack(const T &param)
{
    QByteArray buffer;
    QDataStream s(&buffer,QIODevice::WriteOnly);
    s << param;
    return buffer;
}

template<class T>
T netDataUnPack(const QByteArray &buffer)
{
    T param;
    QDataStream s(buffer);
    s >> param;
    return param;
}

#endif
