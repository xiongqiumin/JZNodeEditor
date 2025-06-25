#include "JZNodeRuntime.h"
#include "JZNodeEngine.h"

void JZSleep(int ms)
{
    if (g_scheduler->isInCoroutine())
    {
        jzco_sleep(ms);
        if(g_engine->isInterruptCo())
            throw JZEngineCoInterrupt();
    }
    else
        QThread::msleep(ms);
}