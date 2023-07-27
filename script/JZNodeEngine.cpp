#include "JZNodeEngine.h"
#include "JZNodeFunctionManager.h"
#include "JZEvent.h"
#include <QPushButton>
#include <QApplication>
#include <math.h>
#include "JZNodeVM.h"

void JZObjectEvent(JZEvent *event)
{
    g_engine->dealEvent(event);
}

void JZScriptLog(const QString &log)
{
    g_engine->print(log);
    qDebug() << log;
}

//RunnerEnv
RunnerEnv::RunnerEnv()
{
    func = nullptr;
    pc = -1;
}

//Stack
Stack::Stack()
{
    clear();
}

Stack::~Stack()
{
}

void Stack::clear()
{
    m_stack.clear();
    m_env.clear();  
}

int Stack::size() const
{
    return m_stack.size();
}

bool Stack::isEmpty() const
{
    return (m_stack.size() == 0);
}

RunnerEnv &Stack::env()
{
    return m_env.back();
}

void Stack::pop()
{        
    m_stack.pop_back();    
    m_env.pop_back();
}

void Stack::push()
{       
    StackVariant v;
    m_stack.push_back(v);
    m_env.push_back(RunnerEnv());
}

QVariant *Stack::getVariableRef(QString name)
{
    int level = m_stack.size() - 1;
    auto it = m_stack[level].locals.find(name);
    if(it != m_stack[level].locals.end())
        return &it.value();
    else
        return nullptr;
}

QVariant Stack::getVariable(const QString &name)
{
    int level = m_stack.size() - 1;
    return m_stack[level].locals.value(name,QVariant());
}

void Stack::setVariable(const QString &name, const QVariant &value)
{
    int level = m_stack.size() - 1;
    m_stack[level].locals[name] = value;
}

QVariant Stack::getVariable(int id)
{
    return getVariable(m_stack.size() - 1,id);
}

void Stack::setVariable(int id, const QVariant &value)
{
    setVariable(m_stack.size() - 1,id,value);
}

QVariant Stack::getVariable(int level,int id)
{
    return m_stack[level].stacks.value(id,QVariant());
}

void Stack::setVariable(int level,int id, const QVariant &value)
{
    m_stack[level].stacks[id] = value;
}

//JZNodeRuntimeError
QDataStream &operator<<(QDataStream &s, const JZNodeRuntimeError &param)
{
    s << param.info;
    s << param.script;
    s << param.pc;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeRuntimeError &param)
{
    s >> param.info;
    s >> param.script;
    s >> param.pc;
    return s;
}

//JZNodeRuntimeInfo
JZNodeRuntimeInfo::JZNodeRuntimeInfo()
{
    status = Status_none;
    nodeId = -1;
    pc = -1;
}

QDataStream &operator<<(QDataStream &s, const JZNodeRuntimeInfo &param)
{        
    s << param.status << param.file << param.nodeId << param.pc;
    return s;    
}

QDataStream &operator>>(QDataStream &s, JZNodeRuntimeInfo &param)
{    
    s >> param.status >> param.file >> param.nodeId >> param.pc;
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

//JZObjectConnect
JZNodeEngine::JZObjectConnect::JZObjectConnect()
{
    eventType = Event_none;
    sender = nullptr;
    receiver = nullptr;
    handle = nullptr;
}

//ParamChangeEvent
JZNodeEngine::ParamChangeEvent::ParamChangeEvent()
{
    receiver = nullptr;
    handle = nullptr;
}

// JZNodeEngine
JZNodeEngine *g_engine = nullptr;
JZNodeEngine::JZNodeEngine()
{    
    m_program = nullptr;
    m_debug = nullptr;
    m_script = nullptr;
    m_pc = -1;    
    m_status = Status_none;
    m_statusCommand = Command_none;
}

JZNodeEngine::~JZNodeEngine()
{    
}

void JZNodeEngine::setProgram(JZNodeProgram *program)
{
    m_program = program;
}

JZNodeProgram *JZNodeEngine::program()
{
    return m_program;
}

void JZNodeEngine::setDebugger(JZNodeDebugServer *debug)
{
    m_debug = debug;
}

void JZNodeEngine::clear()
{
    JZNodeFunctionManager::instance()->clearUserReigst();
    JZNodeObjectManager::instance()->clearUserReigst();

    m_breakPoints.clear();
    m_breakStep.clear();    

    m_watchParam.clear();

    m_stack.clear();
    m_global.clear();
    m_regs.clear();
    m_this.clear();
    m_connectInfo.clear();

    m_statusCommand = Status_none;
    m_status = Status_none;

    m_connects.clear();
    m_paramChangeHandle.clear();
}

void JZNodeEngine::init()
{
    clear();

    // regist type
    auto define_list = m_program->objectDefines();    
    for(int i = 0; i < define_list.size(); i++)
        JZNodeObjectManager::instance()->regist(define_list[i]);

    // init variable    
    auto program_params = m_program->variables();
    auto it = program_params.begin();
    while(it != program_params.end() )
    {        
        if(it.value().dataType < Type_object)
        {            
            if(it.value().value.isNull())
                m_global[it.key()] = QVariant(JZNodeType::typeToQMeta(it.value().dataType));
            else
                m_global[it.key()] = it.value().value;
        }
        else
        {
            m_global[it.key()] = QVariant::fromValue(JZObjectNull());
        }
        it++;
    }        

    // connect events
    auto script_list = m_program->scriptList();
    for (int script_index = 0; script_index < script_list.size(); script_index++)
    {
        auto &script = script_list[script_index];
        QList<JZEventHandle> &events = script->events;
        for (int event_index = 0; event_index < events.size(); event_index++)
        {            
            auto &event = events[event_index];
            if (event.type == Event_programStart)
            {
                JZObjectConnect connect;
                connect.eventType = event.type;
                connect.handle = &event.function;
                m_connects[nullptr].push_back(connect);
                continue;
            }

            ConnectInfo info;            
            info.sender = event.sender;
            info.eventType = event.type;
            info.recv = script->className;
            info.handle = &event.function;
            m_connectInfo.push_back(info);
        }
    }    

    // init value
    for(int i = 0; i < m_paramChangeHandle.size(); i++)
    {
        QVariantList in,out;
        auto &handle = m_paramChangeHandle[i];
        setThis(QVariant::fromValue(handle.receiver));
        call(handle.handle,in,out);
    }
}   

int JZNodeEngine::nodeIdByPc(int m_pc)
{
    auto &op_list = m_script->statmentList;
    for(int i = m_pc; i >= 0 ; i--)
    {
        if(op_list[i]->type == OP_nodeId)
        {
            JZNodeIRNodeId *ir_node = (JZNodeIRNodeId *)op_list[i].data();
            return ir_node->id;
        }
    }
    Q_ASSERT(0);
    return 0;
}

JZNodeRuntimeInfo JZNodeEngine::runtimeInfo()
{   
    JZNodeRuntimeInfo info;
    QMutexLocker lock(&m_mutex);
    info.status = m_status;
    if(m_script)
    {
        info.file = m_script->file;
        info.nodeId = nodeIdByPc(m_pc);
        info.pc = m_pc;
    }
    return info;
}

void JZNodeEngine::pushStack(const FunctionDefine *func)
{
    if(m_stack.size() > 0)        
        m_stack.env().pc = m_pc;
        
    m_stack.push();    
    m_stack.env().func = func;    
    if(!func->isCFunction)
    {
        m_stack.env().pc = func->addr;
        m_pc = func->addr;
        m_script = getScript(func->script);
        Q_ASSERT(m_script);

        for(int i = 0; i < func->paramIn.size(); i++)
            m_stack.setVariable(Stack_User + i,m_regs[Reg_Call + i]);
    }
}

void JZNodeEngine::popStack()
{   
    m_stack.pop();
    if(m_stack.size() > 0)
    {
        m_pc = m_stack.env().pc;
        m_script = getScript(m_stack.env().func->script);
        Q_ASSERT(m_script);
    }
    else
    {
        m_pc = -1;
        m_script = nullptr;
    }
}

void JZNodeEngine::customEvent(QEvent *qevent)
{
    if (qevent->type() == JZEvent::Event)
    {
        JZEvent *event = (JZEvent*)qevent;
        if (event->eventType == Event_idlePause)
        {
            QVariantList in, out;
            call(&m_idleFunc, in, out);
        }
    }
}

void JZNodeEngine::dealEvent(JZEvent *event)
{
    JZNodeObject *obj = event->sender;
    auto it = m_connects.find(obj);
    if(it == m_connects.end())
        return;

    auto &list = it.value();
    for(int i = 0; i < list.size(); i++)
    {
        auto &connect = list[i];
        if(connect.eventType == event->eventType)
        {
            QVariantList out;
            setThis(QVariant::fromValue(connect.receiver));
            call(connect.handle,event->params,out);
        }
    }
}

bool JZNodeEngine::call(const QString &name,const QVariantList &in,QVariantList &out)
{    
    auto *func = function(name);
    Q_ASSERT(func);
    return call(func,in,out);
}

bool JZNodeEngine::call(const FunctionDefine *func,const QVariantList &in,QVariantList &out)
{
    while (true)
    {
        m_mutex.lock();
        if (m_status == Status_none)
        {
            updateStatus(Status_running);
            m_mutex.unlock();
            break;
        }
        else
        {
            Q_ASSERT(m_status == Status_idlePause);
            m_mutex.unlock();
        }
        QThread::msleep(10);
    }
    if (func == &m_idleFunc)
        return true;
        
    Q_ASSERT(func->paramIn.size() == in.size());
    g_engine = this;    
    for (int i = 0; i < in.size(); i++)
    {
        Q_ASSERT(JZNodeType::canConvert(JZNodeType::variantType(in[i]),func->paramIn[i].dataType));
        setReg(Reg_Call + i,in[i]);
    }

    try
    {
        if(func->isCFunction)
        {
            callCFunction(func);
        }
        else
        {
            pushStack(func);
            if(!run())
            {
                updateStatus(Status_none);
                return false;
            }
        }
    }
    catch(const std::exception& e)
    {
        JZNodeRuntimeError error;
        error.info = e.what();
        error.script = m_script->file;
        error.pc = m_pc;
        emit sigRuntimeError(error);

        updateStatus(Status_none);
        m_statusCommand = Command_none;
        return false;
    }

    out.clear();
    for (int i = 0; i < func->paramOut.size(); i++)
        out.push_back(getReg(Reg_Call + i));    

    updateStatus(Status_none);
    m_statusCommand = Command_none;
    return true;
}

QString JZNodeEngine::variableToString(const QVariant &v)
{
    if(isJZObject(v))
    {
        JZNodeObject *obj = toJZObject(v);
        if(obj->isString())
            return *((QString*)obj->cobj);
        else
            return QString();
    }
    else
        return v.toString();
}

QVariant JZNodeEngine::getParam(const JZNodeIRParam &param)
{       
    if(param.isLiteral())    
        return param.value;
    else if(param.isRef())
        return getVariable(param.ref());
    else if(param.isThis())
        return m_this;
    else
        return getReg(param.id());
}

void JZNodeEngine::setParam(const JZNodeIRParam &param,const QVariant &value)
{
    if(param.isId())    
        setReg(param.id(),value);
    else if(param.isRef())
        setVariable(param.ref(),value);
    else
    {
        Q_ASSERT(0);
    }        

    if(isWatch())
        emit sigParamChanged();
}

QVariant JZNodeEngine::getThis()
{
    return m_this;
}

void JZNodeEngine::setThis(QVariant v)
{
    m_this = v;
}

bool JZNodeEngine::splitMember(const QString &fullName,QString &objName,QString &memberName)
{
    int index = -1;
    if((index = fullName.indexOf(".")) >= 0)
    {
        objName = fullName.left(index);
        memberName = fullName.mid(index+1);
        return true;
    }
    return false;
}

QVariant *JZNodeEngine::getVariableRef(QString name)
{
    QString obj_name,param_name;
    if(!splitMember(name,obj_name,param_name))
        obj_name = name;

    QVariant *ref = nullptr;
    if(obj_name == "this")
    {
        ref = &m_this;
    }
    else
    {
        ref = m_stack.isEmpty()? nullptr : m_stack.getVariableRef(obj_name);
        if(!ref)  
        {
            auto it = m_global.find(obj_name);
            if(it == m_global.end())
                return nullptr;
            ref = &it.value();
        }                     
    }        
    if(!ref)
        return nullptr;
    
    if(param_name.isEmpty())
        return ref;
    else
    {
        JZNodeObject *obj = toJZObject(*ref);
        if (!obj)
            return nullptr;
        return obj->paramRef(param_name);
    }
}

QVariant JZNodeEngine::getVariable(QString name)
{    
    QVariant *ref = getVariableRef(name);
    if(!ref)
        throw std::runtime_error("no such variable " + name.toStdString());

    return *ref;
        
}

void JZNodeEngine::setVariable(QString name, const QVariant &value)
{
    QVariant *ref = getVariableRef(name);
    if(!ref)
        throw std::runtime_error("no such variable " + name.toStdString());

    *ref = value;
    objectChanged(nullptr, name);

    //connect
    if(isJZObject(value))
    {
        auto obj = toJZObject(value);
        connectVariableEvent(name, obj);
    }
}

void JZNodeEngine::connectClassEvent(JZNodeObject *obj)
{
    QVariant back = m_this;
    m_this = QVariant::fromValue(obj);

    QString className = obj->className();
    for (int i = 0; i < m_connectInfo.size(); i++)
    {
        auto &info = m_connectInfo[i];
        if (info.recv == className)
        {
            if (info.eventType == Event_paramChanged)
                continue;

            auto ref = getVariableRef(info.sender);                        
            if (ref && isJZObject(*ref))
            {                
                JZNodeObject *sender = toJZObject(*ref);
                JZObjectConnect connect;
                connect.sender = sender;
                connect.eventType = info.eventType;
                connect.receiver = obj;
                connect.handle = info.handle;
                m_connects[obj].push_back(connect);
            }
            else
            {
                ConnectCache cache;
                cache.sender = info.sender;
                if (info.sender.startsWith("this."))
                    cache.parentObject = obj;
                cache.eventType = info.eventType;
                cache.recvObject = obj;
                cache.handle = info.handle;
                m_connectCache.push_back(cache);
            }                                   
        }
    }
    m_this = back;
}

void JZNodeEngine::connectVariableEvent(QString name, JZNodeObject *obj)
{
    QStringList list = name.split(".");
    for (int i = list.size() - 1; i >= 1; i--)
    {
        QStringList tmp = list.mid(i);
        QStringList parent_list = list.mid(0,i);
        QString parent_name = parent_list.join(".");
        tmp.insert(0, "this");
        QString this_name = tmp.join(".");

        JZNodeObject *recv = toJZObject(getVariable(parent_name));
        connectVariableThis(this_name, obj, recv);
    }
        
    for (int i = 0; i < m_connectCache.size(); i++)
    {
        auto &cache = m_connectCache[i];
        JZNodeObject *sender = nullptr;
        if (cache.sender == name)
            sender = obj;
        else if (cache.sender.startsWith(name + "."))
        {
            QString param_name = cache.sender.mid(name.size() + 1);
            QVariant *v = obj->paramRef(param_name);
            sender = toJZObject(*v);
        }

        if(sender)
        { 
            JZObjectConnect connect;
            connect.sender = sender;
            connect.eventType = cache.eventType;
            connect.receiver = cache.recvObject;
            connect.handle = cache.handle;
            connectObject(connect);
        }
    }
}

void JZNodeEngine::connectVariableThis(QString name, JZNodeObject *obj, JZNodeObject *recv)
{
    for (int i = 0; i < m_connectInfo.size(); i++)
    {
        auto &info = m_connectInfo[i];
        if (name == info.sender)
        {
            JZObjectConnect connect;
            connect.sender = obj;
            connect.eventType = info.eventType;
            connect.receiver = recv;
            connect.handle = info.handle;
            connectObject(connect);
        }
    }
}

void JZNodeEngine::connectObject(JZObjectConnect connect)
{
    qDebug() << "sender:" << connect.sender << "recv:" << connect.receiver << "event:" << connect.handle->name;
    m_connects[connect.sender].push_back(connect);

    if (connect.sender->isInherits("object"))     
        connect.sender->connectSingles(connect.eventType);    
}

void JZNodeEngine::objectChanged(JZNodeObject *sender,const QString &name)
{
    QVariantList in,out;
    for(int i = 0; i < m_paramChangeHandle.size(); i++)
    {
        auto &handle = m_paramChangeHandle[i];
        if(handle.name == name)
        {
            setThis(QVariant::fromValue(handle.receiver));
            call(handle.handle,in,out);
        }
    }
}

void JZNodeEngine::print(const QString &log)
{
    emit sigLog(log);
}

QVariant JZNodeEngine::getReg(int id)
{       
    if(id >= Reg_Start)    
        return m_regs.value(id,QVariant());
    else
        return m_stack.getVariable(id);    
}

void JZNodeEngine::setReg(int id, const QVariant &value)
{
    if(id >= Reg_Start)
        m_regs.insert(id,value);
    else    
        m_stack.setVariable(id, value);            
}

JZNodeScript *JZNodeEngine::getScript(QString path)
{
    return m_program->script(path);
}

JZNodeScript *JZNodeEngine::getObjectScript(QString objName)
{
    return m_program->objectScript(objName);
}

void JZNodeEngine::connectParamChanged(JZNodeObject *obj,JZNodeScript *script)
{
    for(int event_idx = 0; event_idx < script->paramChanges.size(); event_idx++)
    {
        JZParamChangeHandle &handle = script->paramChanges[event_idx];         
        ParamChangeEvent change;
        change.name = handle.paramName;
        change.handle = &handle.function;
        change.receiver = obj;
        m_paramChangeHandle.push_back(change);        
    }
}



JZNodeObject* JZNodeEngine::getObject(QString name)
{    
    auto ref = getVariableRef(name);
    if (!ref)
        return nullptr;
    return toJZObject(*ref);
}

bool JZNodeEngine::isWatch()
{
    return false;
}

void JZNodeEngine::addWatch()
{

}

void JZNodeEngine::clearWatch()
{
    m_watchParam.clear();
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
    if (m_status == Status_none) 
    {
        updateStatus(Status_idlePause);

        JZEvent *event = new JZEvent();
        event->eventType = Event_idlePause;
        qApp->postEvent(this, event);
        return;
    }
    else if(m_status != Status_running)    
        return;    
    
    m_statusCommand = Command_pause;
    lock.unlock();
    waitCommand();
}

void JZNodeEngine::resume()
{
    QMutexLocker lock(&m_mutex);
    if (m_status == Status_idlePause)
    {
        updateStatus(Status_none);
        return;
    }
    else if(m_status != Status_pause)    
        return;    
    
    m_statusCommand = Command_resume;
    lock.unlock();
    m_waitCond.wakeOne();
    waitCommand();
}

void JZNodeEngine::stop()
{
    QMutexLocker lock(&m_mutex);
    if(m_status != Status_running && m_status != Status_pause)
        return;
        
    m_statusCommand = Command_stop;
    lock.unlock();
    if(m_status == Status_pause)
        m_waitCond.wakeOne();
    waitCommand();
}

void JZNodeEngine::stepIn()
{
    QMutexLocker lock(&m_mutex);
    if (m_status == Status_idlePause)
    {
        updateStatus(Status_none);
        return;
    }
    else if(m_status != Status_pause)
        return;

    int node_id = nodeIdByPc(m_pc);
    auto info = m_script->nodeInfo[node_id];
    if(info.node_type == Node_function)
    {
        m_breakStep.type = BreakPoint::stackEqual;
        m_breakStep.stack = m_stack.size() + 1;

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
    if (m_status == Status_idlePause)
    {
        updateStatus(Status_none);
        return;
    }
    else if(m_status != Status_pause)
        return;

    int node_id = nodeIdByPc(m_pc);
    m_breakStep.type = BreakPoint::nodeExit;   
    m_breakStep.file = m_script->file;
    m_breakStep.nodeId = node_id;    

    m_statusCommand = Command_resume;
    lock.unlock();
    m_waitCond.wakeOne();
    waitCommand();
}

void JZNodeEngine::stepOut()
{
    QMutexLocker lock(&m_mutex);
    if (m_status == Status_idlePause)
    {
        updateStatus(Status_none);
        return;
    }
    else if (m_status != Status_pause)
        return;

    m_breakStep.type = BreakPoint::stackEqual;
    m_breakStep.stack = m_stack.size() - 1;

    m_statusCommand = Command_resume;
    lock.unlock();
    m_waitCond.wakeOne();
    waitCommand();
}

void JZNodeEngine::callCFunction(const FunctionDefine *func)
{    
    QVariantList paramIn, paramOut;
    // get input
    auto &inList = func->paramIn;
    for (int i = 0; i < inList.size(); i++)
    {
        paramIn.push_back(getReg(Reg_Call + i));
        int inType = JZNodeType::variantType(paramIn.back());
        bool ret = JZNodeType::canConvert(inType,func->paramIn[i].dataType);
        if(!ret)
            throw std::runtime_error("type node match");
    }

    // call function
    pushStack(func);
    func->cfunc->call(paramIn,paramOut);
    popStack();

    // set output
    auto &outList = func->paramOut;
    for (int i = 0; i < outList.size(); i++)
        setReg(Reg_Call + i,paramOut[i]);

    if(func->name == "createObject")
    {
        auto obj = toJZObject(paramOut[0]);
        connectClassEvent(obj);
    }
}

const FunctionDefine *JZNodeEngine::function(QString name)
{
    FunctionDefine *func = m_program->function(name);
    if(func)
        return func;        
    return JZNodeFunctionManager::instance()->function(name);
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
        return a / b;
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
    case OP_bitand:
        return a & b;
    case OP_bitor:
        return a | b;
    case OP_bitxor:
        return a ^ b;
    default:
        throw std::runtime_error("un support operator");
        break;
    }
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
    case OP_le:
        return a <= b;
    case OP_ge:
        return a >= b;
    case OP_lt:
        return a < b;
    case OP_gt:
        return a > b;
    default:
        throw std::runtime_error("un support operator");
        break;
    }
    return 0;
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
                throw std::runtime_error("un support operator");
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
            if (JZNodeType::isObject(dataType1) && JZNodeType::isObject(dataType2))
            {
                bool ret = (toJZObject(a) == toJZObject(b));
                if (op == OP_eq)
                    return ret;
                else
                    return !ret;
            }
            else
                throw std::runtime_error("un support operator");
        }
        else
            throw std::runtime_error("un support operator");
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
        m_statusCommand = Command_none;
        updateStatus(Status_none);
        m_mutex.unlock();
        return true;
    }
    else if(m_statusCommand == Command_pause)
    {
        m_statusCommand = Command_none;
        wait = true;
    }
    else if(op_list[m_pc]->type == OP_nodeId)
    {
        JZNodeIRNodeId *ir_node = dynamic_cast<JZNodeIRNodeId*>(op_list[m_pc].data());
        int node_id = ir_node->id;
        int stack = m_stack.size();
        auto breakTriggred = [this](BreakPoint &pt,const QString &filepath,int stack,int node_id,bool node_flow)->bool
        {
            if(pt.type == BreakPoint::nodeEnter && pt.file == filepath && node_id == pt.nodeId)
                return true;
            else if (pt.type == BreakPoint::nodeExit)
            {
                bool pre_node_flow = m_script->nodeInfo[pt.nodeId].isFlow;
                if (pt.file != filepath)
                    return true;
                else if (node_id != pt.nodeId && (!pre_node_flow || (pre_node_flow && node_flow)))
                    return true;
            }
            else if(pt.type == BreakPoint::stackEqual && stack == pt.stack)
                return true;

            return false;
        };

        bool node_flow = m_script->nodeInfo[node_id].isFlow;
        if(breakTriggred(m_breakStep,m_script->file, stack,node_id, node_flow))
        {
            m_breakStep.clear();
            wait = true;
        }
        else
        {
            for(int i = 0; i < m_breakPoints.size(); i++)
            {
                auto pt = m_breakPoints[i];
                if(breakTriggred(pt,m_script->file, stack,node_id, node_flow))
                {
                    wait = true;
                    break;
                }
            }
        }        
    }
    if(wait)
    {        
        m_statusCommand = Command_none;        
        updateStatus(Status_pause);
        m_waitCond.wait(&m_mutex);
        if(m_statusCommand == Command_stop)
        {
            m_statusCommand = Command_none;
            updateStatus(Status_none);
            m_mutex.unlock();
            return true;
        }
        else
        {
            m_statusCommand = Command_none;
            updateStatus(Status_running);            
            m_mutex.unlock();
        }
    }
    else
    {
        m_mutex.unlock();
    }
    return false;
}

void JZNodeEngine::updateStatus(int status)
{
    if (m_status != status)
    {
        m_status = status;
        sigStatusChanged(m_status);
    }
}

bool JZNodeEngine::run()
{    
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
        case OP_jmp:
        case OP_je:
        case OP_jne:
        {
            JZNodeIRJmp *ir_jmp = (JZNodeIRJmp*)op;
            if(op_type == OP_jmp)
                m_pc = ir_jmp->jmpPc;
            else
            {
                bool flag = getReg(Reg_Cmp).toBool();
                if(op_type == OP_je)
                    m_pc = flag? ir_jmp->jmpPc : m_pc+1;
                else
                    m_pc = flag? m_pc+1 : ir_jmp->jmpPc;
            }
            continue;
        }
        case OP_alloc:
        {
            JZNodeIRAlloc *ir_alloc = (JZNodeIRAlloc*)op;
            if(ir_alloc->allocType == JZNodeIRAlloc::Heap)
                m_global[ir_alloc->name] = ir_alloc->value;
            else
                m_stack.setVariable(ir_alloc->name,ir_alloc->value);
            break;
        }
        case OP_set:
        {
            JZNodeIRSet *ir_set = (JZNodeIRSet*)op;
            setParam(ir_set->dst,getParam(ir_set->src));
            break;
        }
        case OP_get:
        {
            break;
        }
        case OP_call:
        {
            JZNodeIRCall *ir_call = (JZNodeIRCall*)op;
            QString function_name = variableToString(getParam(ir_call->function));
            auto func = function(function_name);
            Q_ASSERT(func);
            if(func->isCFunction)
                callCFunction(func);
            else
                pushStack(func);
            break;
        }
        case OP_return:
        {
            popStack();
            if(m_stack.size() == 0)
                goto RunEnd;
            break;
        }
        case OP_exit:
            goto RunEnd;
        default:
            Q_ASSERT(0);
            break;
        }
        m_pc++;
    }

RunEnd:        
    return true;
}
