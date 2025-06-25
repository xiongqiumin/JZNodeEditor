#include "JZEngineCoroutine.h"
#include "JZNodeEngine.h"

JZEngineCoroutine::JZEngineCoroutine(JZNodeEngine *engine)
{
    m_engine = engine;
    m_coId = m_engine->createCo();
}

JZEngineCoroutine::~JZEngineCoroutine()
{
    if(m_coId != -1)
        m_engine->destoryCo(m_coId);
}

void JZEngineCoroutine::yield()
{
    m_engine->yieldCo(m_coId);
    JZCoCoroutine::yield();
}

void JZEngineCoroutine::resume()
{
    m_engine->resumeCo(m_coId);
    JZCoCoroutine::resume();
}