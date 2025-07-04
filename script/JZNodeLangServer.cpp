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

JZNodeLangServer::~JZNodeLangServer()
{
}

void JZNodeLangServer::setProject(JZProject *project)
{
    m_project = project;
    connect(m_project,&JZProject::sigDefineChanged, this, &JZNodeLangServer::onDefineChanged);

    auto env = m_project->environment();

    QList<int> list = { Type_bool, Type_int, Type_double, Type_string };
    env->typeListToNameList(list);
}

void JZNodeLangServer::onDefineChanged()
{
}

QStringList JZNodeLangServer::type()
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