#ifndef JZ_UPDATE_PACK_H
#define JZ_UPDATE_PACK_H

#include "JZNetPack.h"

enum{    
    NetPack_fileMapRes = 0x200,
    NetPack_fileMapReq,
    NetPack_fileRes,
    NetPack_fileReq,
};


class JZUpdateFileMapReq : public JZNetPack
{
public:
    JZUpdateFileMapReq();
    ~JZUpdateFileMapReq();

    virtual int type() const;
    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);
};

class JZUpdateFileMapRes : public JZNetPack
{
public:
    JZUpdateFileMapRes();
    ~JZUpdateFileMapRes();
    
    virtual int type() const;
    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);

    QMap<QString, QString> fileMap;
};

class JZUpdateFileReq : public JZNetPack
{
public:
    JZUpdateFileReq();
    ~JZUpdateFileReq();

    virtual int type() const;
    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);

    QString file;
    int offset;
};
class JZUpdateFileRes : public JZNetPack
{
public:
    JZUpdateFileRes();
    ~JZUpdateFileRes();

    virtual int type() const;
    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);
    
    bool isEnd;   
    QByteArray buffer;
};

void JZUpdatePackRegist();

#endif // UPDATE_H
