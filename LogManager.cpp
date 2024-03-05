#include "LogManager.h"
#include "stdarg.h"
#include <QTime>
#include <QDir>
#include <QApplication>
#include <QMetaType>

//LogManager
LogManager *LogManager::instance()
{
    static LogManager inst;
    return &inst;
}

LogManager::LogManager()
{
    qRegisterMetaType<LogObjectPtr>("LogObjectPtr");    
}

LogManager::~LogManager()
{
}

void LogManager::log(int module, int level, const QString &text)
{
    log(module, level, "%s", qUtf8Printable(text));
}

void LogManager::log(int module, int type, const char *format, ...)
{    
    va_list var;
    va_start(var, format);
    QString buffer = QString::vasprintf(qUtf8Printable(format), var);
    va_end(var);

    //·¢ËÍ
    LogObject *obj = new LogObject();
    obj->module = module;
    obj->level = type;
    obj->time = QDateTime::currentMSecsSinceEpoch();    
    obj->message = buffer;
    emit sigLog(LogObjectPtr(obj));
}