#include "JZUpdateServer.h"
#include "JZUpdateUtils.h"

JZUpdateServer::JZUpdateServer()
{
    connect(&m_server, &JZNetServer::sigNewConnect, this, &JZUpdateServer::onConnect);
    connect(&m_server, &JZNetServer::sigDisConnect, this, &JZUpdateServer::onDisconnect);
    connect(&m_server, &JZNetServer::sigNetPackRecv, this, &JZUpdateServer::onPacketRecv);
}

JZUpdateServer::~JZUpdateServer()
{
    m_server.stopServer();
}

void JZUpdateServer::init(QString path, int port)
{
    JZUpdateUtils util;
    m_fileMap = util.getMd5(path);
    m_path = path;
    m_server.startServer(port);
}

void JZUpdateServer::onConnect(int id)
{
    ClientInfo info;
    m_client[id] = info;
}

void JZUpdateServer::onDisconnect(int id)
{
    m_client.remove(id);
}

void JZUpdateServer::onPacketRecv(int id, JZNetPackPtr ptr)
{
    int cmd = ptr->type();
    if (cmd == NetPack_fileMapReq)
    {
        JZUpdateFileMapRes res;
        res.setSeq(ptr->seq());
        res.fileMap = m_fileMap;
        m_server.sendPack(id,&res);
    }
    else if (cmd == NetPack_fileReq)
    {
        auto pack_fileReq = dynamic_cast<JZUpdateFileReq*>(ptr.data());
        auto &info = m_client[id];
        if (info.file && info.file->fileName() != pack_fileReq->file)
        {
            info.file->close();
            info.file.reset();
            info.offset = 0;
        }

        if (!info.file)
        {
            info.file = QSharedPointer<QFile>(new QFile());
            info.file->setFileName(m_path + "/" + pack_fileReq->file);
            info.file->open(QFile::ReadOnly);
            info.offset = 0;
        }

        if (info.offset != pack_fileReq->offset)
        {
            info.file->seek(pack_fileReq->offset);
            info.offset = pack_fileReq->offset;
        }

        int max_size = 100 * 1024;

        JZUpdateFileRes res;     
        res.setSeq(ptr->seq());
        res.buffer = info.file->read(max_size);        
        info.offset += res.buffer.size();
        if (info.file->atEnd())
        {
            info.file->close();
            info.file.reset();
            res.isEnd = true;
        }
        m_server.sendPack(id, &res);
    }
}