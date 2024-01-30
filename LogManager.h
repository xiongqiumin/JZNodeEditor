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

class LogModule
{
public:
    int module;
    QString name;
    QFile file;
};
typedef QSharedPointer<LogModule> LogModulePtr;


class LogManager : public QObject
{
    Q_OBJECT

public:
    static LogManager *instance();

    void init();
    void deinit();

    void addModule(int id,QString name);
    void log(int module, int level, const char *format, ...);

    template<typename...Args>
    void log(int module, int level, const QString &format, Args ...args)
    {
        log(module, level, qUtf8Printable(format), args...);
    }

signals:
    void sigLog(LogObjectPtr log);

protected slots:
    void onTimeOut();

protected:
    LogManager();
    ~LogManager();

    void createFile();
    void flush();
    
    bool m_init;
    QTimer m_timer;    

    QMutex m_mutex;
    QMap<int, LogModulePtr> m_logs;
};

#define LOGI(module,format,...)   LogManager::instance()->log(module,LOG_INFO,format,__VA_ARGS__)
#define LOGW(module,format,...)   LogManager::instance()->log(module,LOG_WARNING,format,__VA_ARGS__)
#define LOGE(module,format,...)   LogManager::instance()->log(module,LOG_ERROR,format,__VA_ARGS__)
#define LOGF(module,format,...)   LogManager::instance()->log(module,LOG_FATAL,format,__VA_ARGS__)