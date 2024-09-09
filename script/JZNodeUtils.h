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


MemberInfo jzSplitMember(QString name);
QString makeLink(QString tips, QString path, QString args);
JZUrl fromQUrl(QUrl url);
void projectUpdateLayout(JZProject *project);
void jzScriptItemUpdateLayout(JZScriptItem *item);
void jzScriptItemDump(JZScriptItem *item,QString file);

class JZNodeUtils
{
public:    
    static QString className(QString name);
};

#endif // !JZNODE_UTILS_H_