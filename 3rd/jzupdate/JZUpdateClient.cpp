#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QThread>
#include <QProgressDialog>
#include <QApplication>
#include "JZUpdateClient.h"
#include "JZUpdateUtils.h"

//JZUpdateClient
JZUpdateClient::JZUpdateClient(QString path)
{
    m_client = new JZNetClient(this);
    m_path = path;
    m_download = false;
}

JZUpdateClient::~JZUpdateClient()
{
}

void JZUpdateClient::saveVersion()
{
    JZUpdateUtils util;
    m_localFile = util.getMd5(m_path);

    QFile file_log(versionFile());
    if (file_log.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream s(&file_log);
        auto it = m_localFile.begin();
        while (it != m_localFile.end())
        {
            s << it.key() + "," + it.value() + "\n";
            it++;
        }
    }
}

void JZUpdateClient::saveFinish()
{
    QFile file_log(finishFile());
    if (file_log.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream s(&file_log);
        s << "ok";
    }
}

bool JZUpdateClient::init(QString ip,int port)
{    
    QString version_path = versionFile();
    QFileInfo version_info(versionFile());
    if (version_info.exists())
    {
        QFile file_log(version_path);
        if (file_log.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream s_log(&file_log);
            QStringList list = s_log.readAll().split("\n", Qt::SkipEmptyParts);
            for (int i = 0; i < list.size(); i++)
            {
                auto md5 = list[i].split(",");
                m_localFile[md5[0]] = md5[1];
            }
            file_log.close();
        }
    }
    else
    {                
        QDir dir;
        if (!dir.exists(version_info.path()))
            dir.mkdir(version_info.path());
        
        saveVersion();        
    }
    return m_client->connectToHost(ip,port);
}

QString JZUpdateClient::versionFile()
{
    return m_path + "/update/version.txt";
}

QString JZUpdateClient::finishFile()
{
    return m_path + "/update/finish.txt";
}

QString JZUpdateClient::renameFile()
{
    return m_path + "/update/__renameLog__.txt";
}

QString JZUpdateClient::downloadPath()
{
    return m_path + "/update/download";
}

bool JZUpdateClient::checkUpdate(bool &isUpadte)
{
    JZUpdateFileMapReq req;    
    m_client->sendPack(&req);
    
    JZNetPackPtr ptr = m_client->waitPackBySeq(req.seq());
    if (!ptr)
        return false;

    auto *res = dynamic_cast<JZUpdateFileMapRes*>(ptr.data());
    m_remoteFile = res->fileMap;

    auto it = m_remoteFile.begin();
    while (it != m_remoteFile.end())
    {
        if (!m_localFile.contains(it.key()) || it.value() != m_localFile[it.key()])
        {
            isUpadte = true;
            return true;
        }

        it++;
    }

    isUpadte = true;
    return true;
}

bool JZUpdateClient::isDownloadFinish()
{
    return QFile::exists(finishFile());
}

bool JZUpdateClient::downloadUpdate()
{   
    m_download = true;

    QString down_path = downloadPath();
    QDir dir;
    if (!dir.exists(down_path))
        dir.mkdir(down_path);    
    
    if(QFile::exists(finishFile()))
        QFile::remove(finishFile());

    int num = 0;    
    auto it = m_remoteFile.begin();
    while (it != m_remoteFile.end())
    {                     
        if (!m_download)
            return false;

        QString local_down_path = down_path + "/" + it.key();
        if (it.value() == "dir")
        {
            if (!dir.exists(local_down_path))
                dir.mkdir(local_down_path);
        }
        else if (!m_localFile.contains(it.key()) || m_localFile[it.key()] != it.value())
        {            
            emit sigProgess(num * 100 / m_remoteFile.size(), "download: " + it.key());
            
            QFile file;
            file.setFileName(local_down_path);
            
            if (!file.open(QFile::WriteOnly | QFile::Truncate))
            {
                m_download = false;
                emit sigError("open error");
                return false;
            }

            bool isEnd = false;
            int offset = 0;
            while (!isEnd)
            {
                if (!m_download)
                    return false;

                JZUpdateFileReq req;
                req.file = it.key();
                req.offset = offset;

                m_client->sendPack(&req);
                JZNetPackPtr ptr = m_client->waitPackBySeq(req.seq());
                if (!ptr)
                {
                    m_download = false;
                    emit sigError("net error");
                    return false;
                }
                
                auto *res = dynamic_cast<JZUpdateFileRes*>(ptr.data());                
                file.write(res->buffer);
                offset += res->buffer.size();
                isEnd = res->isEnd;
            }
        }
        it++;
        num++;
    }
    if (!m_download)
        return false;

    saveFinish();
    return true;
}

bool JZUpdateClient::dealUpdate()
{        
    QString update_path = downloadPath();
    if (!QFile::exists(finishFile()))
        return false;

    QFile file_log(renameFile());
    if (!file_log.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return false;

    QTextStream s_log(&file_log);    
    JZUpdateUtils utils;
    QStringList fileList = utils.getFiles(update_path);
    for (int i = 0; i < fileList.size(); i++)
    {        
        QString src = update_path + "/" + fileList[i];
        QString dst = m_path + "/" + fileList[i];
        QFileInfo info(src);
        if (info.isDir())
        {            
            QDir dir;
            dir.mkpath(dst);
        }
        else
        {            
            QFile in(src);            
            if (!in.open(QIODevice::ReadOnly))
                return false;

            if (dst.endsWith(".exe") || dst.endsWith(".dll"))
            {
                QString dst_bak = dst + ".bak";
                if (QFile::exists(dst_bak) && !QFile::remove(dst_bak))
                    return false;

                if(!QFile::rename(dst, dst_bak))
                    return false;
            }
            else
            {
                QFile::remove(dst);
            }

            QFile out(dst);
            if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate))            
                return false;            

            while (!in.atEnd())
            {
                QByteArray buffer = in.read(1024 * 1024);
                out.write(buffer);
            }
            in.close();
            out.close();
        }
    }    
    file_log.close();

    QFile::remove(finishFile());    
    saveVersion();    

    QDir dir_down(downloadPath());
    dir_down.removeRecursively();    
    return true;
}

void JZUpdateClient::clearCache()
{
    if (!QFile::exists(renameFile()))
        return;    

    QThread::msleep(2000);
    QFile file_log(renameFile());
    if (file_log.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream s_log(&file_log);
        QStringList list = s_log.readAll().split("\n", Qt::SkipEmptyParts);
        for (int i = 0; i < list.size(); i++)
        {
            if (!QFile::remove(list[i]))
                return;
        }        
    }
    file_log.close();
    QFile::remove(renameFile());        
}

bool JZUpdateClient::isDownload()
{
    return m_download;
}

void JZUpdateClient::cancel()
{
    m_download = false;
}