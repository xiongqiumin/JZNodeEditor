#include "JZIconManager.h"

JZIconManager *JZIconManager::instance()
{
    static JZIconManager inst;
    return &inst;
}

JZIconManager::JZIconManager()
{
}

JZIconManager::~JZIconManager()
{
}

QIcon JZIconManager::icon(QString name)
{
    return QIcon(":/JZNodeEditor/Resources/icons/" + name + ".png");
}