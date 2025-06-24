#include <QDateTime>
#include <chrono>
#include "JZNodeTrace.h"
#include "JZNodeEngine.h"
#include "JZNodeBind.h"

JZNodeTraceRecordPtr createTrace(JZNodeTraceRecord::Type type)
{
    // 使用 high_resolution_clock 获取当前时间点
    auto now = std::chrono::high_resolution_clock::now();
    // 转换为纳秒
    auto nanoseconds = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
    // 计算自纪元（epoch）以来的纳秒数
    auto epoch_time = nanoseconds.time_since_epoch();
    qint64 timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch_time).count() / 1000;

    JZNodeTraceRecordPtr ptr = JZNodeTraceRecordPtr(new JZNodeTraceRecord());
    ptr->type = type;
    ptr->timestamp = timestamp;
    return ptr;
}

void JZTracePush(const QString &text)
{    
    if (!g_engine)
        return;

    JZNodeTraceRecordPtr ptr = createTrace(JZNodeTraceRecord::Push);
    ptr->name = text;
    g_engine->traceContext()->record(ptr);
}

void JZTracePop()
{
    if (!g_engine)
        return;

    JZNodeTraceRecordPtr ptr = createTrace(JZNodeTraceRecord::Pop);
    g_engine->traceContext()->record(ptr);
}

void JZTraceMark(const QString &text)
{
    if (!g_engine)
        return;

    JZNodeTraceRecordPtr ptr = createTrace(JZNodeTraceRecord::Mark);
    ptr->name = text;
    g_engine->traceContext()->record(ptr);
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

//JZNodeTraceContext
JZNodeTraceContext::JZNodeTraceContext()
{
}

JZNodeTraceContext::~JZNodeTraceContext()
{
}

QList<JZNodeTraceItem> JZNodeTraceContext::parse()
{
    QList<JZNodeTraceItem> result;

    QList<JZNodeTraceRecordPtr> push_items;
    for(int i = 0; i < m_items.size(); i++)
    {
        int type = m_items[i]->type;
        if(type == JZNodeTraceRecord::Push)
        {
            push_items.push_back(m_items[i]);
        }
        else if(type == JZNodeTraceRecord::Pop)
        {
            JZNodeTraceRecordPtr rec = push_items.back();
            push_items.pop_back();

            JZNodeTraceItem item;
            item.type = rec->type;
            item.level = push_items.size();
            item.name = rec->name;
            item.start = rec->timestamp;
            item.duration = m_items[i]->timestamp - rec->timestamp;
            result << item;
        }
        else if(type == JZNodeTraceRecord::Mark)
        {

        }
    }

    while(push_items.size() != 0)
    {
        qint64 last = m_items.back()->timestamp;
        JZNodeTraceRecordPtr rec = push_items.back();
        push_items.pop_back();

        JZNodeTraceItem item;
        item.type = rec->type;
        item.level = push_items.size();
        item.name = rec->name;
        item.start = rec->timestamp;
        item.duration = last - rec->timestamp;
        result << item;
    }

    int maxLevel = -1;
    for (int i = 0; i < result.size(); ++i) {
        if (result[i].level > maxLevel)
            maxLevel = result[i].level;
    }
    for(int i = 0; i < result.size(); i++)
    {
        result[i].level = maxLevel - result[i].level;
    }

    return result;
}

void JZNodeTraceContext::record(JZNodeTraceRecordPtr item)
{
    m_items.push_back(item);
    if(item->type == JZNodeTraceRecord::Custom)
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