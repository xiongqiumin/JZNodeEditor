#include <QDateTime>
#include <QDebug>
#include "JZNodeUtils.h"
#include "JZRegExpHelp.h"

#include "3rd/JZCommon/jzJson/JZJsonHelper.h"

QByteArray JZNodeUtils::toBuffer(const QJsonObject &object)
{
    return QJsonDocument(object).toJson();
}

QJsonObject JZNodeUtils::formBuffer(const QByteArray &buffer)
{
    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(buffer, &parse_error);
    if (parse_error.error != QJsonParseError::NoError)
    {
        throw std::runtime_error(qUtf8Printable(parse_error.errorString()));
    }
    return doc.object();
}

QJsonValue JZNodeUtils::getValue(const QJsonValue& obj, const QString& path)
{
    QJsonValue value;
    if (!JZJsonHelper::getValue(obj, path, value))
        throw std::runtime_error("no such varible");

    return value;
}

MemberInfo JZNodeUtils::splitMember(QString fullName)
{
    MemberInfo info;
    QStringList list = fullName.split(".");
    if (list.size() > 1)
        info.className = list[0];
    
    info.name = list.back();
    return info;
}

QString JZNodeUtils::makeLink(QString tips, QString path, QString args)
{
    QString href = path + "?" + args;
    QString link = "<link href=" + href + ">" + tips + "</link>";
    return link;
}

LinkInfo JZNodeUtils::parseLink(QString text)
{
    LinkInfo tg;
    
    int s1 = text.indexOf("<");
    int e1 = text.indexOf(" ");
    tg.name = text.mid(s1 + 1, e1 - (s1 + 1));
    s1 = e1 + 1;
    e1 = text.indexOf(">");

    QString param_line = text.mid(s1, e1 - s1);
    QStringList lines = param_line.split(" ");
    for (int i = 0; i < lines.size(); i++)
    {
        QString attr = lines[i];
        int idx = attr.indexOf("=");
        tg.params[attr.left(idx)] = attr.mid(idx+1);
    }

    int s2 = text.indexOf("<", e1);
    tg.text = text.mid(e1 + 1, s2 - (e1 + 1));

    return tg;
}