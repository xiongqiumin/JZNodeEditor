#ifndef JZ_ENGINE_COROUTINE_H_
#define JZ_ENGINE_COROUTINE_H_

#include "jzCo/JZCo.h"

class JZEngineCoInterrupt
{
public:
};

class JZNodeEngine;
class JZEngineCoroutine : public JZCoCoroutine
{
public:
    JZEngineCoroutine(JZNodeEngine *engine);
    ~JZEngineCoroutine();

    virtual void resume();
    virtual void yield();
    virtual void endTask();    

protected:    
    JZNodeEngine *m_engine;
    int m_coId;
};


#endif