#include <QDateTime>
#include <chrono>
#include "JZNodeTrace.h"
#include "JZNodeEngine.h"

//JZNodeTrace
JZNodeTrace::JZNodeTrace()
{
}
JZNodeTrace::~JZNodeTrace()
{
}

JZNodeTraceItemPtr createTrace(JZNodeTraceItem::Type type)
{
    // 使用 high_resolution_clock 获取当前时间点
    auto now = std::chrono::high_resolution_clock::now();
    // 转换为纳秒
    auto nanoseconds = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
    // 计算自纪元（epoch）以来的纳秒数
    auto epoch_time = nanoseconds.time_since_epoch();
    qint64 timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch_time).count();

    JZNodeTraceItemPtr ptr = JZNodeTraceItemPtr(new JZNodeTraceItem());
    ptr->type = type;
    ptr->timestamp = timestamp;
    return ptr;
}

void JZNodeTrace::mark(const QString &mark)
{
    JZNodeTraceItemPtr ptr = createTrace(JZNodeTraceItem::Mark);
    ptr->name = mark;
    g_engine->traceContext()->record(ptr);
}
    
void JZNodeTrace::push(const QString &text)
{
    JZNodeTraceItemPtr ptr = createTrace(JZNodeTraceItem::Push);
    ptr->name = text;
    g_engine->traceContext()->record(ptr);
}

void JZNodeTrace::pop()
{
    JZNodeTraceItemPtr ptr = createTrace(JZNodeTraceItem::Pop);
    g_engine->traceContext()->record(ptr);
}

//JZNodeTraceBuilder
JZNodeTraceBuilder::JZNodeTraceBuilder(JZNodeCompiler *c)
{
    m_compiler = c;
    m_id = m_compiler->addAllocStack("JZNodeTrace");
}

JZNodeTraceBuilder::~JZNodeTraceBuilder()
{
    m_compiler->addFreeStack(m_id);
}

void JZNodeTraceBuilder::mark(const QString &mark)
{
    QList<JZNodeIRParam> in,out;
    in << irId(m_id) << irLiteral(mark);
    m_compiler->addCall("JZNodeTrace::mark",in,out);
}

void JZNodeTraceBuilder::push(const QString &text)
{
    QList<JZNodeIRParam> in,out;
    in << irId(m_id) << irLiteral(mark);
    m_compiler->addCall("JZNodeTrace::push",in,out);
}

void JZNodeTraceBuilder::pop()
{
    QList<JZNodeIRParam> in,out;
    in << irId(m_id) << irLiteral(mark);
    m_compiler->addCall("JZNodeTrace::pop",in,out);
}

//JZNodeTraceContext
JZNodeTraceContext::JZNodeTraceContext()
{
}

JZNodeTraceContext::~JZNodeTraceContext()
{
}

void JZNodeTraceContext::record(JZNodeTraceItemPtr item)
{
    m_items.push_back(item);
    if(item->type == JZNodeTraceItem::Custom)
        emit sigTrace(item);
}

void JZNodeTraceContext::clear()
{
    m_items.clear();
}

bool JZNodeTraceContext::load(QString path)
{
    return false;
}

bool JZNodeTraceContext::save(QString path)
{
    return false;
}