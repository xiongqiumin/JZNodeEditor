#include <QTime>
#include <QDir>
#include <QApplication>
#include <QMetaType>
#include "LogManager.h"

void LogManagerInit()
{
    JZLogModuleConfig config;
    config.type = JZLogModuleConfig::Console;
    JZLogManager::instance()->addModule(Log_Compiler, config);
    JZLogManager::instance()->addModule(Log_Runtime, config);
}