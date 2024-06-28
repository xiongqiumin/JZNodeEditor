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

JZNodeVariantAny JZDealExpr(const JZNodeVariantAny &in1, const JZNodeVariantAny &in2, int op)
{
    JZNodeVariantAny ret;
    ret.value = g_engine->dealExpr(in1.value, in2.value, op);
    return ret;
}

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

QString JZObjectToString(JZNodeObject *obj)
{
    JZNodeObjectFormat format;
    return format.format(obj);
}

JZNodeVariantAny JZObjectCreate(QString name)
{
    int id = JZNodeObjectManager::instance()->getClassId(name);
    JZNodeObject *obj = JZNodeObjectManager::instance()->create(id);
    if (obj->isInherits("Object"))
        obj->setCOwner(false);

    JZNodeVariantAny any;
    any.value = QVariant::fromValue(obj);
    return any;
}

void JZScriptLog(const QString &log)
{
    g_engine->print(log);
    qDebug() << log;
}

bool JZScriptInvoke(const QString &function, const QVariantList &in, QVariantList &out)
{
    return g_engine->call(function, in, out);
}

void JZScriptOnSlot(JZNodeObject *sender,const QString &function,const QVariantList &in, QVariantList &out)
{
    g_engine->onSlot(sender,function,in,out);
}

bool statusIsPause(int status)
{
    return (status == Status_pause || status == Status_idlePause || status == Status_error);
}

//RunnerEnv
RunnerEnv::RunnerEnv()
{
    func = nullptr;
    script = nullptr;
    pc = -1;
}

void RunnerEnv::initVariable(QString name, const QVariant &value)
{
    locals[name] = JZVariantPtr(new JZVariant());
    locals[name]->init(value);
}

void RunnerEnv::initVariable(int id, const QVariant &value)
{
    stacks[id] = JZVariantPtr(new JZVariant());
    stacks[id]->init(value);
}

JZVariant *RunnerEnv::getRef(int id)
{
    auto it = stacks.find(id);
    if (it == stacks.end())
        return nullptr;

    return it->data();
}

JZVariant *RunnerEnv::getRef(QString name)
{
    auto it = locals.find(name);
    if (it == locals.end())
        return nullptr;

    return it->data();
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

// JZNodeEngine
JZNodeEngine *g_engine = nullptr;

void JZNodeEngine::regist()
{
    JZNodeEngineIdlePauseEvent::Event = QEvent::registerEventType();

    JZNodeFunctionManager::instance()->registCFunction("dealExpr", false, jzbind::createFuncion(JZDealExpr));
    JZNodeFunctionManager::instance()->registCFunction("createObject", false, jzbind::createFuncion(QOverload<QString>::of(JZObjectCreate)));
    //JZNodeFunctionManager::instance()->registCFunction("connect", false, jzbind::createFuncion(JZObjectConnect));

    JZNodeFunctionManager::instance()->registCFunction("forRuntimeCheck", true, jzbind::createFuncion(JZForRuntimeCheck));    
}

JZNodeEngine::JZNodeEngine()
{    
    m_program = nullptr;
    m_script = nullptr;
    m_sender = nullptr;
    m_pc = -1;        
    m_debug = false;
    m_status = Status_none;
    m_statusCommand = Command_none;
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
    m_regs.clear();      

    m_sender = nullptr;
    m_statusCommand = Status_none;
    m_status = Status_none;
    m_breakNodeId = -1;
    m_breakResume.clear();
    m_watchTime = 0;

    setReg(Reg_Cmp, false);
    if (g_engine == this)
        g_engine = nullptr;
}

void JZNodeEngine::init()
{
    clear();    

    // regist type
    JZNodeFunctionManager::instance()->clearUserReigst();
    JZNodeObjectManager::instance()->clearUserReigst();
    
    auto function_list = m_program->functionList();
    for (int i = 0; i < function_list.size(); i++)
    {
        auto *func = m_program->function(function_list[i]);
        JZNodeFunctionManager::instance()->registFunctionImpl(*func);
    }

    auto define_list = m_program->objectDefines();    
    for(int i = 0; i < define_list.size(); i++)
        JZNodeObjectManager::instance()->regist(define_list[i]);    

    Q_ASSERT(!g_engine);
    g_engine = this;
    QVariantList in, out;
    auto init_func = function("__init__");    
    call(init_func, in,out);
}   

void JZNodeEngine::deinit()
{
    clear();    
}

int JZNodeEngine::nodeIdByPc(JZNodeScript *script, int pc)
{
    int min_range = INT_MAX;
    int node_id = -1;
    auto it = script->nodeInfo.begin();
    while (it != script->nodeInfo.end())
    {
        auto &list = it->pcRanges;
        for (int i = 0; i < list.size(); i++)
        {
            if (pc >= list[i].start && pc < list[i].end)
            {
                int cur_min = qMin(pc - list[i].start, list[i].end - pc);
                if (cur_min < min_range)
                {
                    node_id = it.key();
                    min_range = cur_min;
                }
            }
        }

        it++;
    }
    
    Q_ASSERT(node_id != -1);
    return node_id;
}

NodeRange JZNodeEngine::nodeDebugRange(int node_id, int pc)
{
    auto &list = m_script->nodeInfo[node_id].pcRanges;
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
    return nodeIdByPc(m_script,m_pc);        
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
            s.function = env->func->fullName();
            if (env->script)
            {
                s.file = env->script->file;
                s.nodeId = nodeIdByPc(env->script, env->pc);
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
    m_stack.currentEnv()->func = func;
    if(!func->isCFunction())
    {                
        m_pc = func->addr;
        m_script = getScript(func->file);
        Q_ASSERT(m_script);

        m_stack.currentEnv()->pc = m_pc;
        m_stack.currentEnv()->script = m_script;
        if (func->isMemberFunction())
            m_stack.currentEnv()->object.init(m_regs[Reg_CallIn]->getVariant());        
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
    auto *func = function(name);
    return call(func,in,out);
}

bool JZNodeEngine::call(const JZFunction *func,const QVariantList &in,QVariantList &out)
{    
    if (checkIdlePause(func))
        return false;                    

    m_error = JZNodeRuntimeError();
    out.clear();
    Q_ASSERT(func && in.size() == func->define.paramIn.size());
    for (int i = 0; i < in.size(); i++)
        setReg(Reg_CallIn + i,in[i]);
    
    try
    {
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
                m_regs.clear();
                return false;
            }
        }
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

    out.clear();    
    for (int i = 0; i < func->define.paramOut.size(); i++)
        out.push_back(getReg(Reg_CallOut + i));    

    m_regs.clear();
    if (m_stack.size() == 0)
    {
        updateStatus(Status_none);
        m_statusCommand = Command_none;
        m_breakStep.clear();
    }
    return true;
}

bool JZNodeEngine::onSlot(JZNodeObject *sender,const QString &function,const QVariantList &in,QVariantList &out)
{
    bool ret = false;
    m_sender = sender;
    ret = call(function,in,out);
    m_sender = nullptr;
    return ret;
}

QVariant JZNodeEngine::getParam(const JZNodeIRParam &param)
{       
    if(param.isLiteral())    
        return param.value;
    else if(param.isRef())
        return getVariable(param.ref());
    else if(param.isThis())
        return getThis();
    else
    {
        if (param.id() >= Reg_Start)
            return getReg(param.id());
        else
        {
            auto ref = m_stack.currentEnv()->getRef(param.id());
            if (!ref)
                throw std::runtime_error("no such variable id=" + to_string(param.id()));

            return ref->getVariant();
        }
    }
}

void JZNodeEngine::setParam(const JZNodeIRParam &param,const QVariant &value)
{
    if (param.isId())
    {        
        if (param.id() >= Reg_Start)
            setReg(param.id(),value);
        else
        {
            JZVariant *ref = m_stack.currentEnv()->getRef(param.id());
            if (!ref)
                throw std::runtime_error("no such variable " + to_string(param.id()));
            dealSet(ref, value);
            watchNotify(param.id());
        }
    }
    else if(param.isRef())
        setVariable(param.ref(),value);
    else
    {
        Q_ASSERT(0);
    }                           
}

void JZNodeEngine::initGlobal(QString name, const QVariant &v)
{
	auto it = m_global.find(name);
	if (it == m_global.end())
		m_global[name] = JZVariantPtr(new JZVariant());
	
	m_global[name]->init(v);
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

Stack *JZNodeEngine::stack()
{
    return &m_stack;
}

void JZNodeEngine::splitMember(const QString &fullName, QStringList &objName,QString &memberName)
{
    QStringList list = fullName.split(".");
    if (list.size() > 1)
        objName = list.mid(0, list.size() - 1);
    
    memberName = list.back();
}

JZVariant *JZNodeEngine::getVariableRefSingle(RunnerEnv *env, const QString &name)
{
    JZVariant *obj = nullptr;
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

JZVariant *JZNodeEngine::getVariableRef(int id)
{
    return getVariableRef(id, -1);
}

JZVariant *JZNodeEngine::getVariableRef(int id, int stack_level)
{
    Q_ASSERT(m_stack.size() > 0);
    
    auto env = (stack_level == -1) ? m_stack.currentEnv() : m_stack.env(stack_level);
    auto it = env->stacks.find(id);
    if (it == env->stacks.end())
        return nullptr;

    return it->data();
}

QVariant JZNodeEngine::getVariable(int id)
{
    JZVariant *ref = getVariableRef(id);
    if (!ref)
        throw std::runtime_error("no such variable " + to_string(id));

    return ref->getVariant();
}

void JZNodeEngine::setVariable(int id, const QVariant &value)
{
    JZVariant *ref = getVariableRef(id);
    if (!ref)
        throw std::runtime_error("no such variable " + to_string(id));

    dealSet(ref, value);
}

JZVariant *JZNodeEngine::getVariableRef(const QString &name)
{
    return getVariableRef(name, -1);
}

JZVariant *JZNodeEngine::getVariableRef(const QString &name, int stack_level)
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
        JZVariant *ref = nullptr;
        if (obj_list[0] == "this")
            ref = env? &env->object : nullptr;
        else
            ref = getVariableRefSingle(env,obj_list[0]);

        if (!ref)
            return nullptr;

        JZNodeObject *obj = nullptr;
        for (int i = 1; i < obj_list.size(); i++)
        {
            obj = toJZObject(ref->getVariant());
            ref = obj->paramRef(obj_list[i]);
        }

        obj = toJZObject(ref->getVariant());
        return obj->paramRef(param_name);
    }
    else
    {
        return getVariableRefSingle(env, param_name);
    }
}

QVariant JZNodeEngine::getVariable(const QString &name)
{    
    JZVariant *ref = getVariableRef(name);
    if(!ref)
        throw std::runtime_error("no such variable " + name.toStdString());

    return ref->getVariant();        
}

void JZNodeEngine::setVariable(const QString &name, const QVariant &value)
{    
    JZVariant *ref = getVariableRef(name);
    if(!ref)
        throw std::runtime_error("no such variable " + name.toStdString());

    dealSet(ref, value);
}

void JZNodeEngine::dealSet(JZVariant *ref, const QVariant &value)
{
    ref->setVariant(value);
}

QVariant JZNodeEngine::getThis()
{    
    if (m_stack.size() == 0)
        return QVariant::fromValue(JZObjectNull());

    return m_stack.currentEnv()->object.getVariant();
}

QVariant JZNodeEngine::getSender()
{
    return QVariant::fromValue(m_sender);
}

void JZNodeEngine::print(const QString &log)
{
    emit sigLog(log);
}

QVariant JZNodeEngine::getReg(int id)
{               
    auto *ref = getRegRef(id);
    if (!ref)
        throw std::runtime_error("no such reg " + to_string(id));

    return ref->getVariant();
}

JZVariant *JZNodeEngine::getRegRef(int id)
{
    Q_ASSERT(id >= Reg_Start);
    
    auto it = m_regs.find(id);
    if (it == m_regs.end())
        return nullptr;

    return it.value().data();    
}

void JZNodeEngine::setReg(int id, const QVariant &value)
{    
    Q_ASSERT(id == Reg_Cmp || !m_regs.contains(id));
    auto ptr = JZVariantPtr(new JZVariant());
    ptr->init(value);
    m_regs[id] = ptr;
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
            QString text = JZNodeType::toString(getVariable(id));
            sigNodePropChanged(m_script->file, w.traget, text);
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
    auto script = m_program->script(filepath);

    BreakPoint pt;    
    pt.type = BreakPoint::nodeEnter;
    pt.file = filepath;
    pt.nodeId = nodeId;    
    if (script->nodeInfo[pt.nodeId].isFlow)    
        pt.pcRange = script->nodeInfo[pt.nodeId].pcRanges[0];
    m_breakPoints.push_back(pt);    
}

void JZNodeEngine::removeBreakPoint(QString filepath,int nodeId)
{
    QMutexLocker lock(&m_mutex);
    int idx = indexOfBreakPoint(filepath,nodeId);
    if(idx == -1)
        return;
    
    m_breakPoints.removeAt(idx);
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
    if(statusIsPause(m_status))
        m_waitCond.wakeOne();
    waitCommand();
}

void JZNodeEngine::stepIn()
{
    QMutexLocker lock(&m_mutex);
    if (m_status != Status_pause)
        return;
    
    int node_id = breakNodeId();
    auto info = m_script->nodeInfo[node_id];
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
    m_breakStep.pcRange = nodeDebugRange(m_breakStep.nodeId,m_pc);
    
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
        QVariant v = getReg(Reg_CallIn + i);
        Q_ASSERT(JZNodeType::isSameType(JZNodeType::variantType(v),inList[i].dataType()));        
        if (i == 0 && func->isMemberFunction() && JZNodeType::isNullptr(v))
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
    checkFunctionIn(func);

    QVariantList paramIn, paramOut;
    // get input
    auto &inList = func->define.paramIn;
    for (int i = 0; i < inList.size(); i++)    
        paramIn.push_back(getReg(Reg_CallIn + i));      
    m_regs.clear();
    
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

const JZFunction *JZNodeEngine::function(QString name)
{
    JZFunction *func = m_program->function(name);
    if(func)
        return func;        
    return JZNodeFunctionManager::instance()->functionImpl(name);
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
    else if((dataType1 == Type_bool && dataType2 == Type_bool)
            || (dataType1 == Type_int && dataType2 == Type_bool)
            || (dataType1 == Type_bool && dataType2 == Type_int)
            || (dataType1 == Type_int && dataType2 == Type_int))
    {
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
bool JZNodeEngine::checkPauseStop()
{
    bool wait = false;
    auto &op_list = m_script->statmentList;
    m_mutex.lock();
    if(m_statusCommand == Command_stop)
    {
        m_mutex.unlock();
        return true;
    }
    else if(m_statusCommand == Command_pause)
    {
        m_statusCommand = Command_none;
        wait = true;
    }
    else if(m_script->statmentList[m_pc]->isBreak)
    {                
        int node_id = -1;
        if (op_list[m_pc]->type == OP_nodeId)
        {
            JZNodeIRNodeId *ir_node = dynamic_cast<JZNodeIRNodeId*>(op_list[m_pc].data());
            node_id = ir_node->id;
            m_breakResume.clear();
        }

        int stack = m_stack.size();
        auto breakTriggred = [this](BreakPoint &pt,const QString &filepath,int stack,int pc,int node_id)->bool
        {
            if (pt.type == BreakPoint::nodeEnter && pt.file == filepath)
            {
                if (m_breakStep.nodeId == pt.nodeId
                    || m_breakResume.nodeId == pt.nodeId)
                    return false;

                if (node_id == pt.nodeId
                    || (pt.pcRange.start != -1 && pc >= pt.pcRange.debugStart && pc < pt.pcRange.end))
                {
                    this->m_breakNodeId = pt.nodeId;
                    return true;
                }
            }
            else if (pt.type == BreakPoint::stepOver)
            {                
                if (stack < pt.stack)
                    return true;
                else if (pt.stack == stack)
                {
                    Q_ASSERT(pt.file == filepath);
                    if (pc < pt.pcRange.start || pc >= pt.pcRange.end)                        
                        return true;                    
                }                                         
            }
            else if(pt.type == BreakPoint::stackEqual && stack == pt.stack)
                return true;

            return false;
        };
        
        if(m_breakStep.type != BreakPoint::none 
            && breakTriggred(m_breakStep, m_script->file, stack, m_pc, node_id))
        {            
            wait = true;
        }   
        else
        {
            for (int i = 0; i < m_breakPoints.size(); i++)
            {
                auto &pt = m_breakPoints[i];
                if (breakTriggred(pt, m_script->file, stack, m_pc, node_id))
                {
                    wait = true;
                    break;
                }
            }
        }
    }
    if (wait)
    {        
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

            if (m_script)
            {                
                m_breakResume.file = m_script->file;
                m_breakResume.nodeId = nodeIdByPc(m_stack.currentEnv()->pc);
            }
        }
        m_mutex.unlock();
        if (cmd == Command_stop)
            return true;                            
    }
    else
    {
        m_mutex.unlock();
    }
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

bool JZNodeEngine::run()
{    
    int in_stack_size = m_stack.size();
    while (true)
    {                   
        Q_ASSERT(m_script);
        
        if(checkPauseStop())
            return false;

        auto &op_list = m_script->statmentList;
        //deal op
        JZNodeIR *op = op_list[m_pc].data();           
        int op_type = op->type;
        switch (op_type)
        {
        case OP_nodeId:
            break;
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
            JZNodeIRExpr *ir_expr =  dynamic_cast<JZNodeIRExpr*>(op);
            QVariant a,b,c;
            a = getParam(ir_expr->src1);
            b = getParam(ir_expr->src2);
            c = dealExpr(a,b,ir_expr->type);
            setParam(ir_expr->dst,c);
            break; 
        }
        case OP_not:
        case OP_bitresver:
        {
            JZNodeIRExpr *ir_expr = dynamic_cast<JZNodeIRExpr*>(op);
            QVariant a, c;
            a = getParam(ir_expr->src1);
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
            if(op_type == OP_jmp)
                m_pc = jmpPc;
            else
            {
                bool flag = getReg(Reg_Cmp).toBool();
                if(op_type == OP_je)
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
            JZNodeIRCall *ir_call = (JZNodeIRCall*)op;            
            auto func = function(ir_call->function);
            Q_ASSERT(func);            

            if(func->isCFunction())
                callCFunction(func);
            else
            {
                checkFunctionIn(func);
                pushStack(func);
                continue;
            }
            break;
        }
        case OP_return:
        {            
            checkFunctionOut(m_stack.currentEnv()->func);
            popStack();                  
            if(m_stack.size() < in_stack_size)
                goto RunEnd;
            break;
        }
        case OP_clearRegCall:
        {
            m_regs.clear();
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
