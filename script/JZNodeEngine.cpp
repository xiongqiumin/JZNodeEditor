#include "JZNodeEngine.h"
#include "JZNodeFunctionManager.h"
#include "JZEvent.h"
#include <QPushButton>
#include <QApplication>
#include <math.h>
#include <QDateTime>
#include <QTimer>
#include "JZNodeVM.h"
#include "JZNodeBind.h"
#include "JZNodeObjectParser.h"
#include "JZContainer.h"
#include "JZNodeParamDelegate.h"

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
    else if(op == OP_lt)
    {
        if((first < last && step > 0) || (first >= last))
            return true;
    }
    else if(op == OP_le)
    {
        if((first <= last && step > 0) || (first > last))
            return true;
    }
    else if(op == OP_gt)
    {
        if((first > last && step < 0) || (first <= last))
            return true;
    }
    else if(op == OP_ge)
    {
        if((first >= last && step < 0) || (first < last))
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

class JZCreate: public BuiltInFunction
{
public:
    virtual void call(JZNodeEngine *engine) override
    {
        QString type = engine->getReg(Reg_CallIn).toString();    
        JZNodeObject *obj = obj_inst->create(type);
        engine->setReg(Reg_CallOut,QVariant::fromValue(JZNodeObjectPtr(obj,true)));
    }
};

class JZClone: public BuiltInFunction
{
public:
    virtual void call(JZNodeEngine *engine) override
    {
        engine->setReg(Reg_CallOut,JZNodeType::clone(engine->getReg(Reg_CallIn)));
    }
};

class JZPrint: public BuiltInFunction
{
public:
    virtual void call(JZNodeEngine *engine) override
    {
        QStringList list;
        int count = engine->regInCount();
        for (int i = 0; i < count; i++)
        {
            list << JZNodeType::debugString(engine->getReg(Reg_CallIn + i));
        }
        engine->print(list.join(" "));
    }
};

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
    printNode = INVALID_ID;
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
bool JZNodeRuntimeError::isError()
{
    return !error.isEmpty();
}

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

//BreakStep
BreakStep::BreakStep()
{    
    clear();        
}

void BreakStep::clear()
{    
    type = BreakStep::none;
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
    auto func_inst = m_env.functionManager();
    JZNodeEngineIdlePauseEvent::Event = QEvent::registerEventType();

    JZFunctionDefine print;
    print.name = "print";
    print.isCFunction = true;
    print.isFlowFunction = true;    
    print.paramIn.push_back(JZParamDefine("args", Type_args));
    auto print_func = BuiltInFunctionPtr(new JZPrint());
    func_inst->registBuiltInFunction(print, print_func);

    JZFunctionDefine create;
    create.name = "createObject";
    create.isCFunction = true;
    create.isFlowFunction = false;    
    create.paramIn.push_back(JZParamDefine("type", Type_string));
    create.paramOut.push_back(JZParamDefine("arg", Type_arg));
    auto create_func = BuiltInFunctionPtr(new JZCreate());
    func_inst->registBuiltInFunction(create, create_func);
 
    JZFunctionDefine clone;
    clone.name = "clone";
    clone.isCFunction = true;
    clone.isFlowFunction = false;    
    clone.paramIn.push_back(JZParamDefine("in", Type_arg));
    clone.paramOut.push_back(JZParamDefine("out", Type_arg));
    auto clone_func = BuiltInFunctionPtr(new JZClone());
    func_inst->registBuiltInFunction(clone, clone_func);

    func_inst->registCFunction("connect", true, jzbind::createFuncion(QObjectConnect));
    func_inst->registCFunction("disconnect", true, jzbind::createFuncion(QObjectDisconnect));
    func_inst->registCFunction("forRuntimeCheck", true, jzbind::createFuncion(JZForRuntimeCheck));    
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
    m_regInCount = 0;

    m_watchTimer = new QTimer(this);
    connect(m_watchTimer, &QTimer::timeout, this, &JZNodeEngine::onWatchTimer);
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

JZScriptEnvironment *JZNodeEngine::environment()
{
    return &m_env;
}

void JZNodeEngine::clear()
{    
    m_watchTimer->stop();
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
    m_regInCount = 0;

    clearReg();
    if (g_engine == this)
        g_engine = nullptr;
}

void JZNodeEngine::init()
{
    Q_ASSERT(!g_engine);

    // regist type
    m_env->registType(m_program->typeMeta());
    auto script_list = m_program->scriptList();
    for(int i = 0; i < script_list.size(); i++)
    {
        auto &func_list = script_list[i]->functionList;
        for (int func_idx = 0; func_idx < func_list.size(); func_idx++)
            m_env.functionManager()->registFunctionImpl(func_list[func_idx]);    
    }

    g_engine = this;
    QVariantList in, out;
    call("__init__", in,out);

    if(m_debug)
        m_watchTimer->start(50);
}   

void JZNodeEngine::deinit()
{
    clear();    
}

void JZNodeEngine::statClear()
{
    m_stat.clear();
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

int JZNodeEngine::nodeIdByPc(int pc)
{
    QString func = m_stack.currentEnv()->function->fullName();
    return nodeIdByPc(m_script, func, pc);
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

    checkFunctionIn(func);     
    
    if(!func->isCFunction())
    {          
        updateHook();

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
    checkFunctionOut(m_stack.currentEnv()->function);
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
    if (m_error.isError())
        return false;                
    
    try
    {
        m_error = JZNodeRuntimeError();
        Q_ASSERT(func && (func->define.isVariadicFunction() || in.size() == func->define.paramIn.size()));
        for (int i = 0; i < in.size(); i++)
            setReg(Reg_CallIn + i,in[i]);
        m_regInCount = in.size();
        
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
    Q_ASSERT(func && (func->define.isVariadicFunction() || in.size() == func->define.paramIn.size()));
    for (int i = 0; i < in.size(); i++)
        setReg(Reg_CallIn + i,in[i]);
    m_regInCount = in.size();

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

QVariant *JZNodeEngine::getParamRef(int stack_level,const JZNodeIRParam &param)
{
    QVariant *ref = nullptr;
    if (m_stack.size() == 0)
    {
        auto it = m_global.find(param.ref());
        if (it != m_global.end())
            ref = it->data();
    }
    else
    {
        RunnerEnv *env = (stack_level == -1) ? m_stack.currentEnv() : m_stack.env(stack_level);
        if (param.isStack())
            ref = env->getRef(param.id());
        else if (param.isThis())
            ref = &env->object;
        else
        {
            ref = env->getRef(param.ref());
            if (!ref)
            {
                auto it = m_global.find(param.ref());
                if (it != m_global.end())
                    ref = it->data();
            }
        }
    }
    Q_ASSERT(ref);
    return ref;
}

QVariant JZNodeEngine::getParam(int stack_level, const JZNodeIRParam &param)
{
    m_stat.getTime++;

    if (param.isReg())
        return getReg(param.id());
    else if (param.isLiteral())
        return param.literal();
    else    
    {                
        auto ref = getParamRef(stack_level,param);
        if (param.member.isEmpty())
            return *ref;
        else
        {
            QStringList obj_list;
            QString param_name;
            splitMember(param.member, obj_list, param_name);

            JZNodeObject *obj = getVariableObject(ref, obj_list);
            return obj->param(param_name);
        }
    }
}

void JZNodeEngine::setParam(int stack_level, const JZNodeIRParam &param, const QVariant &value)
{
    m_stat.setTime++;    

    if (param.isReg())
        setReg(param.id(), value);
    else
    {
        auto ref = getParamRef(stack_level, param);
        if (param.member.isEmpty())
            dealSet(ref, value);
        else
        {
            QStringList obj_list;
            QString param_name;
            splitMember(param.member, obj_list, param_name);

            JZNodeObject *obj = getVariableObject(ref, obj_list);
            return obj->setParam(param_name,value);
        }
    }
}

QVariant JZNodeEngine::getParam(const JZNodeIRParam &param)
{       
    return getParam(-1, param);
}

void JZNodeEngine::setParam(const JZNodeIRParam &param,const QVariant &value)
{
    setParam(-1, param, value);
}

QVariant JZNodeEngine::getVariable(const QString &name)
{
    return getParam(irRef(name));
}

void JZNodeEngine::setVariable(const QString &name, const QVariant &value)
{
    setParam(irRef(name),value);
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
    m_regInCount = 0;
}

Stack *JZNodeEngine::stack()
{
    return &m_stack;
}

bool JZNodeEngine::callUnitTest(ScriptDepend *depend,QVariantList &out)
{
    m_depend = depend;
    obj_inst->setUnitTest(true);

    //init hook function
    m_dependHook.clear();
    for (int hook_idx = 0; hook_idx < depend->hook.size(); hook_idx++)
    {
        auto &hook = depend->hook[hook_idx]; 
        if(!hook.enable)
            continue;

        auto func = m_env.functionManager()->function(hook.function);

        QVariantList value_list;
        auto &hook_list = hook.params;
        for(int i = 0; i < hook_list.size(); i++)
        {
            QVariant v = createVariable(func->paramOut[i].dataType(), hook_list[i]);
            
            value_list << v;
        }
        
        m_dependHook[hook.pc] = value_list;
    }

    //global
    auto it = depend->global.begin();
    while(it != depend->global.end())
    {
        auto ptr = m_global[it.key()];
        int data_type = JZNodeType::variantType(*ptr);
        *ptr = createVariable(data_type,it.value());       
        it++;
    }

    //init input
    QVariantList in;
    for (int i = 0; i < depend->function.paramIn.size(); i++)
    {
        auto &p = depend->function.paramIn[i];
        if(depend->function.isMemberFunction() && i == 0)
        {
            auto obj = obj_inst->create(depend->function.className);
            JZNodeObjectPtr ptr(obj,true);
            in << QVariant::fromValue(ptr);

            auto mem_it = depend->member.begin();
            while (mem_it != depend->member.end())
            {   
                auto param_def = obj->meta()->param(it.key());
                auto v = createVariable(param_def->dataType(), mem_it.value());
                obj->setParam(it.key(),v);
                mem_it++;
            }
        }
        else
        {
            auto d = JZNodeParamDelegateManager::instance()->delegate(p.dataType());

            QVariant v;
            if(d && d->createParam)
                v = d->createParam(p.value);
            else
                v = createVariable(p.dataType(), p.value);

            in << v;
        }
    }

    //call    
    bool ret = call(depend->function.fullName(),in,out);
    obj_inst->setUnitTest(false);
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

QVariant JZNodeEngine::createVariable(int type,const QString &value)
{
    auto inst = obj_inst;

    QVariant v;
    if(type < Type_class)
        v = m_env->initValue(type, value);
    else
    {        
        JZNodeObject *sub = nullptr; 
        if (value.isEmpty())
        {
            auto def = inst->meta(type);
            if(def->isValueType())
                sub = inst->create(type);
            else
                sub = inst->createNull(type);
        }
        else if (value == "null")
        {
            sub = inst->createNull(type);
        }
        else if(value.startsWith("{") && value.endsWith("}"))
        {
            QString init_text = value.mid(1,value.size() - 2);
            if(init_text.isEmpty())
                sub = inst->create(type); 
            else
                sub = objectFromString(type, init_text);
        }
        Q_ASSERT(sub);
        v = QVariant::fromValue(JZNodeObjectPtr(sub,true));
    }
    return v;
}

JZNodeObject *JZNodeEngine::getVariableObject(QVariant *ref, const QStringList &obj_list)
{        
    JZNodeObject *obj = toJZObject(*ref);
    Q_ASSERT(obj);
    if(obj->isNull())
        throw std::runtime_error("object is nullptr");

    for (int i = 0; i < obj_list.size(); i++)
    {
        obj = toJZObject(obj->param(obj_list[i]));
        Q_ASSERT(obj);
        if (obj->isNull())
            throw std::runtime_error("object is nullptr");
    }

    return obj;
}

void JZNodeEngine::dealSet(QVariant *ref, const QVariant &value)
{
    Q_ASSERT(m_env->isSameType(value,*ref));
    *ref = value;
}

QVariant JZNodeEngine::getSender()
{
    return QVariant::fromValue(m_sender);
}

void JZNodeEngine::print(const QString &log)
{
    qDebug() << log;
    emit sigLog(log);
}

int JZNodeEngine::regInCount()
{
    return m_regInCount;
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
        text += "  "  + m_env.variantTypeName(*var) + " " + g_it.key() + "\n"; 
        g_it++;
    }

    text += "regs:\n";
    for(int i = 1; i < m_regs.size(); i++)
    {
        if(!m_regs[i].isNull())
            text += "  "  + m_env.variantTypeName(m_regs[i]) + " Reg" + QString::number(i) + "\n"; 
    }
    m_mutex.unlock();

    qDebug().noquote() << text;
}

const QVariant &JZNodeEngine::getReg(int id)
{   
    id = id - Reg_Start;    
    return m_regs[id];
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

void JZNodeEngine::watchNotify()
{
    if (!m_debug || m_stack.size() == 0)
        return;

    if(m_stack.currentEnv()->watchMap.size() == 0)
        return;

    emit sigWatchNotify();
    m_stack.currentEnv()->watchMap.clear();
}

void JZNodeEngine::printNode()
{
    auto env = m_stack.currentEnv();
    int node_id = env->printNode; 
    if(node_id == INVALID_ID)
        return;

    auto info = currentFunctionDebugInfo();
    auto &node_info = info->nodeInfo[node_id];

    QString line = node_info.name + "(id=" + QString::number(node_info.id);
    if(node_info.paramIn.size() > 0)
        line += ",";
    for(int i = 0; i < node_info.paramIn.size(); i++)
    {
        QString name = node_info.paramIn[i].define.name;
        int param_id = JZNodeGemo::paramId(node_info.id, node_info.paramIn[i].id);
        auto ref = env->getRef(param_id);
        line += name + " " + JZNodeType::debugString(*ref);
    }

    if(node_info.paramIn.size() > 0 && node_info.paramOut.size() > 0)
        line += "->";
    for(int i = 0; i < node_info.paramOut.size(); i++)
    {
        QString name = node_info.paramOut[i].define.name;
        int param_id = JZNodeGemo::paramId(node_info.id, node_info.paramOut[i].id);
        auto ref = env->getRef(param_id);
        line += name + " " + JZNodeType::debugString(*ref);
    }
    line += ")";
    print(line);
    m_stack.currentEnv()->printNode = INVALID_ID;
}

void JZNodeEngine::onWatchTimer()
{
    watchNotify();
}

void JZNodeEngine::setDebug(bool flag)
{
    m_debug = flag;
}

void JZNodeEngine::addBreakPoint(QString filepath,int nodeId)
{
    BreakPoint pt;
    pt.type = BreakPoint::nodeEnter;
    pt.nodeId = nodeId;
    pt.file = filepath;

    addBreakPoint(pt);
}

void JZNodeEngine::addBreakPoint(const BreakPoint &pt)
{
    QMutexLocker lock(&m_mutex);    
    int idx = indexOfBreakPoint(pt.file,pt.nodeId);
    if(idx != -1)
    {
        m_breakPoints[idx] = pt;
        return;
    }
    m_breakPoints.push_back(pt);

    auto script = m_program->script(pt.file);
    for(int i = 0; i < script->statmentList.size(); i++)
    {
        auto ir = script->statmentList[i].data();
        if(ir->type == OP_nodeId)
        {
            JZNodeIRNodeId *ir_id = dynamic_cast<JZNodeIRNodeId*>(ir);
            if(ir_id->id == pt.nodeId)
                ir_id->breakPointType = pt.type;
        }
    }
}

void JZNodeEngine::removeBreakPoint(QString filepath,int nodeId)
{
    QMutexLocker lock(&m_mutex);
    int idx = indexOfBreakPoint(filepath,nodeId);
    if(idx == -1)
        return;
    m_breakPoints.removeAt(idx);
    
    auto script = m_program->script(filepath);
    for(int i = 0; i < script->statmentList.size(); i++)
    {
        auto ir = script->statmentList[i].data();
        if(ir->type == OP_nodeId)
        {
            JZNodeIRNodeId *ir_id = dynamic_cast<JZNodeIRNodeId*>(ir);
            if(ir_id->id == nodeId)
                ir_id->breakPointType = BreakPoint::none;
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
    if(info.type == Node_function)
    {
        m_breakStep.type = BreakStep::stackEqual;
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
    
    m_breakStep.type = BreakStep::stepOver;
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

    m_breakStep.type = BreakStep::stackEqual;
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
        if(inList[i].dataType() == Type_args)
            break;
            
        const QVariant &v = getReg(Reg_CallIn + i);
        Q_ASSERT(m_env->isSameType(JZNodeType::variantType(v),inList[i].dataType()));
        if (inList[i].dataType() >= Type_class && JZNodeType::isNullObject(v))
        {
            QString error = "param" + QString::number(i + 1) + " is nullptr object";
            throw std::runtime_error(qUtf8Printable(error));
        }
    }
}

void JZNodeEngine::checkFunctionOut(const JZFunction *func)
{
    auto &outList = func->define.paramOut;
    for (int i = 0; i < outList.size(); i++)
    {        
        Q_ASSERT(m_env->isSameType(JZNodeType::variantType(getReg(Reg_CallOut + i)),outList[i].dataType()));
    }
}

void JZNodeEngine::callCFunction(const JZFunction *func)
{    
    pushStack(func);
    if(func->builtIn)
    {
        func->builtIn->call(this);
    }
    else
    {
        QVariantList paramIn, paramOut;
        // get input
        auto &inList = func->define.paramIn;
        for (int i = 0; i < inList.size(); i++)    
            paramIn.push_back(getReg(Reg_CallIn + i));
        
        // call function
        func->cfunc->call(paramIn,paramOut);

        // set output
        auto &outList = func->define.paramOut;
        for (int i = 0; i < outList.size(); i++)
            setReg(Reg_CallOut + i,paramOut[i]);
    }
    popStack();
}

const JZFunction *JZNodeEngine::function(QString name,const QVariantList *list)
{
    auto func_ptr = m_env.functionManager()->functionImpl(name);
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
    return m_env.functionManager()->functionImpl(func->fullName());
}

const JZFunction *JZNodeEngine::function(JZNodeIRCall *ir_call)
{
    auto func = function(ir_call->function,nullptr);
    if(!func->isVirtualFunction())
        ir_call->cache = func;

    return func;
}

void JZNodeEngine::unSupportSingleOp(int a, int op)
{
    QString error = QString("不支持的操作,操作符%1,数据类型%2").arg(JZNodeType::opName(op),
        m_env->typeToName(a));

    Q_ASSERT_X(0,"unSupportOp:",qUtf8Printable(error));
}

void JZNodeEngine::unSupportOp(int a, int b, int op)
{    
    QString error = QString("操作符%1,数据类型%2,%3").arg(JZNodeType::opName(op), 
        m_env->typeToName(a), m_env->typeToName(b));

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
            auto obj1 = toJZObject(a);
            auto obj2 = toJZObject(b);
            bool ret = obj_inst->equal(obj1,obj2);
            if (op == OP_eq)
                return ret;
            else
                return !ret;
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
        if (m_breakStep.type == BreakStep::stepOver)
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
        else if (m_breakStep.type == BreakStep::stackEqual)
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

bool JZNodeEngine::isWidgetFunction(const JZFunction *function)
{
    if(!function->isCFunction() || !function->isMemberFunction())
        return false;

    return m_env->isInherits(function->className(),"QWidget");
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
                printNode();

                if(ir_id->breakPointType == BreakPoint::print)
                {
                    m_stack.currentEnv()->printNode = ir_id->id;
                }
                if(ir_id->breakPointType == BreakPoint::nodeEnter || checkPause(ir_id->id))
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
            auto a = getParam(ir_expr->src1);
            auto b = getParam(ir_expr->src2);
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
            auto a = getParam(ir_expr->src1);
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
            auto value = createVariable(ir_alloc->dataType);
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
        case OP_clone:
        {
            JZNodeIRClone *ir_set = (JZNodeIRClone*)op;
            auto obj = obj_inst->clone(toJZObject(getParam(ir_set->src)));
            auto ptr = JZNodeObjectPtr(obj,true);
            setParam(ir_set->dst,QVariant::fromValue(ptr));
            break;
        }
        case OP_buffer:
        {
            JZNodeIRBuffer *ir_buffer = (JZNodeIRBuffer*)op;
            auto v = JZObjectCreate<QByteArray>();
            QByteArray *buffer = JZObjectCast<QByteArray>(v);
            *buffer = ir_buffer->buffer;
            setParam(ir_buffer->id, v);
            break;
        }
        case OP_watch:
        {
            JZNodeIRWatch *ir_watch = (JZNodeIRWatch*)op;
            int id = ir_watch->traget.id();

            m_stack.currentEnv()->watchMap[id] = getParam(ir_watch->source);            
            break;
        }
        case OP_convert:
        {
            JZNodeIRConvert *ir_convert = (JZNodeIRConvert*)op;
            QVariant ret = m_env->convertTo(ir_convert->dstType,getParam(ir_convert->src));
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
                if(m_depend && isWidgetFunction(func))
                    throw std::runtime_error("can't use widget function in test,please hook first");

                m_regInCount = ir_call->inCount;
                if(func->isCFunction())
                    callCFunction(func);
                else
                {
                    pushStack(func);
                    continue;
                }
            }
            break;
        }
        case OP_return:
        {            
            if(m_depend && m_stack.size() == 1)
                watchNotify();
                
            printNode();
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
