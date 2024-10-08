﻿#include "JZNodeAutoRunThread.h"

//PropCoor
JZNodeAutoRunThread::JZNodeAutoRunThread()
{
     m_cancel = false;
     m_engine.setWatch(true);
     m_engine.moveToThread(this);
}

JZNodeAutoRunThread::~JZNodeAutoRunThread()
{
}

JZNodeEngine *JZNodeAutoRunThread::engine()
{
    return &m_engine;
}

void JZNodeAutoRunThread::startRun(JZNodeProgram *program,const ScriptDepend &depend)
{
    m_depend = depend;
    m_program = program;
    start();
}

void JZNodeAutoRunThread::stopRun()
{
    m_cancel = true;
    m_engine.stop();
    m_cancel = false;
    wait();
}

void JZNodeAutoRunThread::run()
{
    auto ret = UnitTestResultPtr(new UnitTestResult());
   
    m_engine.setProgram(m_program);
    m_engine.init();

    QVariantList out;
    ret->function = m_depend.function.fullName();
    if (m_engine.callUnitTest(&m_depend,out))
    {
        ret->result = UnitTestResult::Finish;
        ret->out = out;
    }
    else
    {
        if(m_cancel)
            ret->result = UnitTestResult::Cancel;
        else
        {
            ret->result = UnitTestResult::Error;
            ret->runtimeError = m_engine.runtimeError();
        }
    }
    m_engine.deinit();
    m_cancel = false;
    emit sigResult(ret);
}