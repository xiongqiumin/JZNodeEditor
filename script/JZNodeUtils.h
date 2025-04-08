#ifndef JZNODE_UTILS_H_
#define JZNODE_UTILS_H_

#include <QString>
#include "JZProject.h"

struct MemberInfo
{
    QString className;
    QString name;
};

class JZNodeUtils
{
public:    
    static QString className(QString name);
    static MemberInfo splitMember(QString name);
    static QString makeLink(QString tips, QString path, QString args);
};

#endif // !JZNODE_UTILS_H_