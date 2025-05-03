#ifndef DATA_BASE_H_
#define DATA_BASE_H_

#include <QtSql/QSqlDatabase>
#include <QDataStream>
#include "JZNodeUtils.h"

class DataBaseConfig
{
public:
    DataBaseConfig();
    ~DataBaseConfig();

    void init(QString path);

    void addConfig(QString name);
    bool hasConfig(const QString& key);
    void setConfigBuffer(QString key,const QByteArray &buffer);
    QByteArray getConfigBuffer(QString key);

    template<class T>
    T getConfig(QString key)
    {
        QByteArray buffer = getConfigBuffer(key);
        if (buffer.isEmpty())
            return T();

        int magic = 0;
        QDataStream s(&buffer, QIODevice::ReadOnly);
        s >> magic;
        if (magic != sizeof(T))
            return T();

        T t;
        s >> t;
        return t;
    }

    template<class T>
    void setConfig(QString key,const T &t)
    {
        QByteArray buffer;
        QDataStream s(&buffer, QIODevice::WriteOnly);        
        
        int magic = sizeof(t);
        s << magic;
        s << t;

        setConfigBuffer(key, buffer);        
    }

protected:
    QSqlDatabase m_db;
};



#endif // !DATA_BASE_H_
