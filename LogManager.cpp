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
    m_init = false;
}

LogManager::~LogManager()
{
    deinit();
}

void LogManager::init()
{
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimeOut()));
    
    m_timer.start(5 * 1000);
    m_init = true;
}

void LogManager::deinit()
{
    QMutexLocker locker(&m_mutex);
    if (m_init)
    {
        flush();
        this->disconnect();

        m_timer.stop();
        m_init = false;
    }
}

void LogManager::onTimeOut()
{
    QMutexLocker locker(&m_mutex);
    flush();    
}

void LogManager::addModule(int id, QString name)
{
    auto module = new LogModule();
    LogModulePtr ptr = LogModulePtr(module);
    m_logs[id] = ptr;
}

void LogManager::createFile()
{

}

void LogManager::flush()
{

}

void LogManager::log(int module, int type, const char *format, ...)
{
    QMutexLocker locker(&m_mutex);
    if (!m_init)
        return;
    
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