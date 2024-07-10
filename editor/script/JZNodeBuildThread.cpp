#include "JZNodeBuildThread.h"

JZNodeBuildThread::JZNodeBuildThread()
{
}

JZNodeBuildThread::~JZNodeBuildThread()
{
}

void JZNodeBuildThread::init(JZProject *project,JZNodeProgram *program)
{
    m_builder.setProject(project);
    m_program = program;
}

JZNodeBuilder *JZNodeBuildThread::builder()
{
    return &m_builder;
}

void JZNodeBuildThread::startBuild()
{
    start();
}

void JZNodeBuildThread::stopBuild()
{
    m_builder.stopBuild();
}

void JZNodeBuildThread::run()
{
    bool ret = m_builder.build(m_program);
    emit sigResult(ret);
}