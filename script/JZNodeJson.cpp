#include <QJsonDocument>
#include <QJsonParseError>
#include <stdexcept>
#include "JZNodeJson.h"

QByteArray JZNodeJson::toBuffer(const QJsonObject &object)
{
    return QJsonDocument(object).toJson();
}

QJsonObject JZNodeJson::formBuffer(const QByteArray &buffer)
{
    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(buffer, &parse_error);
    if (parse_error.error != QJsonParseError::NoError)
    {
        throw std::runtime_error(qUtf8Printable(parse_error.errorString()));
    }
    return doc.object();
}        