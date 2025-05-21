#include "JZRemotePacket.h"
#include <QDataStream>
#include "JZNodeFunctionManager.h"

//JZRemoteStartPacketReqReq
JZRemoteStartPacketReq::JZRemoteStartPacketReq()
{
}

JZRemoteStartPacketReq::~JZRemoteStartPacketReq()
{
}

int JZRemoteStartPacketReq::type() const
{
    return Remote_StartReq;
}

void JZRemoteStartPacketReq::saveToStream(QDataStream &s) const
{
    program.saveToStream(s);
}

void JZRemoteStartPacketReq::loadFromStream(QDataStream &s)
{
    program.loadFromStream(s);
}

//JZRemoteStartPacketRes
int JZRemoteStartPacketRes::type() const
{
    return Remote_StopRes;
}

void JZRemoteStartPacketRes::saveToStream(QDataStream &s) const
{
    s << code;
}

void JZRemoteStartPacketRes::loadFromStream(QDataStream &s)
{
    s >> code;
}

//JZRemoteStopPacketReq
JZRemoteStopPacketReq::JZRemoteStopPacketReq()
{
}

JZRemoteStopPacketReq::~JZRemoteStopPacketReq()
{
}

int JZRemoteStopPacketReq::type() const
{
    return Remote_StopReq;
}

void JZRemoteStopPacketReq::saveToStream(QDataStream &s) const
{
}

void JZRemoteStopPacketReq::loadFromStream(QDataStream &s)
{
}

//JZRemoteStopPacketRes
int JZRemoteStopPacketRes::type() const
{
    return Remote_StopRes;
}

void JZRemoteStopPacketRes::saveToStream(QDataStream &s) const
{
    s << code;
}

void JZRemoteStopPacketRes::loadFromStream(QDataStream &s)
{
    s >> code;
}

void JZRemotePacketInit()
{
    JZNetPackManager::instance()->registPack(Remote_StartReq, JZNetPackCreate<JZRemoteStartPacketReq>);
    JZNetPackManager::instance()->registPack(Remote_StartRes, JZNetPackCreate<JZRemoteStartPacketRes>);
    JZNetPackManager::instance()->registPack(Remote_StopReq, JZNetPackCreate<JZRemoteStopPacketReq>);
    JZNetPackManager::instance()->registPack(Remote_StopRes, JZNetPackCreate<JZRemoteStopPacketRes>);
}