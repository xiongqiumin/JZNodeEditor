#include "JZNodeBuildThread.h"
#include "LogManager.h"

JZNodeBuildThread::JZNodeBuildThread()
{
    m_builder.setProject(&m_project);
}

JZNodeBuildThread::~JZNodeBuildThread()
{
}

void JZNodeBuildThread::init(JZNodeProgram *program)
{    
    m_program = program;
}

JZNodeBuilder *JZNodeBuildThread::builder()
{
    return &m_builder;
}

void JZNodeBuildThread::sync(JZProject *project)
{
    m_project.clear();
    project->copyTo(&m_project);
}

void JZNodeBuildThread::startBuild(JZProject *project)
{
    sync(project);
    start();
}

void JZNodeBuildThread::stopBuild()
{
    m_builder.stopBuild();
    wait();
}

void JZNodeBuildThread::run()
{
    bool ret = m_builder.build(m_program);
    if(!ret)
        LOGE(Log_Compiler, m_builder.error());
    emit sigResult(ret);
}