#ifndef JZ_REMOTE_PACKET_H_
#define JZ_REMOTE_PACKET_H_

#include "3rd/JZCommon/jzNet/JZNetPack.h"
#include "JZProject.h"
#include "JZNodeProgram.h"
#include "JZNodeEngine.h"

enum {
    Remote_StartReq = NetPack_user + 100,
    Remote_StartRes,
    Remote_StopReq,
    Remote_StopRes,
};

//JZRemoteStartPacket
class JZRemoteStartPacketReq : public JZNetPack
{
public:
    JZRemoteStartPacketReq();
    ~JZRemoteStartPacketReq();

    virtual int type() const;
    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);

    JZNodeProgram program;
};

//JZRemoteStartPacketRes
class JZRemoteStartPacketRes : public JZNetPack
{
public:    
    virtual int type() const;
    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);

    int code;
};

//JZRemoteStopPacketReq
class JZRemoteStopPacketReq : public JZNetPack
{
public:
    JZRemoteStopPacketReq();
    ~JZRemoteStopPacketReq();

    virtual int type() const;
    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);    
};

//JZRemoteStopPacketRes
class JZRemoteStopPacketRes : public JZNetPack
{
public:    
    virtual int type() const;
    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);

    int code;
};

void JZRemotePacketInit();

#endif
