#include "JZNodeEngine.h"
#include "JZNodeFunctionManager.h"
#include <QPushButton>
#include <QApplication>
#include <math.h>
#include <QDateTime>
#include <QTimer>
#include "JZNodeVM.h"
#include "JZNodeBind.h"
#include "JZNodeObjectParser.h"
#include "JZContainer.h"
#include "runtime/JZNodeUiLoader.h"
#include "LogManager.h"
#include "JZNodeCompiler.h"

void JZScriptLog(const QString &log)
{
    g_engine->print(log);
}

QVariant JZConvertVariant(const QVariant &in, int type)
{
    auto env = g_engine->environment();
    return env->convertTo(in, type);
}

QString JZObjectToString(JZNodeObject* obj)
{
    JZNodeObjectFormat format;
    return format.format(obj);
}

JZNodeObject* JZObjectFromString(int type, const QString& text)
{
    auto env = g_engine->environment();

    JZNodeObjectParser parser;
    auto obj = parser.parseToType(env->typeToName(type), text);
    if (!obj)
        throw std::runtime_error("create from string failed");

    return obj;
}

void JZScriptInvoke(const QString &function, const QVariantList &in, QVariantList &out)
{
    g_engine->invoke(function, in, out);
}

void JZScriptInvokeVirtual(const QString& function, const QVariantList& in, QVariantList& out)
{
    g_engine->invokeVirtual(function, in, out);
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
    inCount = 0;
    pc = -1;    
}

RunnerEnv::~RunnerEnv()
{       
}

void RunnerEnv::initVariable(QString name, QVariantPtr ptr)
{
    locals[name] = ptr;
}

void RunnerEnv::initVariable(int id, QVariantPtr ptr)
{ 
    stacks[id] = ptr;
}

QVariantPtr *RunnerEnv::getRef(int id)
{
    auto it = stacks.find(id);
    if (it == stacks.end())
        return nullptr;

    return &it.value();
}

QVariantPtr *RunnerEnv::getRef(const QString& name)
{
    auto it = locals.find(name);
    if (it == locals.end())
        return nullptr;

    return &it.value();
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
    m_env.pop_back();    
}

void Stack::push()
{       
    m_env.push_back(RunnerEnv());
}

//JZNodeRuntimeError
bool JZNodeRuntimeError::isError() const
{
    return !error.isEmpty();
}

QString JZNodeRuntimeError::errorReport() const
{
    QString text = "Error: " + error + "\n\n";
    int stack_size = info.stacks.size();
    for (int i = 0; i < stack_size; i++)
    {
        auto s = info.stacks[stack_size - i - 1];
        text += QString().asprintf("# %2d: ",i+1) + s.function;
        if (!s.scriptItemPath.isEmpty())
            text += +"(" + s.scriptItemPath + "," + QString::number(s.pc) + ")";
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
    s << param.scriptItemPath;
    s << param.nodeId;
    s << param.pc;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeRuntimeInfo::Stack &param)
{
    s >> param.function;
    s >> param.scriptItemPath;
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

//BreakStep
BreakStep::BreakStep()
{    
    clear();        
}

void BreakStep::clear()
{    
    type = BreakStep::none;
    scriptItemPath.clear();
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
thread_local JZNodeEngine *g_engine = nullptr;
void JZNodeEngine::regist()
{        
    JZNodeEngineIdlePauseEvent::Event = QEvent::registerEventType();      
}

JZNodeEngine::JZNodeEngine(QObject *parent)
    :QObject(parent)
{ 
    m_program = nullptr;
    m_script = nullptr;
    m_sender = nullptr;
    m_pc = -1;    
    m_debug = false;
    m_status = Status_none;
    m_statusCommand = Command_none;
    m_regs.resize(Reg_End - Reg_Start);
    m_idleFunc.define.name = "idle";
    m_watch = false;
}

JZNodeEngine::~JZNodeEngine()
{    
    Q_ASSERT(!isInit());
}

void JZNodeEngine::setProgram(const JZNodeProgram *program)
{
    m_program = program;
}

const JZNodeProgram *JZNodeEngine::program()
{
    return m_program;
}

JZScriptEnvironment *JZNodeEngine::environment()
{
    return &m_env;
}

bool JZNodeEngine::isInit() const
{
    return (g_engine == this);
}

bool JZNodeEngine::init()
{
    Q_ASSERT(!g_engine);

    // regist type
    m_program->initEnv(&m_env);
    updateStatus(Status_idle);

    g_engine = this;
    QVariantList in, out;
    if (!call("__init__", in, out))
    {        
        updateStatus(Status_none);
        g_engine = nullptr;
        return false;
    }
    
    return true;
}   

void JZNodeEngine::deinit()
{        
    m_watch = false;
    m_breakPoints.clear();
    m_breakIr.clear();
    m_breakStep.clear();

    m_stack.clear();
    m_global.clear();
    m_sender = nullptr;
    m_statusCommand = Command_none;
    updateStatus(Status_none);

    clearReg();
    if (g_engine == this)
        g_engine = nullptr;
}

void JZNodeEngine::statClear()
{
    m_stat.clear();
}

void JZNodeEngine::statReport()
{
    m_stat.report();
}

const JZFunctionDebugInfo *JZNodeEngine::currentFunctionDebugInfo()
{
    QString function = m_stack.currentEnv()->function->fullName();
    return m_script->functionDebug(function);
}

int JZNodeEngine::nodeIdByPc(const JZNodeScript *script,QString function, int pc)
{
    int min_range = INT_MAX;
    int node_id = -1;

    for (int i = pc; i >= 0; i--)
    {
        if (script->statmentList[i]->type == OP_nodeEnter)
        {
            auto ir = dynamic_cast<JZNodeIRNodeEnter*>(script->statmentList[i].data());
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
    return nodeIdByPc(m_pc);
}

int JZNodeEngine::nodeIdByPc(int pc)
{
    QString func = m_stack.currentEnv()->function->fullName();
    return nodeIdByPc(m_script, func, pc);
}

JZEngineStatus JZNodeEngine::status()
{
    QMutexLocker lock(&m_mutex);
    return m_status;
}

JZNodeRuntimeInfo JZNodeEngine::runtimeInfo()
{   
    JZNodeRuntimeInfo info;
    QMutexLocker lock(&m_mutex);
    info.status = m_status;
    if (m_status == Status_pause && m_stack.size() == 0)
    {
        JZNodeRuntimeInfo::Stack s;
        s.function = "__idle__";
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
                s.scriptItemPath = env->script->itemPath;
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
    if (m_stack.size() == 128) {
        throw std::runtime_error("stack overflow");
    }

    if (m_stack.size() > 0)    
        m_stack.currentEnv()->pc = m_pc;
    
    m_stack.push();    
    m_stack.currentEnv()->function = func;

    checkFunctionIn(func);     
    
    if(!func->isCFunction())
    {          
        m_pc = func->addr;
        m_script = getScript(func->path);
        Q_ASSERT_X(m_script,"No Scrpit in path:",qUtf8Printable(func->path));

        m_stack.currentEnv()->pc = m_pc;
        m_stack.currentEnv()->script = m_script;
        if (func->isMemberFunction())
        {
            auto &obj = m_stack.currentEnv()->self;
            obj.type = JZNodeType::variantType(m_regs[Reg_CallIn - Reg_Start]);
            *obj.ptr = m_regs[Reg_CallIn - Reg_Start];
        }
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
        m_mutex.lock();
        if (m_status == Status_idle && m_statusCommand == Command_pause)
        {
            m_statusCommand = Command_none;
            updateStatus(Status_pause);
            m_waitCond.wait(&m_mutex);
            m_statusCommand = Command_none;
            updateStatus(Status_idle);
        }
        m_mutex.unlock();
    }
}

bool JZNodeEngine::call(const QString &name,const QVariantList &in,QVariantList &out)
{    
    const JZFunction *func = function(name);
    return call(func,in,out);
}

bool JZNodeEngine::callVirtual(const QString& function, const QVariantList& in, QVariantList& out)
{
    JZNodeObject* obj = toJZObject(in[0]);
    const JZFunction* func = virtualFunction(obj,function);
    return call(func, in, out);
}

bool JZNodeEngine::call(const JZFunction *func,const QVariantList &in,QVariantList &out)
{    
    Q_ASSERT(m_stack.size() == 0);
    if (m_status == Status_error)
        return false;                
    
    try
    {
        updateStatus(Status_running);
        m_error = JZNodeRuntimeError();
        Q_ASSERT(func && (func->define.isVariadicFunction() || in.size() == func->define.paramIn.size()));
        for (int i = 0; i < in.size(); i++)
            setReg(Reg_CallIn + i,in[i]);
        
        if(func->isCFunction())
        {
            callCFunction(func);
        }
        else
        {            
            pushStack(func);            
            if(!run())  //停止运行会返回false, 异常在catch处理
            {                
                updateStatus(Status_idle);
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
            updateStatus(Status_idle);
            m_statusCommand = Command_none;
        }
        return true;
    }
    catch (const std::exception& e)
    {
        qDebug() << "Runtime Exception: " << e.what();

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

void JZNodeEngine::invoke(const QString& name, const QVariantList& in, QVariantList& out)
{
    if (status() == Status_idle) //顶层调用,需要try catch
    {
        call(name, in, out);
        return;
    }

    const JZFunction* func = function(name);
    Q_ASSERT(func && (func->define.isVariadicFunction() || in.size() == func->define.paramIn.size()));
    for (int i = 0; i < in.size(); i++)
        setReg(Reg_CallIn + i, in[i]);

    if (func->isCFunction())
    {
        callCFunction(func);
    }
    else
    {
        pushStack(func);
        if (!run())
            return;
    }

    out.clear();
    for (int i = 0; i < func->define.paramOut.size(); i++)
        out.push_back(getReg(Reg_CallOut + i));
    clearReg();
}

void JZNodeEngine::invokeVirtual(const QString& function, const QVariantList& in, QVariantList& out)
{
    JZNodeObject* obj = toJZObject(in[0]);
    const JZFunction* func = virtualFunction(obj, function);
    Q_ASSERT(func);
    return invoke(func->fullName(), in, out);
}

void JZNodeEngine::onSlot(const QString& function, const QVariantList& in, QVariantList& out)
{
    m_sender = toJZObject(in[0]);
    invoke(function, in, out);
    m_sender = nullptr;
}

QVariantPtr* JZNodeEngine::getParamRef(int stack_level, const JZNodeIRParam& param)
{
    QStringList obj_list;
    if (param.isRef())
    {
        obj_list = param.ref().split(".");
    }
    else if(param.isIdRef())
    {
        obj_list = param.ref().split(".");
        obj_list.insert(0, "__id__");
    }

    auto memberRef = [&obj_list](QVariantPtr* ref)->QVariantPtr* {
        for (int i = 1; i < obj_list.size(); i++)
        {
            auto obj = toJZObject(*ref->ptr);
            if (!obj)
                throw std::runtime_error("is null object");

            ref = obj->paramRef(obj_list[i]);
        }
        return ref;
    };
    
    if (m_stack.size() == 0)
    {
        QVariantPtr *ref = nullptr;
        auto it = m_global.find(obj_list[0]);
        if (it != m_global.end())
            ref = &it.value();
        
        if (obj_list.size() == 1)
            return ref;
        else
            return memberRef(ref);
    }
    else
    {
        RunnerEnv *env = (stack_level == -1) ? m_stack.currentEnv() : m_stack.env(stack_level);
        if (param.isStack())
            return env->getRef(param.id());
        else if (param.isThis())
            return &env->self;
        else
        {
            QVariantPtr *ref = nullptr;
            if (param.isIdRef())
                ref = env->getRef(param.id());
            else
            {
                if (obj_list[0] == "this")
                    ref = &env->self;
                else
                    ref = env->getRef(obj_list[0]);

                if (!ref)
                {
                    auto it = m_global.find(obj_list[0]);
                    if (it != m_global.end())
                        ref = &it.value();
                }
            }
            if (!ref)
                return nullptr;

            if (obj_list.size() == 1)
                return ref;
            else
                return memberRef(ref);
        }
    }
}

bool JZNodeEngine::hasParam(int stack_level, const JZNodeIRParam &param)
{
    return (getParamRef(stack_level, param) != nullptr);
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
        Q_ASSERT(ref);
        return ref->value();        
    }
}

void JZNodeEngine::setParam(int stack_level, const JZNodeIRParam &param, const QVariant &value)
{
    m_stat.setTime++;    

    if (param.isReg())
        setReg(param.id(), value);
    else
    {
        Q_ASSERT(!param.isThis());

        auto ref = getParamRef(stack_level, param);
        dealSet(ref, value);
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

QVariantPtr JZNodeEngine::initVariantPtr(int data_type)
{
    QVariantPtr ptr;
    ptr.type = data_type;    
    if (JZNodeType::isPointer(data_type))
    {
        JZNodeObjectPointer obj_ptr(data_type);
        *ptr.ptr = QVariant::fromValue(obj_ptr);
    }
    else
    {
        if (data_type < Type_class)
            *ptr.ptr = m_env.defaultValue(data_type);
        else
            *ptr.ptr = QVariant::fromValue(JZNodeObjectPointer(data_type));
    }

    return ptr;
}

void JZNodeEngine::initGlobal(QString name, int data_type)
{
	m_global[name] = initVariantPtr(data_type);
}

void JZNodeEngine::initLocal(QString name, int data_type)
{
    auto env = m_stack.currentEnv();
    env->initVariable(name, initVariantPtr(data_type));
}

void JZNodeEngine::initLocal(int id, int data_type)
{
    auto env = m_stack.currentEnv();
    env->initVariable(id, initVariantPtr(data_type));
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

QVariant JZNodeEngine::createVariable(int type,const QString &value)
{
    auto inst = m_env.objectManager();

    QVariant v;
    if (JZNodeType::isPointer(type))
    {
        JZNodeObjectPointer ref(type);
        v = QVariant::fromValue(ref);
    }
    else if(type < Type_class)
        v = m_env.initValue(type, value);
    else
    {        
        JZNodeObject *sub = nullptr; 
        if (value.isEmpty())
        {
            auto def = inst->meta(type);
            if(def->isValueType())
                sub = inst->create(type);
            else
            {
                sub = inst->create(type);             
            }
        }        
        else if(value.startsWith("{") && value.endsWith("}"))
        {
            QString init_text = value.mid(1,value.size() - 2);
            if(init_text.isEmpty())
                sub = inst->create(type); 
            else
                sub = JZObjectFromString(type, init_text);
        }
        v = QVariant::fromValue(JZNodeObjectPointer(sub,true));
    }
    return v;
}

JZNodeObject *JZNodeEngine::getVariableObject(QVariant *ref, const QStringList &obj_list)
{        
    JZNodeObject *obj = toJZObject(*ref);
    if(!obj)
        throw std::runtime_error("object is nullptr");

    for (int i = 0; i < obj_list.size(); i++)
    {
        obj = toJZObject(obj->param(obj_list[i]));
        Q_ASSERT(obj);
        if (!obj)
            throw std::runtime_error("object is nullptr");
    }

    return obj;
}

void JZNodeEngine::dealSet(QVariantPtr *ref, const QVariant &value)
{
    Q_ASSERT_X(ref && m_env.isSameType(JZNodeType::variantType(value),ref->type),"",qUtf8Printable("set " + m_env.variantTypeName(value) 
        + " to " + m_env.typeToName(ref->type)));
    
    if(!JZNodeType::variantIsPointer(value) && JZNodeType::isPointer(ref->type))
        *ref->ptr = JZNodeType::convertToPointer(value);
    else if (JZNodeType::isNullptr(value) && JZNodeType::variantIsHolder(*ref->ptr))
    {
        JZNodeObjectPointer* h = (JZNodeObjectPointer*)ref->ptr.data();
        h->relaseObject();
    }
    else
    {
        if (ref->isCParam())
            ref->setValue(value);
        else
            *ref->ptr = value;
    }
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

void JZNodeEngine::printMemory()
{
    QString text;
    m_mutex.lock();

    text += "global:\n";
    auto g_it = m_global.begin();
    while(g_it != m_global.end())
    {
        auto *var = g_it->ptr.data();
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

int JZNodeEngine::regInCount()
{
    for (int i = 0; i < 16; i++)
    {
        int reg_start = Reg_CallIn - Reg_Start;
        if (!m_regs[reg_start + i].isValid())
            return i;
    }
    return 16;
}

const JZNodeScript *JZNodeEngine::getScript(QString path)
{
    return m_program->script(path);
}

void JZNodeEngine::startWatch()
{    
    QMutexLocker lock(&m_mutex);
    m_watch = true;
}

void JZNodeEngine::stopWatch()
{
    QMutexLocker lock(&m_mutex);
    m_watch = false;
}

void JZNodeEngine::watchNotify()
{
    if(!m_watch || m_stack.size() == 0)
        return;

    emit sigWatchNotify();
}

void JZNodeEngine::printNode(int node_id)
{
    auto env = m_stack.currentEnv();         
    auto info = currentFunctionDebugInfo();
    auto &node_info = info->nodeInfo[node_id];
/*
    QString line = node_info.name + "(id=" + QString::number(node_info.id);
    if(node_info.paramIn.size() > 0)
        line += ",";
    for(int i = 0; i < node_info.paramIn.size(); i++)
    {
        QString name = node_info.paramIn[i].define.name;
        int param_id = JZNodeCompiler::paramId(node_info.id, node_info.paramIn[i].id);
        auto ref = env->getRef(param_id);
        line += name + " " + JZNodeType::debugString(*ref->ptr);
    }

    if(node_info.paramIn.size() > 0 && node_info.paramOut.size() > 0)
        line += "->";
    for(int i = 0; i < node_info.paramOut.size(); i++)
    {
        QString name = node_info.paramOut[i].define.name;
        int param_id = JZNodeCompiler::paramId(node_info.id, node_info.paramOut[i].id);
        auto ref = env->getRef(param_id);
        line += name + " " + JZNodeType::debugString(*ref->ptr);
    }
    line += ")";
    print(line);
*/
}

void JZNodeEngine::onWatchTimer()
{
    watchNotify();
}

void JZNodeEngine::setDebug(bool flag)
{
    m_debug = flag;
}

void JZNodeEngine::addBreakPoint(QString itemPath,int nodeId)
{
    BreakPoint pt;
    pt.type = BreakPoint::nodeEnter;
    pt.nodeId = nodeId;
    pt.scriptItemPath = itemPath;

    addBreakPoint(pt);
}

void JZNodeEngine::addBreakPoint(const BreakPoint &pt)
{
    QMutexLocker lock(&m_mutex);
    int idx = indexOfBreakPoint(pt.scriptItemPath,pt.nodeId);
    if(idx != -1)
    {
        m_breakPoints[idx] = pt;
        return;
    }
    m_breakPoints.push_back(pt);
    if (pt.type == BreakPoint::nodeEnter)
    {
        auto script = m_program->script(pt.scriptItemPath);
        for (int i = 0; i < script->statmentList.size(); i++)
        {
            auto ir = script->statmentList[i].data();
            if (ir->type == OP_nodeEnter && ((JZNodeIRNodeEnter*)ir)->id == pt.nodeId)
            {
                m_breakIr.insert((JZNodeIRNodeEnter*)ir);
            }
        }
    }
}

void JZNodeEngine::removeBreakPoint(QString filepath,int nodeId)
{
    QMutexLocker lock(&m_mutex);
    int idx = indexOfBreakPoint(filepath,nodeId);
    if(idx == -1)
        return;

    BreakPoint pt = m_breakPoints[idx];
    m_breakPoints.removeAt(idx);
    if (pt.type == BreakPoint::nodeEnter)
    {
        auto script = m_program->script(pt.scriptItemPath);
        for (int i = 0; i < script->statmentList.size(); i++)
        {
            auto ir = script->statmentList[i].data();
            if (ir->type == OP_nodeEnter && ((JZNodeIRNodeEnter*)ir)->id == pt.nodeId)
            {
                m_breakIr.remove((JZNodeIRNodeEnter*)ir);
            }
        }
    }
}

int JZNodeEngine::indexOfBreakPoint(QString filepath,int nodeId)
{
    for(int i = 0; i < m_breakPoints.size(); i++)
    {
        if(m_breakPoints[i].scriptItemPath == filepath && m_breakPoints[i].nodeId == nodeId)
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
        removeBreakPoint(m_breakPoints[i].scriptItemPath, m_breakPoints[i].nodeId);
    
    QMutexLocker lock(&m_mutex);
    m_breakPoints.clear();
}

bool JZNodeEngine::isPauseOrError()
{
    QMutexLocker lock(&m_mutex);
    return m_status == Status_pause || m_status == Status_error;
}

void JZNodeEngine::pause()
{
    Q_ASSERT(m_status != Status_none);

    QMutexLocker lock(&m_mutex);
    if (m_status != Status_running && m_status != Status_idle)
        return;

    if (m_status == Status_idle)
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
    if (m_status != Status_pause)
        return;    
    
    m_statusCommand = Command_resume;
    lock.unlock();
    m_waitCond.wakeOne();
    waitCommand();
}

void JZNodeEngine::stop()
{
    QMutexLocker lock(&m_mutex);
    if(m_status == Status_idle || m_status == Status_none)
        return;

    if (m_status == Status_error && !m_debug)
        return;
        
    m_statusCommand = Command_stop;
    lock.unlock();
    if(m_status == Status_pause || m_status == Status_error)
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
    m_breakStep.scriptItemPath = m_script->itemPath;
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
        int data_type = m_env.nameToType(inList[i].type);
        if(data_type == Type_args)
            break;
            
        const QVariant &v = getReg(Reg_CallIn + i);
        Q_ASSERT_X(m_env.isSameType(JZNodeType::variantType(v), data_type), "", qUtf8Printable("set " 
            + m_env.variantTypeName(v) + " to " + m_env.typeToName(data_type)));        
        if (func->className() != "string" && func->isMemberFunction() && i == 0 && JZNodeType::isNullObject(v))
        {
            QString error = "this is nullptr";
            throw std::runtime_error(qUtf8Printable(error));
        }
    }
}

void JZNodeEngine::checkFunctionOut(const JZFunction *func)
{
    auto &outList = func->define.paramOut;
    for (int i = 0; i < outList.size(); i++)
    {        
        Q_ASSERT(m_env.isSameType(JZNodeType::variantType(getReg(Reg_CallOut + i)),m_env.nameToType(outList[i].type)));
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
        try {
            func->cfunc->call(paramIn, paramOut);
        }
        catch (const std::exception& e) 
        {
            if (catchException(e.what()))
                return;

            throw e;
        }

        // set output
        auto &outList = func->define.paramOut;
        for (int i = 0; i < outList.size(); i++)
            setReg(Reg_CallOut + i,paramOut[i]);
    }
    popStack();
}

const JZFunction* JZNodeEngine::virtualFunction(JZNodeObject* obj, QString name)
{
    auto def = JZFunctionHelper::splitFunction(name);

    auto func = obj->function(def.name);
    Q_ASSERT_X(func, "Error", qUtf8Printable("no function " + def.name));
    return m_env.functionManager()->functionImpl(func->fullName());
}

const JZFunction *JZNodeEngine::function(QString name)
{
    auto func_ptr = m_env.functionManager()->functionImpl(name);
    return func_ptr;
}

const JZFunction *JZNodeEngine::function(const JZNodeIRCall *ir_call)
{
    if(ir_call->isVirtual)
    {
        auto obj = toJZObject(getReg(Reg_CallIn));
        return virtualFunction(obj,ir_call->function);
    }
    else
    {
        auto func = function(ir_call->function);
        return func;
    }
}

template<class T>
QVariant dealExprInt(T a, T b, int op)
{
    switch (op)
    {
    case OP_add:
        return QVariant::fromValue<T>(a + b);
    case OP_sub:
        return QVariant::fromValue<T>(a - b);
    case OP_mul:
        return QVariant::fromValue<T>(a * b);
    case OP_div:
    {
        if(b == 0)
            throw std::runtime_error("divide zero");
        return QVariant::fromValue<T>(a / b);
    }
    case OP_mod:
        return QVariant::fromValue<T>(a % b);
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
    case OP_bitand:
        return QVariant::fromValue<T>(a & b);
    case OP_bitor:
        return QVariant::fromValue<T>(a | b);
    case OP_bitxor:
        return QVariant::fromValue<T>(a ^ b);
    default:
        Q_ASSERT(0);
        return QVariant();
    }
}

template<class T>
QVariant dealExprDouble(T a, T b, int op)
{
    switch (op)
    {
    case OP_add:
        return QVariant::fromValue<T>(a + b);
    case OP_sub:
        return QVariant::fromValue<T>(a - b);
    case OP_mul:
        return QVariant::fromValue<T>(a * b);
    case OP_div:
        return QVariant::fromValue<T>(a / b);
    case OP_mod:
        return QVariant::fromValue<T>(fmod(a, b));
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
    default:
        Q_ASSERT(0);
        return QVariant();
    }
}

QVariant JZNodeEngine::dealExpr(const QVariant &a, const QVariant &b,int op)
{   
    int dataType1 = JZNodeType::variantType(a);
    int dataType2 = JZNodeType::variantType(b);
    Q_ASSERT(dataType1 == dataType2);

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
                Q_ASSERT(0);
                break;
        }
    }
    else if (dataType1 == Type_bool)
    {
        if (op == OP_eq)
            return a.toBool() == b.toBool();
        else if(op == OP_ne)
            return a.toBool() != b.toBool();

        Q_ASSERT(0);
    }
    else if(dataType1 == Type_int8)
        return dealExprInt(a.value<int8_t>(), b.value<int8_t>(), op);
    else if (dataType1 == Type_int16)
        return dealExprInt(a.value<int16_t>(), b.value<int16_t>(), op);
    else if (dataType1 == Type_int)
        return dealExprInt(a.value<int>(), b.value<int>(), op);
    else if (dataType1 == Type_int64)
        return dealExprInt(a.value<int64_t>(), b.value<int64_t>(), op);
    else if (dataType1 == Type_uint8)
        return dealExprInt(a.value<uint8_t>(), b.value<uint8_t>(), op);
    else if (dataType1 == Type_uint16)
        return dealExprInt(a.value<uint16_t>(), b.value<uint16_t>(), op);
    else if (dataType1 == Type_uint)
        return dealExprInt(a.value<uint>(), b.value<uint>(), op);
    else if (dataType1 == Type_uint64)
        return dealExprInt(a.value<uint64_t>(), b.value<uint64_t>(), op);
    else if (dataType1 == Type_float)
        return dealExprDouble(a.value<float>(), b.value<float>(), op);
    else if (dataType1 == Type_double)
        return dealExprDouble(a.value<double>(), b.value<double>(), op);
    else
    {
        if (op == OP_eq || op == OP_ne)
        {
            auto obj1 = toJZObject(a);
            auto obj2 = toJZObject(b);
            bool ret = m_env.objectManager()->equal(obj1,obj2);
            if (op == OP_eq)
                return ret;
            else
                return !ret;
        } 
        Q_ASSERT(0);
    }
    return QVariant();
}

QVariant JZNodeEngine::dealSingleExpr(const QVariant &a, int op)
{
    int dataType = JZNodeType::variantType(a);
    if (op == OP_not)
    {
        Q_ASSERT(dataType == Type_bool);
        return !a.toBool();
    }
    else if (op == OP_neg)
    {
        if (dataType == Type_int8)
            return QVariant::fromValue<int8_t>(-a.value<int8_t>());
        else if (dataType == Type_int16)
            return QVariant::fromValue<int16_t>(-a.value<int16_t>());
        else if (dataType == Type_int)
            return QVariant::fromValue<int>(-a.value<int>());
        else if (dataType == Type_int64)
            return QVariant::fromValue<int64_t>(-a.value<int64_t>());
        else if (dataType == Type_float)
            return QVariant::fromValue<float>(-a.value<float>());
        else if (dataType == Type_double)
            return QVariant::fromValue<double>(-a.value<double>());
    }
    else if (op == OP_bitreverse)
    {
        if (dataType == Type_int8)
            return QVariant::fromValue<int8_t>(~a.value<int8_t>());
        else if (dataType == Type_int16)
            return QVariant::fromValue<int16_t>(~a.value<int16_t>());
        else if (dataType == Type_int)
            return QVariant::fromValue(~a.value<int>());
        else if (dataType == Type_int64)
            return QVariant::fromValue(~a.value<int64_t>());
        else if (dataType == Type_uint8)
            return QVariant::fromValue<uint8_t>(~a.value<uint8_t>());
        else if (dataType == Type_uint16)
            return QVariant::fromValue<uint16_t>(~a.value<uint16_t>());
        else if (dataType == Type_uint)
            return QVariant::fromValue(~a.value<uint>());
        else if (dataType == Type_uint64)
            return QVariant::fromValue(~a.value<uint64_t>());
    }

    Q_ASSERT(0);
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
            else if (m_breakStep.scriptItemPath != m_script->itemPath)
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
    m_breakStep.clear();
    m_stack.currentEnv()->pc = m_pc;
    m_statusCommand = Command_none;
    updateStatus(Status_pause);
    m_waitCond.wait(&m_mutex);
    
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

void JZNodeEngine::updateStatus(JZEngineStatus status)
{
    Q_ASSERT((m_status == Status_none && (status == Status_idle))
        || (m_status == Status_idle && (status == Status_running || status == Status_none || status == Status_pause))
        || (m_status == Status_running && (status == Status_idle || status == Status_pause || status == Status_error))
        || (m_status == Status_pause && (status == Status_idle || status == Status_running))
        || (m_status == Status_error && (status == Status_none)));

    int pre_status = m_status;
    if (m_status != status)
    {
        m_status = status;
        sigStatusChanged(m_status);
    }
}

void JZNodeEngine::pushTryCatch(JZNodeIRTry *ir)
{
    TryCatchInfo info;
    info.stack = m_stack.size() - 1;
    info.catchPc = ir->catchPc;
    info.irExcep = ir->irExcep;
    m_tryCatchList.push_back(info);
}

void JZNodeEngine::popTryCatch()
{
    m_tryCatchList.pop_back();
}

bool JZNodeEngine::catchException(QString tips)
{
    if (m_tryCatchList.size() == 0)
        return false;

    TryCatchInfo info = m_tryCatchList.back();
    m_tryCatchList.pop_back();
    
    Q_ASSERT(info.stack + 1 <= m_stack.size());
    while (info.stack + 1 < m_stack.size())
        m_stack.pop();
    
    setParam(info.irExcep, tips);
    m_pc = info.catchPc;
    return true;
}

bool JZNodeEngine::run()
{    
    auto obj_inst = m_env.objectManager();
    
    int in_stack_size = m_stack.size();
    while (true)
    {                   
        if(m_statusCommand == Command_stop)
            return false;

        m_stat.statmentTime++;

        auto &op_list = m_script->statmentList;
        const JZNodeIR* op = op_list[m_pc].data();
        switch (op->type)
        {        
        case OP_nodeEnter:
        {            
            if (m_debug)
            {
                const JZNodeIRNodeEnter *ir_pt = (const JZNodeIRNodeEnter*)(op);
                if (m_breakIr.contains(ir_pt) || checkPause(ir_pt->id))
                {
                    if (breakPointTrigger(ir_pt->id))
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

            const JZNodeIRExpr *ir_expr =  (const JZNodeIRExpr*)(op);
            QVariant c;
            auto a = getParam(ir_expr->src1);
            auto b = getParam(ir_expr->src2);
            c = dealExpr(a,b,ir_expr->type);
            setParam(ir_expr->dst,c);
            break; 
        }
        case OP_not:
        case OP_neg:
        case OP_bitreverse:
        {
            m_stat.exprTime++;

            const JZNodeIRExpr* ir_expr = (const JZNodeIRExpr*)(op);
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
            const JZNodeIRJmp* ir_jmp = (const JZNodeIRJmp*)(op);
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
            const JZNodeIRAlloc *ir_alloc = (const JZNodeIRAlloc*)op;                        
            if (ir_alloc->allocType == JZNodeIRAlloc::Heap)            
                initGlobal(ir_alloc->dst.ref(), ir_alloc->dataType);
            else if (ir_alloc->allocType == JZNodeIRAlloc::Stack)            
                initLocal(ir_alloc->dst.ref(), ir_alloc->dataType);            
            else            
                initLocal(ir_alloc->dst.id(), ir_alloc->dataType);
            break;
        }
        case OP_reference:
        {
            const JZNodeIRReference* ir_ref = (const JZNodeIRReference*)op;

            QVariantPtr ptr = *getParamRef(-1, ir_ref->orig);
            auto env = m_stack.currentEnv();
            env->initVariable(ir_ref->ref.id(), ptr);
            break;
        }
        case OP_clearReg:
        {
            clearReg();
            break;
        }
        case OP_set:
        {
            const JZNodeIRSet *ir_set = (const JZNodeIRSet*)op;
            setParam(ir_set->dst,getParam(ir_set->src));
            break;
        }
        case OP_clone:
        {
            const JZNodeIRClone *ir_set = (const JZNodeIRClone*)op;
            auto obj = obj_inst->clone(toJZObject(getParam(ir_set->src)));
            auto ptr = JZNodeObjectPointer(obj,true);
            setParam(ir_set->dst,QVariant::fromValue(ptr));
            break;
        }
        case OP_convert:
        {
            const JZNodeIRConvert *ir_convert = (const JZNodeIRConvert*)op;
            QVariant ret = m_env.convertTo(getParam(ir_convert->src),ir_convert->dstType);
            setParam(ir_convert->dst,ret);
            break;   
        }
        case OP_call:
        {           
            m_stat.callTime++;

            const JZNodeIRCall *ir_call = (const JZNodeIRCall*)op;
            const JZFunction *func = function(ir_call);
            Q_ASSERT(func);            

            if(func->isCFunction())
                callCFunction(func);
            else
            {
                pushStack(func);
                continue;
            }
            break;
        }
        case OP_return:
        {              
            watchNotify();
            popStack();                  
            if(m_stack.size() < in_stack_size)
                goto RunEnd;
            break;
        }
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
        case OP_try:
        {
            JZNodeIRTry* ir_throw = (JZNodeIRTry*)op;
            if (ir_throw->catchType == JZNodeIRTry::InTry)
                pushTryCatch(ir_throw);
            else
                popTryCatch();
            break;
        }
        case OP_throw:
        {
            JZNodeIRThrow* ir_throw = (JZNodeIRThrow*)op;
            QString tips = getParam(ir_throw->exception).toString();
            if (catchException(tips))
                continue;
            else
                throw std::runtime_error(qUtf8Printable(tips));
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
