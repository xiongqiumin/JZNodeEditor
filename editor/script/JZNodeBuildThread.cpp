#include "JZNodeBuildThread.h"
#include "LogManager.h"

JZNodeBuildThread::JZNodeBuildThread()
{
    m_builder.setProject(&m_project);
}

JZNodeBuildThread::~JZNodeBuildThread()
{
}

void JZNodeBuildThread::sync(JZProject *project)
{
    m_project.clear();
    project->copyTo(&m_project);
}

void JZNodeBuildThread::setMute(bool mute)
{
    m_builder.setMute(mute);
}

void JZNodeBuildThread::startBuild(JZProject *project)
{
    sync(project);
    start();
}

bool JZNodeBuildThread::isBuild()
{
    return isRunning();
}

void JZNodeBuildThread::stopBuild()
{
    m_builder.stopBuild();
    wait();
}

void JZNodeBuildThread::run()
{
    bool ret = m_builder.build(&m_program);
    JZNodeBuildResultPtr ptr = JZNodeBuildResultPtr(new JZNodeBuildResult());
    ptr->status = ret ? Build_Successed : Build_Failed;        
    ptr->compilerResult = m_builder.compilerResult();
    if (ret)
        m_program.copyTo(&ptr->program);

    emit sigResult(ptr);
}