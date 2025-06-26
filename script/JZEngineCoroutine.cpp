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

void JZEngineCoroutine::beforeYield()
{
    auto pre_co = dynamic_cast<JZEngineCoroutine*>(m_preCoList.back());
    if (pre_co)
        m_engine->switchCo(pre_co->m_coId);
    else
        m_engine->switchCo(-1);
}

void JZEngineCoroutine::beforeResume()
{
    if (m_coId == -1)
    {
        m_coId = m_engine->createCo(this);
    }
    m_engine->switchCo(m_coId);
}

void JZEngineCoroutine::endTask()
{
    auto pre_co = dynamic_cast<JZEngineCoroutine*>(m_preCoList.back());
    if (pre_co)
        m_engine->switchCo(pre_co->m_coId);
    else
        m_engine->switchCo(-1);

    m_engine->destoryCo(m_coId);
    m_coId = -1;
}