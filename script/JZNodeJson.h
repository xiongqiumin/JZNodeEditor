#ifndef JZNODE_JSON_H_
#define JZNODE_JSON_H_

#include <QJsonObject>
#include <QByteArray>
#include "3rd/JZCommon/jzJson/JZJsonStream.h"

class JZNodeJson
{
public:
    static QByteArray toBuffer(const QJsonObject& object);
    static QJsonObject formBuffer(const QByteArray& buffer);

    static QJsonValue getValue(const QJsonValue& obj, const QString& path);

    template<class T>
    static T getValue(const QJsonValue& obj, const QString& path)
    {
        QJsonValue json_value = getValue(obj, path);
        JZJsonStream s;
        s.value = json_value;

        T t;
        s >> t;
        return t;
    }
};


#endif