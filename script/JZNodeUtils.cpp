#include <QDateTime>
#include <QDebug>
#include "JZNodeUtils.h"
#include "JZRegExpHelp.h"

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