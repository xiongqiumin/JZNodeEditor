#include "JZNodeLangServer.h"
#include "JZProject.h"

JZNodeLangServer::JZNodeLangServer()
{
    m_project = nullptr;
}

void JZNodeLangServer::setProject(JZProject *project)
{
    m_project = project;
}

QStringList JZNodeLangServer::param(const QString &path)
{
    QStringList list;
    return list;
}

QStringList JZNodeLangServer::member(const QString &path, QString param)
{
    QStringList list;
    return list;
}