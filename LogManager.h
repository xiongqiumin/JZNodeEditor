#pragma once
#include <QObject>
#include <QString>
#include <QMutex>
#include <QFile>
#include <QTimer>
#include <QDate>
#include <QMap>
#include <QSharedPointer>

enum {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL,
};

enum {
    Log_Compiler,
    Log_Runtime,
};

class LogObject
{
public:
    int module;
    int level;
    int64_t time;        
    QString message;    
};
typedef QSharedPointer<LogObject> LogObjectPtr;

class LogManager : public QObject
{
    Q_OBJECT

public:
    static LogManager *instance();

    void log(int module, int level, const QString &text);
    void log(int module, int level, const char *format, ...);        

signals:
    void sigLog(LogObjectPtr log);

protected:
    LogManager();
    ~LogManager();
};

#define LOGI(module,format,...)   LogManager::instance()->log(module,LOG_INFO,format,##__VA_ARGS__)
#define LOGW(module,format,...)   LogManager::instance()->log(module,LOG_WARNING,format,##__VA_ARGS__)
#define LOGE(module,format,...)   LogManager::instance()->log(module,LOG_ERROR,format,##__VA_ARGS__)
#define LOGF(module,format,...)   LogManager::instance()->log(module,LOG_FATAL,format,##__VA_ARGS__)
