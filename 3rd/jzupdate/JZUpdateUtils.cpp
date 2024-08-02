#include <QDir>
#include <QCryptographicHash>
#include "JZUpdateUtils.h"

JZUpdateUtils::JZUpdateUtils()
{
    m_excludeList << "update" << "Qt5Core.dll"
        << "Qt5Designer.dll"
        << "Qt5DesignerComponents.dll"
        << "Qt5Gui.dll"
        << "Qt5Network.dll"
        << "Qt5Test.dll"
        << "Qt5Widgets.dll"
        << "Qt5Xml.dll";
}

QMap<QString, QString> JZUpdateUtils::getMd5(QString path)
{    
    m_path = path;
    QMap<QString, QString> info;    
    walkDirMd5(m_path, info);
    return info;
}

QStringList JZUpdateUtils::getFiles(QString path)
{
    m_path = path;
    QStringList info;
    walkDir(m_path, info);
    info.sort();
    return info;
}

bool JZUpdateUtils::isExclude(QString name)
{
    return m_excludeList.contains(name) || name.endsWith(".bak");
}

void JZUpdateUtils::walkDir(QString path, QStringList &info)
{
    QDir dir(path);
    auto info_list = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
    for (int i = 0; i < info_list.size(); i++)
    {        
        if (isExclude(info_list[i].fileName()))
            continue;

        QString dst_path = info_list[i].filePath();
        if (info_list[i].isDir())
        {            
            walkDir(dst_path, info);
        }
        Q_ASSERT(dst_path.startsWith(m_path));

        QString relate_path = dst_path.mid(m_path.size() + 1);
        info << relate_path;
    }
}

void JZUpdateUtils::walkDirMd5(QString path, QMap<QString, QString> &info)
{
    QDir dir(path);
    auto info_list = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
    for (int i = 0; i < info_list.size(); i++)
    {
        if (isExclude(info_list[i].fileName()))
            continue;

        QString md5;
        QString dst_path = info_list[i].filePath();
        if (info_list[i].isFile())
        {            
            QFile file(dst_path);
            if (file.open(QFile::ReadOnly))
                md5 = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex();
        }
        else
        {
            md5 = "dir";
            walkDirMd5(dst_path, info);
        }
        Q_ASSERT(dst_path.startsWith(m_path));

        QString relate_path = dst_path.mid(m_path.size() + 1);
        info[relate_path] = md5;
    }
}