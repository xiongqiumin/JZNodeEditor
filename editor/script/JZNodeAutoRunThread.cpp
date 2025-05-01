#include "JZNodeAutoRunThread.h"

//PropCoor
JZNodeAutoRunThread::JZNodeAutoRunThread()
{
     m_cancel = false;
     m_test.moveToThread(this);
}

JZNodeAutoRunThread::~JZNodeAutoRunThread()
{
}

JZNodeEngine *JZNodeAutoRunThread::engine()
{
    return m_test.engine();
}

JZScriptItemDepend *JZNodeAutoRunThread::genDepend(JZScriptItem *script)
{
    return m_test.genDepend(script);
}

void JZNodeAutoRunThread::startRun()
{        
    start();
}

void JZNodeAutoRunThread::stopRun()
{
    if (!isRunning())
        return;

    m_cancel = true;
    m_test.engine()->stop();
    m_cancel = false;
    wait();
}

void JZNodeAutoRunThread::run()
{   
    if (!m_test.init())
    {
        m_test.start();
        exec();
        m_test.deinit();
    }
    emit sigResult(0);
}