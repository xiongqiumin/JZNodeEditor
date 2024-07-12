#include "JZNodeEngine.h"
#include "JZNodeFunctionManager.h"
#include "JZEvent.h"
#include <QPushButton>
#include <QApplication>
#include <math.h>
#include <QDateTime>
#include "JZNodeVM.h"
#include "JZNodeBind.h"
#include "JZNodeQtBind.h"
#include "JZNodeObjectParser.h"
#include "JZContainer.h"

bool JZForCheck(int first,int last,int step,int op,QString &error)
{
    if (op == OP_eq)
    {
        if (first == last)
            return true;
        if (first < last && step > 0 && (last - first) % step == 0)
            return true;
        if (first > last && step < 0 && (first - last) % (-step) == 0)
            return true;
    }
    else if (op == OP_ne)
    {
        if ((first != last) || (step != 0))
            return true;
    }
    else
    {
        if(!((first < last && step <= 0 && (op == OP_lt || op == OP_le))
            || (first > last && step >= 0 && (op == OP_gt || op == OP_ge))))
            return true;
    }

    error = QString::asprintf("Dead loop, please check, first=%d, last=%d, step=%d, op='%s'", 
        first, last, step, qUtf8Printable(JZNodeType::opName(op)));
    return false;
}

void JZForRuntimeCheck(int first, int last, int step, int op)
{
    QString error;
    if (JZForCheck(first, last, step, op, error))
        return;
    
    throw std::runtime_error(qUtf8Printable(error));
}

void QObjectConnect(QObject *sender, JZFunctionPointer signal, QObject *recv, JZFunctionPointer slot)
{
    auto jz_sender = qobjectToJZObject(sender);
    auto jz_recv = qobjectToJZObject(recv);
    Q_ASSERT(jz_sender && jz_recv);
    JZObjectConnect(jz_sender,signal,jz_recv,slot);
}

void QObjectDisconnect(QObject *sender, JZFunctionPointer signal, QObject *recv, JZFunctionPointer slot)
{
    auto jz_sender = qobjectToJZObject(sender);
    auto jz_recv = qobjectToJZObject(recv);
    Q_ASSERT(jz_sender && jz_recv);
    JZObjectDisconnect(jz_sender,signal,jz_recv,slot);
}

QString JZObjectToString(JZNodeObject *obj)
{
    JZNodeObjectFormat format;
    return format.format(obj);
}

JZNodeVariantAny JZObjectCreate(QString name)
{
    int id = JZNodeObjectManager::instance()->getClassId(name);
    JZNodeObject *obj = JZNodeObjectManager::instance()->create(id);
    JZNodeVariantAny any;
    any.value = QVariant::fromValue(JZNodeObjectPtr(obj,true));
    return any;
}

void JZScriptLog(const QString &log)
{
    g_engine->print(log);
    qDebug() << log;
}

void JZScriptInvoke(const QString &function, const QVariantList &in, QVariantList &out)
{
    g_engine->invoke(function, in, out);
}

void JZScriptOnSlot(const QString &function, const QVariantList &in, QVariantList &out)
{
    g_engine->onSlot(function,in,out);
}

//RunnerEnv
RunnerEnv::RunnerEnv()
{
    function = nullptr;
    script = nullptr;
    pc = -1;
}

RunnerEnv::~RunnerEnv()
{   
    clearIrCache();
}

void RunnerEnv::initVariable(QString name, const QVariant &value)
{
    locals[name] = QVariantPtr(new QVariant(value));
}

void RunnerEnv::initVariable(int id, const QVariant &value)
{
    stacks[id] = QVariantPtr(new QVariant(value));
}

QVariant *RunnerEnv::getRef(int id)
{
    auto it = stacks.find(id);
    if (it == stacks.end())
        return nullptr;

    return it->data();
}

QVariant *RunnerEnv::getRef(QString name)
{
    auto it = locals.find(name);
    if (it == locals.end())
        return nullptr;

    return it->data();
}

void RunnerEnv::clearIrCache()
{
    auto it = irParamCache.begin();
    while(it != irParamCache.end())
    {
        it.key()->cache = nullptr;
        it++;
    }
}

void RunnerEnv::applyIrCache()
{
    auto it = irParamCache.begin();
    while(it != irParamCache.end())
    {
        it.key()->cache = it.value();
        it++;
    }
}

Stack::Stack()
{
    clear();
}

Stack::~Stack()
{
}

void Stack::clear()
{
    m_env.clear();  
}

int Stack::size() const
{
    return m_env.size();
}

bool Stack::isEmpty() const
{
    return (m_env.size() == 0);
}

RunnerEnv *Stack::currentEnv()
{
    return &m_env.back();
}

RunnerEnv *Stack::env(int index)
{
    return &m_env[index];
}

void Stack::pop()
{       
    m_env.back().clearIrCache();
    m_env.pop_back();
    if(m_env.size() > 0)
        m_env.back().applyIrCache();
}

void Stack::push()
{        
    if(m_env.size() > 0)
        m_env.back().clearIrCache();   
    m_env.push_back(RunnerEnv());
}

//JZNodeRuntimeError
QString JZNodeRuntimeError::errorReport()
{
    QString text = "Error: " + error + "\n\n";
    int stack_size = info.stacks.size();
    for (int i = 0; i < stack_size; i++)
    {
        auto s = info.stacks[stack_size - i - 1];
        text += QString().asprintf("# %2d: ",i+1) + s.function;
        if (!s.file.isEmpty())
            text += +"(" + s.file + "," + QString::number(s.pc) + ")";
        text += "\n";
    }
    return text;
}

QDataStream &operator<<(QDataStream &s, const JZNodeRuntimeError &param)
{
    s << param.error;
    s << param.info;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeRuntimeError &param)
{
    s >> param.error;
    s >> param.info;
    return s;
}

//JZNodeRuntimeInfo
JZNodeRuntimeInfo::Stack::Stack()
{
    nodeId = -1;
    pc = -1;
}

JZNodeRuntimeInfo::JZNodeRuntimeInfo()
{
    status = Status_none;
}

QDataStream &operator<<(QDataStream &s, const JZNodeRuntimeInfo::Stack &param)
{
    s << param.function;
    s << param.file;
    s << param.nodeId;
    s << param.pc;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeRuntimeInfo::Stack &param)
{
    s >> param.function;
    s >> param.file;
    s >> param.nodeId;
    s >> param.pc;
    return s;
}

QDataStream &operator<<(QDataStream &s, const JZNodeRuntimeInfo &param)
{        
    s << param.status << param.stacks;
    return s;    
}

QDataStream &operator>>(QDataStream &s, JZNodeRuntimeInfo &param)
{    
    s >> param.status >> param.stacks;
    return s;
}

//UnitTestResult
UnitTestResult::UnitTestResult()
{
    result = None;
}

//BreakPoint
BreakPoint::BreakPoint()
{    
    clear();        
}

void BreakPoint::clear()
{    
    type = BreakPoint::none;
    file.clear();
    nodeId = -1;
    stack = -1;        
}

//JZNodeEngineIdlePauseEvent
class JZNodeEngineIdlePauseEvent: public QEvent
{
public:    
    static int Event;

    JZNodeEngineIdlePauseEvent()
        :QEvent((QEvent::Type)Event)
    {
    }

    virtual ~JZNodeEngineIdlePauseEvent()
    {
    }
};
int JZNodeEngineIdlePauseEvent::Event = 0;


JZNodeEngine::Stat::Stat()
{
    clear();
}

void JZNodeEngine::Stat::clear()
{   
    getTime = 0;
    setTime = 0;
    statmentTime = 0;
    callTime = 0;
    exprTime = 0;
}

void JZNodeEngine::Stat::report()
{
    QString text;
    text += "statmentTime:" + QString::number(statmentTime) + "\n"; 
    text += "callTime:" + QString::number(callTime) + "\n";
    text += "exprTime:" + QString::number(exprTime) + "\n"; 
    text += "getTime:" + QString::number(getTime) + "\n"; 
    text += "setTime:" + QString::number(setTime) + "\n"; 
    qDebug().noquote() << text;
}

// JZNodeEngine
JZNodeEngine *g_engine = nullptr;

void JZNodeEngine::regist()
{
    JZNodeEngineIdlePauseEvent::Event = QEvent::registerEventType();

    JZNodeFunctionManager::instance()->registCFunction("createObject", false, jzbind::createFuncion(QOverload<QString>::of(JZObjectCreate)));
    JZNodeFunctionManager::instance()->registCFunction("connect", false, jzbind::createFuncion(QObjectConnect));
    JZNodeFunctionManager::instance()->registCFunction("disconnect", false, jzbind::createFuncion(QObjectDisconnect));

    JZNodeFunctionManager::instance()->registCFunction("forRuntimeCheck", true, jzbind::createFuncion(JZForRuntimeCheck));    
}

JZNodeEngine::JZNodeEngine()
{    
    m_program = nullptr;
    m_script = nullptr;
    m_sender = nullptr;
    m_depend = nullptr;
    m_pc = -1;        
    m_debug = false;
    m_status = Status_none;
    m_statusCommand = Command_none;
    m_regs.resize(Reg_End - Reg_Start);
    m_hookEnable = false;
}

JZNodeEngine::~JZNodeEngine()
{    
    clear();
}

void JZNodeEngine::setProgram(JZNodeProgram *program)
{
    m_program = program;
}

JZNodeProgram *JZNodeEngine::program()
{
    return m_program;
}

void JZNodeEngine::clear()
{    
    m_breakPoints.clear();
    m_breakStep.clear();    

    m_stack.clear();
    m_global.clear();   
    m_sender = nullptr;
    m_depend = nullptr;
    m_hookEnable = false;
    m_dependHook.clear();
    m_statusCommand = Command_none;
    m_status = Status_none;
    m_breakNodeId = -1;    
    m_watchTime = 0;

    clearReg();
    if (g_engine == this)
        g_engine = nullptr;
}

void JZNodeEngine::init()
{
    Q_ASSERT(!g_engine);

    // regist type
    m_program->registType();

    g_engine = this;
    QVariantList in, out;
    call("__init__", in,out);
}   

void JZNodeEngine::deinit()
{
    clear();    
}

void JZNodeEngine::statReport()
{
    m_stat.report();
}

JZFunctionDebugInfo *JZNodeEngine::currentFunctionDebugInfo()
{
    QString function = m_stack.currentEnv()->function->fullName();
    return m_script->functionDebug(function);
}

int JZNodeEngine::nodeIdByPc(JZNodeScript *script,QString function, int pc)
{
    int min_range = INT_MAX;
    int node_id = -1;

    for (int i = pc; i >= 0; i--)
    {
        if (script->statmentList[i]->type == OP_nodeId)
        {
            auto ir = dynamic_cast<JZNodeIRNodeId*>(script->statmentList[i].data());
            node_id = ir->id;
            break;
        }
    }
    
    Q_ASSERT_X(node_id != -1,qUtf8Printable(function),qUtf8Printable("pc = " + QString::number(pc)));
    return node_id;
}

NodeRange JZNodeEngine::nodeDebugRange(int node_id, int pc)
{
    auto &list = currentFunctionDebugInfo()->nodeInfo[node_id].pcRanges;
    for (int i = 0; i < list.size(); i++)
    {
        if (pc >= list[i].start && pc < list[i].end)
            return list[i];
    }
    Q_ASSERT(0);
    return NodeRange();
}

int JZNodeEngine::breakNodeId()
{
    if (m_breakNodeId != -1)
        return m_breakNodeId;
    else
        return nodeIdByPc(m_pc);
}

int JZNodeEngine::nodeIdByPc(int m_pc)
{
    QString func = m_stack.currentEnv()->function->fullName();
    return nodeIdByPc(m_script, func, m_pc);
}

int JZNodeEngine::status()
{
    QMutexLocker lock(&m_mutex);
    return m_status;
}

JZNodeRuntimeInfo JZNodeEngine::runtimeInfo()
{   
    JZNodeRuntimeInfo info;
    QMutexLocker lock(&m_mutex);
    info.status = m_status;
    if (m_status == Status_idlePause)
    {
        JZNodeRuntimeInfo::Stack s;
        s.file = "__idle__";
        info.stacks.push_back(s);
    }
    else
    {        
        for (int i = 0; i < m_stack.size(); i++)
        {
            JZNodeRuntimeInfo::Stack s;
            auto env = m_stack.env(i);
            s.function = env->function->fullName();
            if (env->script)
            {
                s.file = env->script->file;
                s.nodeId = nodeIdByPc(env->script, s.function, env->pc);
            }            
            s.pc = env->pc;
            info.stacks.push_back(s);
        }        
    }
    return info;
}

JZNodeRuntimeError JZNodeEngine::runtimeError()
{
    return m_error;
}

void JZNodeEngine::pushStack(const JZFunction *func)
{
    if (m_stack.size() > 0)    
        m_stack.currentEnv()->pc = m_pc;

    m_stack.push();    
    m_stack.currentEnv()->function = func;
    updateHook();
    if(!func->isCFunction())
    {                
        m_pc = func->addr;
        m_script = getScript(func->path);
        Q_ASSERT_X(m_script,"Error",qUtf8Printable(func->path));

        m_stack.currentEnv()->pc = m_pc;
        m_stack.currentEnv()->script = m_script;
        if (func->isMemberFunction())
            m_stack.currentEnv()->object = m_regs[Reg_CallIn - Reg_Start];        
    }
    else
    {
        m_pc = -1;
        m_script = nullptr;
    }
}

void JZNodeEngine::popStack()
{       
    m_stack.pop();
    updateHook();

    if(m_stack.size() > 0)
    {
        m_pc = m_stack.currentEnv()->pc;
        m_script = m_stack.currentEnv()->script;
        Q_ASSERT(m_pc == -1 || m_script);
    }
    else
    {
        m_pc = -1;
        m_script = nullptr;        
    }
}

void JZNodeEngine::customEvent(QEvent *qevent)
{
    if (qevent->type() == JZNodeEngineIdlePauseEvent::Event)
    {
        QVariantList in,out;
        call(&m_idleFunc, in, out);
    }
}

bool JZNodeEngine::checkIdlePause(const JZFunction *func)
{
    m_mutex.lock();    
    if (m_status == Status_none)
    {
        if (m_statusCommand == Command_pause)
        {
            m_statusCommand = Command_none;
            updateStatus(Status_idlePause);
            m_waitCond.wait(&m_mutex);

            int cmd = m_statusCommand;
            m_statusCommand = Command_none;                        
            if (cmd == Command_stop || func == &m_idleFunc)
            {
                updateStatus(Status_none);
                m_mutex.unlock();
                return true;
            }
        }   
        updateStatus(Status_running);
        m_mutex.unlock();
    }
    else if (m_status == Status_running) //嵌套事件,多次调用
    {
        m_mutex.unlock();
    }
    else if (m_status == Status_error)   //错误直接返回
    {
        m_mutex.unlock();
        return true;
    }
    return false;    
}

bool JZNodeEngine::call(const QString &name,const QVariantList &in,QVariantList &out)
{    
    const JZFunction *func = function(name,&in);
    return call(func,in,out);
}

bool JZNodeEngine::call(const JZFunction *func,const QVariantList &in,QVariantList &out)
{    
    if (checkIdlePause(func))
        return false;                    
    
    try
    {
        m_error = JZNodeRuntimeError();
        Q_ASSERT(func && in.size() == func->define.paramIn.size());
        for (int i = 0; i < in.size(); i++)
            setReg(Reg_CallIn + i,in[i]);

        if(func->isCFunction())
        {
            callCFunction(func);
        }
        else
        {            
            pushStack(func);            
            if(!run())
            {                
                updateStatus(Status_none);
                m_statusCommand = Command_none;                                                
                m_breakStep.clear();
                m_stack.clear();
                return false;
            }
        }

        out.clear();    
        for (int i = 0; i < func->define.paramOut.size(); i++)
            out.push_back(getReg(Reg_CallOut + i));    
        clearReg();
        
        if (m_stack.size() == 0)
        {
            updateStatus(Status_none);
            m_statusCommand = Command_none;        
        }
        return true;
    }
    catch(const std::exception& e)
    {           
        m_mutex.lock();
        m_stack.currentEnv()->pc = m_pc;                                
        m_statusCommand = Command_none;
        updateStatus(Status_error);
        m_mutex.unlock();

        JZNodeRuntimeError error;
        error.error = e.what();
        error.info = runtimeInfo();
        m_error = error;
        emit sigRuntimeError(error);        

        if (m_debug) //保留错误现场
        {
            m_mutex.lock();
            m_waitCond.wait(&m_mutex);
            m_statusCommand = Command_none;
            m_mutex.unlock();
        }
        return false;
    }
}

void JZNodeEngine::invoke(const QString &name,const QVariantList &in,QVariantList &out)
{
    if (status() == Status_none)
    {
        call(name, in, out);
        return;
    }

    const JZFunction *func = function(name,&in);
    Q_ASSERT(func && in.size() == func->define.paramIn.size());
    for (int i = 0; i < in.size(); i++)
        setReg(Reg_CallIn + i,in[i]);
    
    if(func->isCFunction())
    {
        callCFunction(func);
    }
    else
    {            
        pushStack(func);            
        if(!run())
            return;
    }

    out.clear();    
    for (int i = 0; i < func->define.paramOut.size(); i++)
        out.push_back(getReg(Reg_CallOut + i));
    clearReg();
}

void JZNodeEngine::onSlot(const QString &function,const QVariantList &in,QVariantList &out)
{
    m_sender = toJZObject(in[0]);
    invoke(function,in,out);
    m_sender = nullptr;
}

QVariant *JZNodeEngine::getParamRef(JZNodeIRParam &param)
{
    QVariant *ref = nullptr;
    if(param.isId())
    {
        if (param.id() >= Reg_Start)
            ref = getRegRef(param.id());
        else
            ref = m_stack.currentEnv()->getRef(param.id());
    }
    else if(param.isLiteral())    
        ref = (QVariant *)&param.value;
    else if(param.isRef())
        ref = getVariableRef(param.ref());
    else if(param.isThis())
        ref = &m_stack.currentEnv()->object;

    m_stack.currentEnv()->irParamCache[&param] = ref;
    param.cache = ref;
    return ref;
}

const QVariant &JZNodeEngine::getParam(JZNodeIRParam &param)
{       
    m_stat.getTime++;
    if(param.cache)
        return *param.cache;

    auto ref = getParamRef(param);
    Q_ASSERT(ref);
    return *ref;
}

void JZNodeEngine::setParam(JZNodeIRParam &param,const QVariant &value)
{
    m_stat.setTime++;
    Q_ASSERT(param.isId() || param.isRef());

    QVariant *ref = nullptr;
    if(param.cache)
        ref = param.cache;
    else
        ref = getParamRef(param);
        
    if(param.isReg())
        *ref = value;
    else
        dealSet(ref, value);
}

void JZNodeEngine::initGlobal(QString name, const QVariant &v)
{
	m_global[name] = QVariantPtr(new QVariant(v));
}

void JZNodeEngine::initLocal(QString name, const QVariant &v)
{
    auto env = m_stack.currentEnv();
    env->initVariable(name, v);
}

void JZNodeEngine::initLocal(int id, const QVariant &v)
{
    auto env = m_stack.currentEnv();
    env->initVariable(id, v);
}

void JZNodeEngine::clearReg()
{
    setReg(Reg_Cmp, false);
    for(int i = 0; i < 16; i++)
    {
        setReg(Reg_CallIn + i,QVariant());
        setReg(Reg_CallOut + i,QVariant());
    }
}

Stack *JZNodeEngine::stack()
{
    return &m_stack;
}

bool JZNodeEngine::callUnitTest(ScriptDepend *depend,QVariantList &out)
{
    auto toVariant = [this](const JZParamDefine &param,QVariant &v)->bool
    {
        if(param.dataType() >= Type_class)
        {
            auto obj = objectFromString(param.type, param.value);
            if(!obj)
                return false;
            v = QVariant::fromValue(JZNodeObjectPtr(obj,true));
        }
        else
        {
            v = JZNodeType::initValue(param.dataType(),param.value);
        }
        return true;
    };

    //init hook function
    m_dependHook.clear();
    for (int i = 0; i < depend->hook.size(); i++)
    {
        if(!depend->hook[i].enable)
            continue;

        QVariantList value_list;
        auto &hook_list = depend->hook[i].params;
        for(int i = 0; i < hook_list.size(); i++)
        {
            QVariant v; 
            if(!toVariant(hook_list[i],v))
                return false;
            
            value_list << v;
        }
        m_dependHook[depend->hook[i].pc] = value_list;
    }

    //global
    for (int i = 0; i < depend->global.size(); i++)
    {
        QVariant v; 
        if(!toVariant(depend->global[i],v))
            return false;
        
        setVariable(depend->global[i].name,v);
    }

    //init input
    QVariantList in;
    for (int i = 0; i < depend->function.paramIn.size(); i++)
    {
        auto &p = depend->function.paramIn[i];
        if(depend->function.isMemberFunction() && i == 0)
        {
            auto obj = JZNodeObjectManager::instance()->create(depend->function.className);
            JZNodeObjectPtr ptr(obj,true);
            in << QVariant::fromValue(ptr);

            for(int mem_idx = 0; mem_idx < depend->member.size(); i++)
            {   
                QVariant v; 
                if(!toVariant(depend->member[mem_idx],v))
                    return false;
                
                obj->setParam(depend->member[mem_idx].name,v);
            }
        }
        else
        {
            QVariant v; 
            if(!toVariant(p,v))
                return false;
            in << v;
        }
    }

    //call
    m_depend = depend; 
    bool ret = call(depend->function.fullName(),in,out);
    m_depend = nullptr;
    return ret;
}

void JZNodeEngine::splitMember(const QString &fullName, QStringList &objName,QString &memberName)
{
    QStringList list = fullName.split(".");
    if (list.size() > 1)
        objName = list.mid(0, list.size() - 1);
    
    memberName = list.back();
}

QVariant *JZNodeEngine::getVariableRefSingle(RunnerEnv *env, const QString &name)
{
    QVariant *obj = nullptr;
    if (env)
    {
        obj = env->getRef(name);
        if (obj)
            return obj;
    }

    auto it = m_global.find(name);
    if (it == m_global.end())
        return nullptr;

    return it->data();
}

QVariant *JZNodeEngine::getVariableRef(int id)
{
    return getVariableRef(id, -1);
}

QVariant *JZNodeEngine::getVariableRef(int id, int stack_level)
{
    Q_ASSERT(m_stack.size() > 0);
    
    auto env = (stack_level == -1) ? m_stack.currentEnv() : m_stack.env(stack_level);
    auto it = env->stacks.find(id);
    if (it == env->stacks.end())
        return nullptr;

    return it->data();
}

const QVariant &JZNodeEngine::getVariable(int id)
{
    QVariant *ref = getVariableRef(id);
    if (!ref)
        throw std::runtime_error("no such variable " + to_string(id));

    return *ref;
}

void JZNodeEngine::setVariable(int id, const QVariant &value)
{
    QVariant *ref = getVariableRef(id);
    if (!ref)
        throw std::runtime_error("no such variable " + to_string(id));

    dealSet(ref, value);
}

QVariant *JZNodeEngine::getVariableRef(const QString &name)
{
    return getVariableRef(name, -1);
}

QVariant *JZNodeEngine::getVariableRef(const QString &name, int stack_level)
{
    RunnerEnv *env = nullptr;
    if(m_stack.size() > 0)
        env = (stack_level == -1) ? m_stack.currentEnv() : m_stack.env(stack_level);

    if (name == "this")
        return env? &env->object : nullptr;

    QStringList obj_list;
    QString param_name;
    splitMember(name, obj_list, param_name);

    if (obj_list.size() > 0)
    {
        QVariant *ref = nullptr;
        if (obj_list[0] == "this")
            ref = env? &env->object : nullptr;
        else
            ref = getVariableRefSingle(env,obj_list[0]);

        if (!ref)
            return nullptr;

        JZNodeObject *obj = nullptr;
        for (int i = 1; i < obj_list.size(); i++)
        {
            obj = toJZObject(*ref);
            ref = obj->paramRef(obj_list[i]);
        }

        obj = toJZObject(*ref);
        return obj->paramRef(param_name);
    }
    else
    {
        return getVariableRefSingle(env, param_name);
    }
}

const QVariant &JZNodeEngine::getVariable(const QString &name)
{    
    QVariant *ref = getVariableRef(name);
    if(!ref)
        throw std::runtime_error("no such variable " + name.toStdString());

    if(JZNodeType::isPointer(*ref))
        return *JZNodeType::getPointer(*ref);

    return *ref;        
}

void JZNodeEngine::setVariable(const QString &name, const QVariant &value)
{    
    QVariant *ref = getVariableRef(name);
    if(!ref)
        throw std::runtime_error("no such variable " + name.toStdString());

    if(JZNodeType::isPointer(*ref))
    {
        ref = JZNodeType::getPointer(*ref);
    }

    dealSet(ref, value);
}

void JZNodeEngine::dealSet(QVariant *ref, const QVariant &value)
{
    Q_ASSERT(JZNodeType::isSameType(value,*ref));
    *ref = value;
}

const QVariant &JZNodeEngine::getThis()
{    
    return m_stack.currentEnv()->object;
}

QVariant JZNodeEngine::getSender()
{
    return QVariant::fromValue(m_sender);
}

void JZNodeEngine::print(const QString &log)
{
    emit sigLog(log);
}

void JZNodeEngine::printMemory()
{
    QString text;
    m_mutex.lock();

    text += "global:\n";
    auto g_it = m_global.begin();
    while(g_it != m_global.end())
    {
        auto *var = g_it->data();
        text += "  "  + JZNodeType::variantTypeName(*var) + " " + g_it.key() + "\n"; 
        g_it++;
    }

    text += "regs:\n";
    for(int i = 1; i < m_regs.size(); i++)
    {
        if(!m_regs[i].isNull())
            text += "  "  + JZNodeType::variantTypeName(m_regs[i]) + " Reg" + QString::number(i) + "\n"; 
    }
    m_mutex.unlock();

    qDebug().noquote() << text;
}

const QVariant &JZNodeEngine::getReg(int id)
{               
    auto *ref = getRegRef(id);
    if (!ref)
        throw std::runtime_error("no such reg " + to_string(id));

    return *ref;
}

QVariant *JZNodeEngine::getRegRef(int id)
{
    id = id - Reg_Start; 
    return &m_regs[id];    
}

void JZNodeEngine::setReg(int id, const QVariant &value)
{   
    id = id - Reg_Start; 
    m_regs[id] = value;
}

JZNodeScript *JZNodeEngine::getScript(QString path)
{
    return m_program->script(path);
}

void JZNodeEngine::watchNotify(int id)
{
    if (!m_debug || !m_script)
        return;

    qint64 cur = QDateTime::currentMSecsSinceEpoch();
    if (cur - m_watchTime < 50)
        return;
    m_watchTime = cur;

    for (int i = 0; i < m_script->watchList.size(); i++)
    {
        auto &w = m_script->watchList[i];
        if (w.source.contains(id))
        {
            //QString text = getVariable(id);
            //sigNodePropChanged(m_script->file, w.traget, text);
        }
    }
}

void JZNodeEngine::setDebug(bool flag)
{
    m_debug = flag;
}

void JZNodeEngine::addBreakPoint(QString filepath,int nodeId)
{
    QMutexLocker lock(&m_mutex);    
    for(int i = 0; i < m_breakPoints.size(); i++)
    {        
        if(m_breakPoints[i].file == filepath && m_breakPoints[i].nodeId == nodeId)
            return;
    }

    BreakPoint pt;
    pt.type = BreakPoint::nodeEnter;
    pt.nodeId = nodeId;
    pt.file = filepath;
    m_breakPoints.push_back(pt);

    auto script = m_program->script(filepath);
    for(int i = 0; i < script->statmentList.size(); i++)
    {
        auto ir = script->statmentList[i].data();
        if(ir->type == OP_nodeId)
        {
            JZNodeIRNodeId *ir_id = dynamic_cast<JZNodeIRNodeId*>(ir);
            if(ir_id->id == nodeId)
                ir_id->isBreakPoint = true;
        }
    }    
}

void JZNodeEngine::removeBreakPoint(QString filepath,int nodeId)
{
    QMutexLocker lock(&m_mutex);
    int idx = indexOfBreakPoint(filepath,nodeId);
    if(idx == -1)
        return;
    
    auto script = m_program->script(filepath);
    for(int i = 0; i < script->statmentList.size(); i++)
    {
        auto ir = script->statmentList[i].data();
        if(ir->type == OP_nodeId)
        {
            JZNodeIRNodeId *ir_id = dynamic_cast<JZNodeIRNodeId*>(ir);
            if(ir_id->id == nodeId)
                ir_id->isBreakPoint = false;
        }
    }
}

int JZNodeEngine::indexOfBreakPoint(QString filepath,int nodeId)
{
    for(int i = 0; i < m_breakPoints.size(); i++)
    {
        if(m_breakPoints[i].file == filepath && m_breakPoints[i].nodeId == nodeId)
            return i;
    }
    return -1;
}

void JZNodeEngine::waitCommand()
{
    while(true)
    {   
        {
            QMutexLocker locker(&m_mutex);
            if(m_statusCommand == Command_none)
                return;
        }
        QThread::msleep(10);
    }
}

void JZNodeEngine::clearBreakPoint()
{
    for(int i = 0; i < m_breakPoints.size(); i++)
        removeBreakPoint(m_breakPoints[i].file,m_breakPoints[i].nodeId);
    
    QMutexLocker lock(&m_mutex);
    m_breakPoints.clear();
}

void JZNodeEngine::pause()
{
    QMutexLocker lock(&m_mutex);
    if (m_status != Status_none && m_status != Status_running)
        return;

    if (m_status == Status_none) 
    {
        auto *event = new JZNodeEngineIdlePauseEvent();
        qApp->postEvent(this, event);
    } 
    
    m_statusCommand = Command_pause;
    lock.unlock();
    waitCommand();
}

void JZNodeEngine::resume()
{
    QMutexLocker lock(&m_mutex);
    if (m_status != Status_pause && m_status != Status_idlePause)
        return;    
    
    m_statusCommand = Command_resume;
    lock.unlock();
    m_waitCond.wakeOne();
    waitCommand();
}

void JZNodeEngine::stop()
{
    QMutexLocker lock(&m_mutex);
    if(m_status == Status_none)
        return;
        
    m_statusCommand = Command_stop;
    lock.unlock();
    if(m_status == Status_idlePause || m_status == Status_pause)
        m_waitCond.wakeOne();
    waitCommand();
}

void JZNodeEngine::stepIn()
{
    QMutexLocker lock(&m_mutex);
    if (m_status != Status_pause)
        return;
    
    int node_id = breakNodeId();
    auto info = currentFunctionDebugInfo()->nodeInfo[node_id];
    if(info.node_type == Node_function)
    {
        m_breakStep.type = BreakPoint::stackEqual;
        m_breakStep.stack = m_stack.size() + 1;
        m_breakStep.nodeId = node_id;

        m_statusCommand = Command_resume;
        lock.unlock();
        m_waitCond.wakeOne();
        waitCommand();
    }
    else
    {
        lock.unlock();
        stepOver();        
    }    
}

void JZNodeEngine::stepOver()
{
    QMutexLocker lock(&m_mutex);  
    if (m_status != Status_pause)
        return;
    
    m_breakStep.type = BreakPoint::stepOver;
    m_breakStep.file = m_script->file;
    m_breakStep.nodeId = breakNodeId();
    m_breakStep.stack = m_stack.size();
    
    m_statusCommand = Command_resume;
    lock.unlock();
    m_waitCond.wakeOne();
    waitCommand();
}

void JZNodeEngine::stepOut()
{
    QMutexLocker lock(&m_mutex);
    if (m_status != Status_pause)
        return;

    m_breakStep.type = BreakPoint::stackEqual;
    m_breakStep.nodeId = breakNodeId();
    m_breakStep.stack = m_stack.size() - 1;

    m_statusCommand = Command_resume;
    lock.unlock();
    m_waitCond.wakeOne();
    waitCommand();
}

void JZNodeEngine::checkFunctionIn(const JZFunction *func)
{
    // get input
    auto &inList = func->define.paramIn;    
    for (int i = 0; i < inList.size(); i++)
    {
        const QVariant &v = getReg(Reg_CallIn + i);
        Q_ASSERT(JZNodeType::isSameType(JZNodeType::variantType(v),inList[i].dataType()));        
        if (i == 0 && func->isMemberFunction() && (v.type() != QVariant::String && JZNodeType::isNullObject(v)))
            throw std::runtime_error("object is nullptr");
    }
}

void JZNodeEngine::checkFunctionOut(const JZFunction *func)
{
    auto &outList = func->define.paramOut;
    for (int i = 0; i < outList.size(); i++)
    {        
        Q_ASSERT(JZNodeType::isSameType(JZNodeType::variantType(getReg(Reg_CallOut + i)),outList[i].dataType()));
    }
}

void JZNodeEngine::callCFunction(const JZFunction *func)
{    
    if(func->builtIn)
    {
        func->builtIn->call(this);
        return;
    }

    checkFunctionIn(func);

    QVariantList paramIn, paramOut;
    // get input
    auto &inList = func->define.paramIn;
    for (int i = 0; i < inList.size(); i++)    
        paramIn.push_back(getReg(Reg_CallIn + i));
    
    // call function
    pushStack(func);
    func->cfunc->call(paramIn,paramOut);    

    // set output
    auto &outList = func->define.paramOut;
    for (int i = 0; i < outList.size(); i++)
        setReg(Reg_CallOut + i,paramOut[i]);

    checkFunctionOut(func);
    popStack();
}

const JZFunction *JZNodeEngine::function(QString name,const QVariantList *list)
{
    auto func_ptr = JZNodeFunctionManager::instance()->functionImpl(name);
    if(!func_ptr)
        return nullptr;
    if(!func_ptr->isVirtualFunction())
        return func_ptr;
        
    QVariant v;
    if(list)  
        v = list->at(0);
    else
        v = getReg(Reg_CallIn);

    if (!isJZObject(v))
        return func_ptr;
    
    JZNodeObject *obj = toJZObject(v);
    int idx = name.indexOf(".");
    QString func_name = name.mid(idx + 1);
    auto func = obj->function(func_name);
    Q_ASSERT_X(func,"Error",qUtf8Printable("no function " + func_name));
    return JZNodeFunctionManager::instance()->functionImpl(func->fullName());
}

const JZFunction *JZNodeEngine::function(JZNodeIRCall *ir_call)
{
    //if(ir_call->cache)
    //    return ir_call->cache;

    auto func = function(ir_call->function,nullptr);
    if(!func->isVirtualFunction())
        ir_call->cache = func;

    return func;
}

void JZNodeEngine::unSupportSingleOp(int a, int op)
{
    QString error = QString("不支持的操作,操作符%1,数据类型%2").arg(JZNodeType::opName(op),
        JZNodeType::typeToName(a));

    Q_ASSERT_X(0,"unSupportOp:",qUtf8Printable(error));
}

void JZNodeEngine::unSupportOp(int a, int b, int op)
{    
    QString error = QString("操作符%1,数据类型%2,%3").arg(JZNodeType::opName(op), 
        JZNodeType::typeToName(a), JZNodeType::typeToName(b));

    Q_ASSERT_X(0,"unSupportOp:",qUtf8Printable(error));
}

QVariant JZNodeEngine::dealExprInt(const QVariant &va, const QVariant &vb, int op)
{
    int a = va.toInt();
    int b = vb.toInt();
    switch (op)
    {
    case OP_add:
        return a + b;
    case OP_sub:
        return a - b;
    case OP_mul:
        return a * b;
    case OP_div:
    {
        if(b == 0)
            throw std::runtime_error("divide zero");
        return a / b;
    }
    case OP_mod:
        return a % b;
    case OP_eq:
        return a == b;
    case OP_ne:
        return a != b;
    case OP_le:
        return a <= b;
    case OP_ge:
        return a >= b;
    case OP_lt:
        return a < b;
    case OP_gt:
        return a > b;
    case OP_and:
        return a && b;
    case OP_or:
        return a || b;
    case OP_not:
        return !a;
    case OP_bitand:
        return a & b;
    case OP_bitor:
        return a | b;
    case OP_bitxor:
        return a ^ b;
    default:
        unSupportOp(Type_int, Type_int,op);
        break;
    }
    return QVariant();
}

QVariant JZNodeEngine::dealExprInt64(const QVariant &va, const QVariant &vb, int op)
{
    qint64 a = va.toLongLong();
    qint64 b = vb.toLongLong();
    switch (op)
    {
    case OP_add:
        return a + b;
    case OP_sub:
        return a - b;
    case OP_mul:
        return a * b;
    case OP_div:
    {
        if(b == 0)
            throw std::runtime_error("divide zero");
        return a / b;
    }
    case OP_mod:
        return a % b;
    case OP_eq:
        return a == b;
    case OP_ne:
        return a != b;
    case OP_le:
        return a <= b;
    case OP_ge:
        return a >= b;
    case OP_lt:
        return a < b;
    case OP_gt:
        return a > b;
    case OP_and:
        return a && b;
    case OP_or:
        return a || b;
    case OP_not:
        return !a;
    case OP_bitand:
        return a & b;
    case OP_bitor:
        return a | b;
    case OP_bitxor:
        return a ^ b;
    default:
        unSupportOp(Type_int, Type_int,op);
        break;
    }
    return QVariant();
}

QVariant JZNodeEngine::dealExprDouble(const QVariant &va, const QVariant &vb, int op)
{
    double a = va.toDouble();
    double b = vb.toDouble();
    switch (op)
    {
    case OP_add:
        return a + b;
    case OP_sub:
        return a - b;
    case OP_mul:
        return a * b;
    case OP_div:
        return a / b;
    case OP_mod:
        return fmod(a, b);
    case OP_eq:
        return a == b;
    case OP_ne:
        return a != b;
    case OP_not:
        return !a;
    case OP_le:
        return a <= b;
    case OP_ge:
        return a >= b;
    case OP_lt:
        return a < b;
    case OP_gt:
        return a > b;
    default:
        unSupportOp(Type_double, Type_double, op);
        break;
    }
    return QVariant();
}

QVariant JZNodeEngine::dealExpr(const QVariant &a, const QVariant &b,int op)
{   
    int dataType1 = JZNodeType::variantType(a);
    int dataType2 = JZNodeType::variantType(b);
    if(dataType1 == Type_string && dataType2 == Type_string)
    {
        QString str_a = a.toString();
        QString str_b = b.toString();

        switch (op)
        {
            case OP_add:
                return str_a + str_b;
            case OP_eq:
                return str_a == str_b;
            case OP_ne:
                return str_a != str_b;
            case OP_le:
                return str_a <= str_b;
            case OP_ge:
                return str_a >= str_b;
            case OP_lt:
                return str_a < str_b;
            case OP_gt:
                return str_a > str_b;
            default:
                unSupportOp(dataType1, dataType2, op);
                break;
        }
    }
    else if((dataType1 >= Type_bool && dataType1 <= Type_int64)
            && (dataType2 >= Type_bool && dataType2 <= Type_int64))
    {
        if(dataType1 == Type_int64 || dataType2 == Type_int64)
            return dealExprInt64(a, b, op);
        else
            return dealExprInt(a, b, op);
    }
    else if(JZNodeType::isNumber(dataType1) && JZNodeType::isNumber(dataType2))
    {
        return dealExprDouble(a, b,op);
    }
    else
    {
        if (op == OP_eq || op == OP_ne)
        {
            if (dataType1 == dataType2 && JZNodeType::isObject(dataType1))
            {
                bool ret = (toJZObject(a) == toJZObject(b));
                if (op == OP_eq)
                    return ret;
                else
                    return !ret;
            }
        } 
        unSupportOp(dataType1, dataType2, op);
    }
    return QVariant();
}

QVariant JZNodeEngine::dealSingleExpr(const QVariant &a, int op)
{
    int dataType = JZNodeType::variantType(a);
    if (op == OP_not)
    {
        if (dataType == Type_bool)
            return !a.toBool();
        else
            unSupportSingleOp(dataType, op);
    }
    else if (op == OP_bitresver)
    {
        if (dataType == Type_int)
            return ~(a.toInt());
        else
            unSupportSingleOp(dataType, op);
    }
    return QVariant();
}

// check stop,pause
bool JZNodeEngine::checkPause(int node_id)
{
    if(m_statusCommand == Command_pause)
        return true;
    else
    {                 
        int stack = m_stack.size();
        if (m_breakStep.type == BreakPoint::stepOver)
        {                
            if (stack < m_breakStep.stack)
                return true;
            else if (m_breakStep.file != m_script->file)
                return true;
            else if (m_breakStep.stack == stack)
            {                    
                return (m_breakStep.nodeId != node_id);
            }
            return false;
        }
        else if (m_breakStep.type == BreakPoint::stackEqual)
        {
            return (stack == m_breakStep.stack);
        }
        else
        {
            return false;
        }   
    }

    return false;
}

bool JZNodeEngine::breakPointTrigger(int node_id)
{
    m_mutex.lock();
    m_breakNodeId = node_id;
    m_breakStep.clear();
    m_stack.currentEnv()->pc = m_pc;
    m_statusCommand = Command_none;
    updateStatus(Status_pause);
    m_waitCond.wait(&m_mutex);
    m_breakNodeId = -1;

    int cmd = m_statusCommand;
    if (m_statusCommand == Command_resume) //stop 等到最外层设置
    {
        updateStatus(Status_running);
        m_statusCommand = Command_none;
    }
    m_mutex.unlock();
    if (cmd == Command_stop)
        return true;

    return false;
}

void JZNodeEngine::updateStatus(int status)
{
    Q_ASSERT((m_status == Status_none && (status == Status_running || status == Status_idlePause))
        || (m_status == Status_running && (status == Status_none || status == Status_pause || status == Status_error))
        || (m_status == Status_pause && (status == Status_none || status == Status_running))
        || (m_status == Status_idlePause && (status == Status_none))
        || (m_status == Status_pause && (status == Status_none)));        
    if (m_status != status)
    {
        m_status = status;
        sigStatusChanged(m_status);
    }
}

void JZNodeEngine::updateHook()
{
    if(m_stack.size() > 0)
        m_hookEnable = (m_depend && m_depend->function.fullName() == m_stack.currentEnv()->function->fullName());
    else
        m_hookEnable = false;
}

bool JZNodeEngine::run()
{    
    updateHook();

    int in_stack_size = m_stack.size();
    while (true)
    {                   
        if(m_statusCommand == Command_stop)
            return false;

        m_stat.statmentTime++;

        auto &op_list = m_script->statmentList;
        JZNodeIR *op = op_list[m_pc].data();
        switch (op->type)
        {
        case OP_nodeId:
        {
            if(m_debug)
            {
                JZNodeIRNodeId *ir_id =  dynamic_cast<JZNodeIRNodeId*>(op);
                if(ir_id->isBreakPoint || checkPause(ir_id->id))
                {
                    if(breakPointTrigger(ir_id->id))
                        return false;
                }
            }
            break;
        }
        case OP_nop:
            break;
        case OP_add:
        case OP_sub:
        case OP_mul:
        case OP_div:
        case OP_mod:
        case OP_eq:
        case OP_ne:
        case OP_le:
        case OP_ge:
        case OP_lt:
        case OP_gt:
        case OP_and:
        case OP_or:
        case OP_bitand:
        case OP_bitor:
        case OP_bitxor:
        {
            m_stat.exprTime++;

            JZNodeIRExpr *ir_expr =  dynamic_cast<JZNodeIRExpr*>(op);
            QVariant c;
            auto &a = getParam(ir_expr->src1);
            auto &b = getParam(ir_expr->src2);
            c = dealExpr(a,b,ir_expr->type);
            setParam(ir_expr->dst,c);
            break; 
        }
        case OP_not:
        case OP_bitresver:
        {
            m_stat.exprTime++;

            JZNodeIRExpr *ir_expr = dynamic_cast<JZNodeIRExpr*>(op);
            QVariant c;
            auto &a = getParam(ir_expr->src1);
            c = dealSingleExpr(a, ir_expr->type);
            setParam(ir_expr->dst, c);
            break;
        }
        case OP_jmp:
        case OP_je:
        case OP_jne:
        {
            JZNodeIRJmp *ir_jmp = (JZNodeIRJmp*)op;
            int jmpPc = ir_jmp->jmpPc;
            Q_ASSERT(jmpPc >= 0 && jmpPc < op_list.size());
            if(op->type == OP_jmp)
                m_pc = jmpPc;
            else
            {
                bool flag = getReg(Reg_Cmp).toBool();
                if(op->type == OP_je)
                    m_pc = flag? jmpPc : m_pc+1;
                else
                    m_pc = flag? m_pc+1 : jmpPc;
            }
            continue;
        }
        case OP_alloc:
        {
            JZNodeIRAlloc *ir_alloc = (JZNodeIRAlloc*)op;
            auto value = JZNodeType::defaultValue(ir_alloc->dataType);
            if(ir_alloc->allocType == JZNodeIRAlloc::Heap)
                initGlobal(ir_alloc->name,value);
            else if (ir_alloc->allocType == JZNodeIRAlloc::Stack)
                initLocal(ir_alloc->name, value);
            else
                initLocal(ir_alloc->id, value);            
            break;
        }
        case OP_clearReg:
        {
            clearReg();
            break;
        }
        case OP_set:
        {
            JZNodeIRSet *ir_set = (JZNodeIRSet*)op;
            setParam(ir_set->dst,getParam(ir_set->src));
            break;
        }
        case OP_convert:
        {
            JZNodeIRConvert *ir_convert = (JZNodeIRConvert*)op;
            QVariant ret = JZNodeType::convertTo(ir_convert->dstType,getParam(ir_convert->src));
            setParam(ir_convert->dst,ret);
            break;   
        }
        case OP_call:
        {           
            m_stat.callTime++;

            JZNodeIRCall *ir_call = (JZNodeIRCall*)op;            
            const JZFunction *func = function(ir_call);
            Q_ASSERT(func);            

            if(m_hookEnable && m_dependHook.contains(m_pc))
            {
                auto &hook_list = m_dependHook[m_pc];
                for(int i = 0; i < hook_list.size(); i++)
                    setReg(Reg_CallOut + i, hook_list[i]);
            }
            else
            {
                if(func->isCFunction())
                    callCFunction(func);
                else
                {
                    checkFunctionIn(func);
                    pushStack(func);
                    continue;
                }
            }
            break;
        }
        case OP_return:
        {            
            checkFunctionOut(m_stack.currentEnv()->function);
            popStack();                  
            if(m_stack.size() < in_stack_size)
                goto RunEnd;
            break;
        }
        case OP_exit:
            goto RunEnd;
        case OP_assert:
        {            
            if (!getReg(Reg_Cmp).toBool())
            {
                JZNodeIRAssert *ir_assert = (JZNodeIRAssert*)op;
                QString tips = getParam(ir_assert->tips).toString();
                throw std::runtime_error(qUtf8Printable(tips));
            }
            break;
        }
        default:
            Q_ASSERT(0);
            break;
        }
        m_pc++;
    }

RunEnd:        
    return true;
}
