#include <QDateTime>
#include <chrono>
#include "JZNodeTrace.h"
#include "JZNodeEngine.h"
#include "JZNodeBind.h"

JZNodeTraceLog createTrace(JZNodeTraceLog::Type type)
{
    // 使用 high_resolution_clock 获取当前时间点
    auto now = std::chrono::high_resolution_clock::now();
    // 转换为纳秒
    auto nanoseconds = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
    // 计算自纪元（epoch）以来的纳秒数
    auto epoch_time = nanoseconds.time_since_epoch();
    qint64 timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch_time).count() / 1000;

    JZNodeTraceLog ptr;
    ptr.type = type;
    ptr.thread = -1;
    ptr.timestamp = timestamp;
    return ptr;
}

void JZTracePush(const QString &text)
{    
    if (!g_engine)
        return;

    JZNodeTraceLog ptr = createTrace(JZNodeTraceLog::Push);
    ptr.name = text;
    g_engine->trace(ptr);
}

void JZTracePop()
{
    if (!g_engine)
        return;

    JZNodeTraceLog ptr = createTrace(JZNodeTraceLog::Pop);
    g_engine->trace(ptr);
}

void JZTraceMark(const QString &text)
{
    if (!g_engine)
        return;

    JZNodeTraceLog ptr = createTrace(JZNodeTraceLog::Mark);
    ptr.name = text;
    g_engine->trace(ptr);
}

//JZNodeTrace
JZTraceScoped::JZTraceScoped(QString nodeName)
{
    JZTracePush(nodeName);
}

JZTraceScoped::~JZTraceScoped()
{
    JZTracePop();
}

//JZNodeTraceBuilder
JZNodeTraceBuilder::JZNodeTraceBuilder(JZNodeCompiler *c)
{
    m_compiler = c;
    JZNode* node = c->currentNode();    
    m_compiler->addCall("JZTracePush", { irLiteral(node->name()) }, { });
}

JZNodeTraceBuilder::~JZNodeTraceBuilder()
{
    m_compiler->addCall("JZTracePop", {}, {});
}

void JZNodeTraceBuilder::mark(const QString & text)
{
    QList<JZNodeIRParam> in,out;
    in << irLiteral(text);
    m_compiler->addCall("JZTraceMark",in,out);
}

void JZNodeTraceBuilder::push(const QString &text)
{
    QList<JZNodeIRParam> in,out;
    in << irLiteral(text);
    m_compiler->addCall("JZTracePush",in,out);
}

void JZNodeTraceBuilder::pop()
{
    QList<JZNodeIRParam> in,out;    
    m_compiler->addCall("JZTracePop",in,out);
}

//JZNodeTraceItem
JZNodeTraceItem::JZNodeTraceItem()
{
    thread = 0;
    type = 0;
    level = 0;
    start = 0;
    duration = 0;
}


//JZNodeTraceContext
JZNodeTraceContext::JZNodeTraceContext()
{
}

JZNodeTraceContext::~JZNodeTraceContext()
{
}

const JZNodeTraceRecord& JZNodeTraceContext::record() const
{
    return m_record;
}

JZNodeTraceRecord JZNodeTraceContext::parse()
{
    JZNodeTraceRecord result;

    QMap<int,QList<JZNodeTraceLog>> thread_map;
    for(int i = 0; i < m_items.size(); i++)
    {
        int tid = m_items[i].thread;
        int type = m_items[i].type;

        auto& thread_item = thread_map[tid];
        if(type == JZNodeTraceLog::Push)
        {
            thread_item.push_back(m_items[i]);
        }
        else if(type == JZNodeTraceLog::Pop)
        {
            JZNodeTraceLog rec = thread_item.back();
            thread_item.pop_back();

            JZNodeTraceItem item;
            item.type = rec.type;
            item.thread = tid;
            item.level = thread_item.size();
            item.name = rec.name;
            item.start = rec.timestamp;
            item.duration = m_items[i].timestamp - rec.timestamp;
            result.items << item;
        }
        else if(type == JZNodeTraceLog::Mark)
        {

        }
    }

    auto it = thread_map.begin();
    while (it != thread_map.end())
    {
        int tid = it.key();

        QList<JZNodeTraceLog> & push_items = it.value();
        while (push_items.size() != 0)
        {
            qint64 last = m_items.back().timestamp;
            JZNodeTraceLog rec = push_items.back();
            push_items.pop_back();

            JZNodeTraceItem item;
            item.type = rec.type;
            item.level = push_items.size();
            item.name = rec.name;
            item.start = rec.timestamp;
            item.duration = last - rec.timestamp;
            result.items << item;
        }

        int maxLevel = -1;
        for (int i = 0; i < result.items.size(); ++i) {
            if (result.items[i].thread == tid && result.items[i].level > maxLevel)
                maxLevel = result.items[i].level;
        }
        for (int i = 0; i < result.items.size(); i++)
        {
            if (result.items[i].thread == tid)
                result.items[i].level = maxLevel - result.items[i].level;
        }
        it++;
    }

    return result;
}

void JZNodeTraceContext::insert(JZNodeTraceLog item)
{
    m_items.push_back(item);
    if(item.type == JZNodeTraceLog::Custom)
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

void JZNodeTraceInit(JZScriptEnvironment *env)
{
    auto func_inst = env->functionManager();
    func_inst->registCFunction("JZTracePush", true, jzbind::createFuncion(JZTracePush));
    func_inst->registCFunction("JZTracePop", true, jzbind::createFuncion(JZTracePop));
    func_inst->registCFunction("JZTraceMark", true, jzbind::createFuncion(JZTraceMark));
}