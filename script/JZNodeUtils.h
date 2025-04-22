#ifndef JZNODE_UTILS_H_
#define JZNODE_UTILS_H_

#include <QString>
#include "JZProject.h"

struct MemberInfo
{
    QString className;
    QString name;
};

struct FunctionInfo
{
    QString className;
    QString name;
};


struct LinkInfo {
    QString name;    
    QString text;
    QVariantMap params;
};

class JZNodeUtils
{
public:    
    static MemberInfo splitMember(QString name);
    static QString makeLink(QString tips, QString path, QString args);
    static LinkInfo parseLink(QString line);
};

#endif // !JZNODE_UTILS_H_