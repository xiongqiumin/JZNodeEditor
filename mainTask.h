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
    qint64 changeTimestamp;    
    qint64 buildVersion;       
    qint64 buildTimestamp;     
    qint64 saveTimestamp;    
};

class MainTask
{
public:
    enum {
        Task_genProgram,        
        Task_unitTest,
        Task_dumpCpp,
        Task_dumpExe,
        Task_running,
    };

    MainTask();

    int type;
    bool isRunOnce;

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
    void stopRunThread();

    void removeTask(int type);
    void clearTask();

    void addAutoCompilerTask();
    void addBuildProgramTask();        
    void addTestTask(QString path,bool runOnce);
    void addExportExeTask();
    void addExportCppTask();    
    void addRunTask();

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