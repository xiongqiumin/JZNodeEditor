#include "JZModule.h"

// /JZModule
JZModule::JZModule()
{
}

JZModule::~JZModule()
{
}

QString JZModule::name()
{
    return QString();
}

QStringList JZModule::depends()
{
    return QStringList();
}

void JZModule::regist()
{
}

//JZModuleManager
JZModuleManager *JZModuleManager::instance()
{
    return nullptr;
}

JZModuleManager::JZModuleManager()
{

}

JZModuleManager::~JZModuleManager()
{
}