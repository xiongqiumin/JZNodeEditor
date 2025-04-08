#include "JZNodeUtils.h"
#include <QDateTime>
#include <QDebug>

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

QString JZNodeUtils::className(QString name)
{
    MemberInfo ret = JZNodeUtils::splitMember(name);
    return ret.className;
}