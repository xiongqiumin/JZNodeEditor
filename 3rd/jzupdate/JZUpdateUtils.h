#ifndef JZ_UPDATE_UTILS_H
#define JZ_UPDATE_UTILS_H

#include <QMap>

class JZUpdateUtils
{	
public:
    JZUpdateUtils();

    QMap<QString, QString> getMd5(QString dir);
    QStringList getFiles(QString dir);

protected:
    bool isExclude(QString name);
    void walkDirMd5(QString dir, QMap<QString, QString> &info);
    void walkDir(QString dir, QStringList &info);

    QString m_path;
    QStringList m_excludeList;
};


#endif // UPDATE_H
