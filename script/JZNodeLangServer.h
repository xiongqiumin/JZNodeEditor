#ifndef JZNODE_LANG_SERVER_H_
#define JZNODE_LANG_SERVER_H_

#include <QString> 

class JZProject;
class JZNodeLangServer
{
public:
    void setProject(JZProject *project);

    QStringList param(const QString &path);
    QStringList member(const QString &path,QString param);

protected:
    JZNodeLangServer();

    JZProject *m_project;
};

#endif // ! JZNODE_LANG_SERVER_H_
