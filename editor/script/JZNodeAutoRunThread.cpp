#include "JZNodeAutoRunThread.h"

//PropCoor
JZNodeAutoRunThread::JZNodeAutoRunThread()
{
}

JZNodeAutoRunThread::~JZNodeAutoRunThread()
{
}

void JZNodeAutoRunThread::startRun(JZNodeProgram *program,const ScriptDepend &depend)
{
    m_depend = depend;
    m_program = program;
    start();
}

void JZNodeAutoRunThread::stopRun()
{
    m_engine.stop();
}

void JZNodeAutoRunThread::run()
{
    auto ret = QSharedPointer<UnitTestResult>(new UnitTestResult());
   
    m_engine.setProgram(m_program);
    m_engine.init();

    QVariantList out;
    if (m_engine.callUnitTest(&m_depend,out))
    {
        ret->result = true;
        ret->out = out;
    }
    else
    {
        ret->result = false;
        ret->runtimeError = m_engine.runtimeError();
    }

    emit sigResult(ret);
}