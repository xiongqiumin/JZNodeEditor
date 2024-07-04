#ifndef JZPROJECT_STREAM_H_
#define JZPROJECT_STREAM_H_

#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QSharedPointer>
#include <QDataStream>
#include <QFile>

//JZProjectFile
class JZProjectFile
{
public:
    JZProjectFile();
    ~JZProjectFile();

    bool openLoad(const QString &filename);
    bool openSave(const QString &filename);
    void close();

    QDataStream &stream();    
    QString error();

protected:
    QDataStream m_stream;
    QFile m_file;
    QString m_error;
};









#endif
