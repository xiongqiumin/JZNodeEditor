#ifndef JZNODE_JSON_H_
#define JZNODE_JSON_H_

#include <QJsonObject>
#include <QByteArray>

class JZNodeJson
{
public:
    static QByteArray toBuffer(const QJsonObject& object);
    static QJsonObject formBuffer(const QByteArray& buffer);
};


#endif