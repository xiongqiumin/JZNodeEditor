#include "jzCo/JZCo.h"
#include "JZNodeRuntime.h"
#include "JZNodeEngine.h"

void JZSleep(int ms)
{
    if (g_scheduler->isInCoroutine())
    {
        jzco_sleep(ms);
        if(g_engine->status() == Status_error)
            throw std::runtime_error("interrupt");
    }
    else
        QThread::msleep(ms);
}