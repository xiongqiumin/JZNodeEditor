#ifndef MAIN_TASK_H_
#define MAIN_TASK_H_

#include <QObject>
#include "JZNodeAutoRunThread.h"
#include "JZNodeBuildThread.h"

struct BuildInfo
{
    BuildInfo();      

    void clear();

    bool success;
    qint64 changeTimestamp;    //改动时间
    qint64 buildVersion;       //编译版本
    qint64 buildTimestamp;     //编译成功时间
    qint64 saveTimestamp;    
};

class MainTask
{
public:
    enum {
        Task_build,        
        Task_unitTest,
        Task_dumpCpp,
        Task_dumpExe,
        Task_running,
    };

    int type;
    ScriptDepend unitDepend;
};

class MainTaskManager : public QObject
{
    Q_OBJECT

public:
    MainTaskManager();
    ~MainTaskManager();

    void setProject(JZProject *project);
    JZNodeBuildThread *buildThread();
    JZNodeAutoRunThread *runThread();

    void removeTask(int type);
    void clearTask();

    void addAutoCompilerTask();
    void addBuildTask();        
    void addUnitTestTask(QString path);
    void addExportExeTask();
    void addExportCppTask();    
    void addRunningTask();

signals:
    void sigBuildStart();
    void sigBuildFinish(JZNodeBuildResultPtr result);
    void sigTaskRunning();

protected slots:
    void onAutoCompilerTimer();
    void onBuildFinish(JZNodeBuildResultPtr result);

protected:
    void addTask(MainTask task);
    void build(bool mute);
    void dealTask();
    void saveProgram();

    QTimer *m_compilerTimer;

    BuildInfo m_buildInfo;
    JZNodeBuildResultPtr m_buildResult;
    JZNodeAutoRunThread m_runThread;
    JZNodeBuildThread m_buildThread;
    JZProject *m_project;
    QList<MainTask> m_taskList;        
};

#endif