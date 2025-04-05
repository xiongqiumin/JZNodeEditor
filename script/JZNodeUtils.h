#ifndef JZNODE_UTILS_H_
#define JZNODE_UTILS_H_

#include <QString>
#include "JZProject.h"

struct MemberInfo
{
    QString className;
    QString name;
};

class JZUrl
{
public:
    QString path;
    QMap<QString, QString> args;
};


class JZNodeUtils
{
public:    
    static QString className(QString name);
    static MemberInfo splitMember(QString name);
    static QString makeLink(QString tips, QString path, QString args);
    static JZUrl fromQUrl(QUrl url);

    static void projectUpdateLayout(JZProject *project);
    static void scriptItemUpdateLayout(JZScriptItem *item);
    static void scriptItemDump(JZScriptItem *item,QString file);
};

#endif // !JZNODE_UTILS_H_