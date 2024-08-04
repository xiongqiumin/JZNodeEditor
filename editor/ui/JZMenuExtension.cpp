#include "JZMenuExtension.h"

//JZMenuExtension
JZMenuExtension::JZMenuExtension()
{
}

JZMenuExtension::~JZMenuExtension()
{
}

QList<QAction*> JZMenuExtension::taskActions() const
{
    QList<QAction*> ret;
    ret.push_back(new QAction("hello world"));
    return ret;
}

//JZMenuExtensionFactory
JZMenuExtensionFactory::JZMenuExtensionFactory()
{
}

JZMenuExtensionFactory::~JZMenuExtensionFactory()
{
}

QObject *JZMenuExtensionFactory::extension(QObject *object, const QString &iid) const
{
    return (QObject*)&m_extMenu;
}