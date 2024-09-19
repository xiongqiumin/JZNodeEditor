#include "JZNodeLangServer.h"
#include "JZProject.h"

JZNodeLangServer *JZNodeLangServer::instance()
{
    static JZNodeLangServer inst;
    return &inst;
}

JZNodeLangServer::JZNodeLangServer()
{
    m_project = nullptr;
}

void JZNodeLangServer::setProject(JZProject *project)
{
    m_project = project;
}

QStringList JZNodeLangServer::type(const QString &path)
{
    QStringList list;
    return list;
}

QStringList JZNodeLangServer::param(const QString &path)
{
    QStringList list;
    return list;
}

QStringList JZNodeLangServer::member(const QString &class_name)
{
    QStringList list;
    return list;
}