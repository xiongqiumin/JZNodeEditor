#include "JZEngineCoroutine.h"
#include "JZNodeEngine.h"

JZEngineCoroutine::JZEngineCoroutine(JZNodeEngine *engine)
{
    m_engine = engine;
    m_coId = -1;
}

JZEngineCoroutine::~JZEngineCoroutine()
{    
}

void JZEngineCoroutine::yield()
{
    m_engine->yieldCo(m_coId);
    JZCoCoroutine::yield();
}

void JZEngineCoroutine::resume()
{
    if(m_coId == -1)
        m_coId = m_engine->createCo();

    m_engine->resumeCo(m_coId);
    JZCoCoroutine::resume();
}

void JZEngineCoroutine::endTask()
{
    m_engine->yieldCo(m_coId);
    m_engine->destoryCo(m_coId);
    m_coId = -1;
}