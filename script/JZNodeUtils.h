#ifndef JZNODE_UTILS_H_
#define JZNODE_UTILS_H_

#include <QString>
#include "JZProject.h"

struct MemberInfo
{
    QString className;
    QString name;
};

MemberInfo jzSplitMember(QString name);
QString makeLink(QString tips, QString filename, int nodeId);
void projectUpdateLayout(JZProject *project);
void jzScriptItemUpdateLayout(JZScriptItem *item);
void jzScriptItemDump(JZScriptItem *item,QString file);

class JZNodeUtils
{
public:    
    static QString className(QString name);
};

#endif // !JZNODE_UTILS_H_