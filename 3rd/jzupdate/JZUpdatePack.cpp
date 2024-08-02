#include "JZUpdatePack.h"

//JZUpdateFileMapReq
JZUpdateFileMapReq::JZUpdateFileMapReq()
{
}
JZUpdateFileMapReq::~JZUpdateFileMapReq()
{
}

int JZUpdateFileMapReq::type() const
{
    return NetPack_fileMapReq;
}

void JZUpdateFileMapReq::saveToStream(QDataStream &s) const
{
}
void JZUpdateFileMapReq::loadFromStream(QDataStream &s)
{
}

//JZUpdateFileMapRes
JZUpdateFileMapRes::JZUpdateFileMapRes()
{
}
JZUpdateFileMapRes::~JZUpdateFileMapRes()
{
}

int JZUpdateFileMapRes::type() const
{
    return NetPack_fileMapRes;
}

void JZUpdateFileMapRes::saveToStream(QDataStream &s) const
{
    s << fileMap;
}
void JZUpdateFileMapRes::loadFromStream(QDataStream &s)
{
    s >> fileMap;
}

//JZUpdateFileReq
JZUpdateFileReq::JZUpdateFileReq()
{
}
JZUpdateFileReq::~JZUpdateFileReq()
{
}

int JZUpdateFileReq::type() const
{
    return NetPack_fileReq;
}

void JZUpdateFileReq::saveToStream(QDataStream &s) const
{
    s << file << offset;
}
void JZUpdateFileReq::loadFromStream(QDataStream &s)
{
    s >> file >> offset;
}

//JZUpdateFileRes
JZUpdateFileRes::JZUpdateFileRes()
{
    isEnd = false;    
}
JZUpdateFileRes::~JZUpdateFileRes()
{
}

int JZUpdateFileRes::type() const
{
    return NetPack_fileRes;
}

void JZUpdateFileRes::saveToStream(QDataStream &s) const
{
    s << isEnd << buffer;
}
void JZUpdateFileRes::loadFromStream(QDataStream &s)
{
    s >> isEnd >> buffer;
}

void JZUpdatePackRegist()
{
    JZNetPackManager::instance()->registPack(NetPack_fileMapRes, JZNetPackCreate<JZUpdateFileMapRes>);
    JZNetPackManager::instance()->registPack(NetPack_fileMapReq, JZNetPackCreate<JZUpdateFileMapReq>);
    JZNetPackManager::instance()->registPack(NetPack_fileRes, JZNetPackCreate<JZUpdateFileRes>);
    JZNetPackManager::instance()->registPack(NetPack_fileReq, JZNetPackCreate<JZUpdateFileReq>);
}