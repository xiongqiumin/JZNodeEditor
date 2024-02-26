#ifndef JZPROJECT_STREAM_H_
#define JZPROJECT_STREAM_H_

#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QSharedPointer>

class JZProjectFile;
class JZProjectStream
{
public:
    JZProjectStream();
    ~JZProjectStream();

    void setReadOnly(bool flag);
    bool readOnly();

    QJsonValue value();
    void setValue(const QJsonValue &value);
    JZProjectStream &operator[](const QString &name);

protected:   
    int indexOfStream(const QString &name);

    bool m_readOnly;
    QString m_name;    
    QJsonValue m_value;
    QList<QSharedPointer<JZProjectStream>> m_childs;
};

void operator<<(JZProjectStream& stream, int value);
void operator<<(JZProjectStream& stream, double value);
void operator<<(JZProjectStream& stream, const QString &value);
void operator>>(JZProjectStream& stream, int &value);
void operator>>(JZProjectStream& stream, double &value);
void operator>>(JZProjectStream& stream, QString &value);

template<class T>
void operator<<(JZProjectStream& stream, const QList<T> &list)
{
    QJsonArray array;
    for (int i = 0; i < list.size(); i++)
    {
        JZProjectStream s;
        s << list[i];        
        array.push_back(s.value());
    }
    stream.setValue(array);
}
template<class T>
void operator>>(JZProjectStream& stream, QList<T> &list)
{
    list.clear();

    QJsonArray array = stream.value().toArray();
    for (int i = 0; i < array.size(); i++)
    {
        JZProjectStream s;
        s.setReadOnly(true);
        s.setValue(array[i]);                

        T v;
        s >> v;
        list.push_back(v);
    }    
}
template<class T>
void operator<<(JZProjectStream& stream, const QMap<QString,T> &map)
{
    QJsonObject obj;
    auto it = map.begin();
    while(it != map.end())
    {        
        JZProjectStream s;
        s << it.value();
        obj[it.key()] = s.value();
        
        it++;
    }
    stream.setValue(obj);
}
template<class T>
void operator>>(JZProjectStream& stream, QMap<QString, T> &map)
{
    map.clear();

    QJsonObject obj = stream.value().toObject();
    auto it = obj.begin();
    while (it != obj.end())
    {
        T v;
        JZProjectStream s;       
        s.setReadOnly(true);
        s.setValue(it.value());        

        s >> v;
        map.insert(it.key(), v);
        it++;
    }
    stream.setValue(obj);
}

class JZProjectFile
{
public:
    JZProjectFile();
    ~JZProjectFile();

    bool load(const QString &filename);
    bool save(const QString &filename);
    QString error();
    JZProjectStream &stream();    

protected:
    JZProjectStream m_stream;
    QString m_error;
};









#endif