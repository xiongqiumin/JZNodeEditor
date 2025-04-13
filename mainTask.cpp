#include <QDebug>
#include <QDateTime>
#include <QDir>
#include "MainTask.h"
#include "JZNodeProgramDumper.h"
#include "LogManager.h"

//AutoBuildInfo
BuildInfo::BuildInfo()
{
    clear();
}

void BuildInfo::clear()
{
    changeTimestamp = QDateTime::currentMSecsSinceEpoch();
    buildTimestamp = 0;
    saveTimestamp = 0;
    buildVersion = 0;
    success = false;
}

//MainTaskManager
MainTaskManager::MainTaskManager()
{    
    m_compilerTimer = new QTimer(this);
    connect(m_compilerTimer, &QTimer::timeout, this, &MainTaskManager::onAutoCompilerTimer);
    connect(&m_buildThread, &JZNodeBuildThread::sigResult, this, &MainTaskManager::onBuildFinish);
    m_compilerTimer->start(100);
}

MainTaskManager::~MainTaskManager()
{
}

void MainTaskManager::setProject(JZProject *project)
{
    m_project = project;
}

JZNodeBuildThread *MainTaskManager::buildThread()
{
    return &m_buildThread;
}

JZNodeAutoRunThread *MainTaskManager::runThread()
{
    return &m_runThread;
}

void MainTaskManager::addTask(MainTask task)
{
    removeTask(task.type);
    m_taskList.append(task);
}

void MainTaskManager::removeTask(int type)
{
    if (type == MainTask::Task_unitTest)
        m_runThread.stopRun();
}

void MainTaskManager::clearTask()
{

}

void MainTaskManager::build(bool mute)
{
    if (m_buildThread.isRunning() && m_buildInfo.changeTimestamp == m_buildInfo.buildVersion)
        return;
    
    if (!m_buildResult || m_buildResult->status != Build_Successed || m_buildInfo.changeTimestamp > m_buildInfo.buildTimestamp)
    {
        m_buildInfo.buildVersion = m_buildInfo.changeTimestamp;

        clearTask();
        if(!mute)
            emit sigBuildStart();

        m_buildInfo.success = false;        
        m_runThread.stopRun();
        m_buildThread.stopBuild();
        m_buildThread.setMute(mute);
        m_buildThread.startBuild(m_project);
    }
    else
    {
        QTimer::singleShot(0, this, [this] {
            dealTask();
        });
    }
}

void MainTaskManager::addBuildTask()
{    
    build(false);

    MainTask task;
    task.type = MainTask::Task_build;
    addTask(task);
}

void MainTaskManager::addAutoCompilerTask()
{
    m_buildInfo.changeTimestamp = QDateTime::currentMSecsSinceEpoch();
}

void MainTaskManager::addUnitTestTask(QString path)
{
    build(false);

    MainTask task;
    task.type = MainTask::Task_unitTest;
    addTask(task);
}

void MainTaskManager::addExportCppTask()
{
    addBuildTask();

    MainTask task;
    task.type = MainTask::Task_dumpCpp;
    addTask(task);
}

void MainTaskManager::addExportExeTask()
{
    addBuildTask();

    MainTask task;
    task.type = MainTask::Task_dumpExe;
    addTask(task);
}

void MainTaskManager::addRunningTask()
{
    addBuildTask();

    MainTask task;
    task.type = MainTask::Task_running;
    addTask(task);
}

void MainTaskManager::onAutoCompilerTimer()
{
    qint64 cur = QDateTime::currentMSecsSinceEpoch();
    if (cur - m_buildInfo.changeTimestamp <= 1000)
        return;

    if (m_project->isNull())
        return;

    if (m_buildInfo.changeTimestamp > m_buildInfo.buildVersion)
        build(true);
}

void MainTaskManager::onBuildFinish(JZNodeBuildResultPtr result)
{
    m_buildResult = result;

    emit sigBuildFinish(result);
    if (result->status != Build_Successed)
        return;

    dealTask();
}

void MainTaskManager::dealTask()
{    
    for (int i = 0; i < m_taskList.size(); i++)
    {
        auto &task = m_taskList[i];
        switch (task.type)
        {   
            case MainTask::Task_build:
            {
                saveProgram();
                break;
            }
            case MainTask::Task_unitTest:
            {
                m_runThread.startRun(&m_buildResult->program, task.unitDepend);
                break;
            }
            case MainTask::Task_dumpCpp:
                break;
            case MainTask::Task_dumpExe:
                break;
            case MainTask::Task_running:
            {
                emit sigTaskRunning();
                break;
            }
            default:
                break;
        }
    }
    m_taskList.clear();
}

void MainTaskManager::saveProgram()
{
    QString build_path = m_project->path() + "/build";
    QString build_exe = build_path + "/" + m_project->name() + ".program";

    QElapsedTimer timer;
    timer.start();

    QDir dir;
    if (!dir.exists(build_path))
        dir.mkdir(build_path);
    if (!m_buildResult->program.save(build_exe))
    {
        LOGMOD_W(Log_Compiler, "generate program failed");        
    }

    JZNodeProgramDumper dumper;
    dumper.init(m_project, &m_buildResult->program);
    dumper.dump(build_path + "/dump");    
}