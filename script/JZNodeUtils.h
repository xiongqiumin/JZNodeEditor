#ifndef JZNODE_UTILS_H_
#define JZNODE_UTILS_H_

#include <QString>
#include <QByteArray>
#include <QDataStream>

struct MemberInfo
{
    QString className;
    QString name;
};

struct FunctionInfo
{
    QString className;
    QString name;
};


struct LinkInfo {
    QString name;    
    QString text;
    QVariantMap params;
};

class JZNodeUtils
{
public:    
    template<class T>
    static QByteArray toBuffer(const T &data)
    {
        QByteArray buffer;
        QDataStream s(&buffer, QIODevice::WriteOnly);
        s << data;
        return buffer;
    }

    template<class T>
    static T fromBuffer(const QByteArray &buffer)
    {    
        QDataStream s(buffer);
        T data;
        s >> data;
        return data;
    }

    static MemberInfo splitMember(QString name);
    static QString makeLink(QString tips, QString path, QString args);
    static LinkInfo parseLink(QString line);
};

#endif // !JZNODE_UTILS_H_