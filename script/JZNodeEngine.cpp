﻿#include "JZNodeEngine.h"
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

void JZScriptInvoke(const QString &function, const QVariantList &in, QVariantList &out)
{
    g_engine->call(function, in, out);
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

RunnerEnv &Stack::env(int index)
{
    return m_env[index];
}

Stack::StackVariant *Stack::stackVariable(int index)
{
    return &m_stack[index];
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
    m_script = nullptr;
    m_pc = -1;    
    m_breakNodeId = -1;
    m_debug = false;
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
    auto function_list = m_program->functionDefines();
    for (int i = 0; i < function_list.size(); i++)
        JZNodeFunctionManager::instance()->registFunction(function_list[i]);

    auto define_list = m_program->objectDefines();    
    for(int i = 0; i < define_list.size(); i++)
        JZNodeObjectManager::instance()->regist(define_list[i]);

    // init variable    
    auto global_params = m_program->globalVariables();
    auto it = global_params.begin();
    while(it != global_params.end() )
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
            if (JZEvent::isQtEvent(event.type))
                continue;

            if (!event.sender.startsWith("this.") && script->className.isEmpty())
            {
                ConnectCache cache;
                cache.eventType = event.type;
                cache.parentObject = nullptr;
                cache.recvObject = nullptr;
                cache.sender = event.sender;
                cache.handle = &event.function;
                m_connectCache.push_back(cache);
            }
            else
            {
                ConnectInfo info;
                info.sender = event.sender;
                info.eventType = event.type;
                info.recv = script->className;
                info.handle = &event.function;
                m_connectInfo.push_back(info);
            }
        }
    }    

    // init value
    for(int i = 0; i < m_paramChangeHandle.size(); i++)
    {
        QVariantList in,out;
        auto &handle = m_paramChangeHandle[i];        
        call(handle.handle,in,out);
    }
}   

int JZNodeEngine::breakNodeId(int pc, int skip_id)
{
    auto &op_list = m_script->statmentList;
    for (int i = pc; i >= 0; i--)
    {
        if (op_list[i]->type == OP_nodeId)
        {
            JZNodeIRNodeId *ir_node = (JZNodeIRNodeId *)op_list[i].data();
            if (ir_node->id == skip_id)
                continue;

            int rng_idx = m_script->nodeInfo[ir_node->id].indexOfRange(m_pc);
            if (rng_idx >= 0)
                return ir_node->id;
        }
    }
    Q_ASSERT(0);
    return 0;
}

int JZNodeEngine::nodeIdByPc(JZNodeScript *script, int pc)
{
    auto &op_list = script->statmentList;
    for(int i = pc; i >= 0 ; i--)
    {
        if(op_list[i]->type == OP_nodeId)
        {
            JZNodeIRNodeId *ir_node = (JZNodeIRNodeId *)op_list[i].data();
            int rng_idx = script->nodeInfo[ir_node->id].indexOfRange(pc);
            if (rng_idx >= 0)
                return ir_node->id;
        }
    }
    Q_ASSERT(0);
    return 0;
}

int JZNodeEngine::nodeIdByPc(int m_pc)
{
    return nodeIdByPc(m_script,m_pc);    
}

JZNodeRuntimeInfo JZNodeEngine::runtimeInfo()
{   
    JZNodeRuntimeInfo info;
    QMutexLocker lock(&m_mutex);
    info.status = m_status;
    if (m_status == Status_idlePause)
    {
        JZNodeRuntimeInfo::Stack s;
        s.file = "idle";
        info.stacks.push_back(s);
    }
    else if(m_script)
    {        
        for (int i = 0; i < m_stack.size(); i++)
        {
            JZNodeRuntimeInfo::Stack s;
            auto &env = m_stack.env(i);
            s.function = env.func->fullName();
            if (env.script)
            {
                s.file = env.script->file;
                if (i == m_stack.size() - 1 && m_breakNodeId != -1)
                    s.nodeId = m_breakNodeId;
                else
                    s.nodeId = nodeIdByPc(env.script, env.pc);
            }            
            s.pc = env.pc;
            info.stacks.push_back(s);
        }        
    }
    return info;
}

void JZNodeEngine::pushStack(const FunctionDefine *func)
{
    if (m_stack.size() > 0)
    {
        m_stack.env().pc = m_pc;
        m_stack.env().object = toJZObject(m_this);
        m_stack.env().script = m_script;
    }
        
    m_stack.push();    
    m_stack.env().func = func;    
    if(!func->isCFunction)
    {        
        auto runtime = m_program->runtimeInfo(func->fullName());
        m_pc = runtime->addr;
        m_script = getScript(runtime->file);
        Q_ASSERT(m_script);

        m_stack.env().pc = m_pc;
        m_stack.env().script = m_script;        

        for (int i = 0; i < runtime->localVariables.size(); i++)
        {
            auto &def = runtime->localVariables[i];
            m_stack.setVariable(def.name, def.initialValue());
        }

        for (int i = 0; i < func->paramIn.size(); i++)
            m_stack.setVariable(Stack_User + i, m_regs[Reg_Call + i]);
    }    
}

void JZNodeEngine::popStack()
{   
    m_stack.pop();
    if(m_stack.size() > 0)
    {
        m_pc = m_stack.env().pc;
        m_script = m_stack.env().script;
        setThis(QVariant::fromValue(m_stack.env().object));
        Q_ASSERT(m_script);
    }
    else
    {
        m_pc = -1;
        m_script = nullptr;
        setThis(QVariant());
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
    if (JZEvent::isQtEvent(event->eventType))
        dealQtEvent(event);
    else
        dealSingleEvent(event);    
}

void JZNodeEngine::dealQtEvent(JZEvent *event)
{
    auto script = getObjectScript(event->sender->className());
    if (!script)
        return;
        
    QVariantList in = event->params;
    QVariantList out;
    for (int i = 0; i < script->events.size(); i++)
    {
        m_this = QVariant::fromValue(event->sender);

        JZEventHandle &handle = script->events[i];
        if(handle.type == event->eventType)
            call(&handle.function, in , out);
    }    
}

void JZNodeEngine::dealSingleEvent(JZEvent *event)
{
    JZNodeObject *obj = event->sender;
    auto it = m_connects.find(obj);
    if (it == m_connects.end())
        return;

    auto &list = it.value();
    for(int i = 0; i < list.size(); i++)
    {
        auto &connect = list[i];
        if(connect.eventType == event->eventType)
        {
            m_this = QVariant::fromValue(connect.receiver);

            QVariantList out;            
            call(connect.handle,event->params,out);
        }
    }
}

bool JZNodeEngine::checkIdlePause(const FunctionDefine *func)
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
    Q_ASSERT(func);
    return call(func,in,out);
}

bool JZNodeEngine::call(const FunctionDefine *func,const QVariantList &in,QVariantList &out)
{
    if (checkIdlePause(func))
        return false;    
        
    Q_ASSERT(in.size() == func->paramIn.size());
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
        m_stack.env().pc = m_pc;                                
        m_statusCommand = Command_none;
        updateStatus(Status_error);
        m_mutex.unlock();

        JZNodeRuntimeError error;
        error.error = e.what();
        error.info = runtimeInfo();
        emit sigRuntimeError(error);

        if (m_debug)
        {
            m_mutex.lock();
            m_waitCond.wait(&m_mutex);
            m_statusCommand = Command_none;
            m_mutex.unlock();
        }

        return false;
    }

    out.clear();    
    for (int i = 0; i < func->paramOut.size(); i++)
        out.push_back(getReg(Reg_Call + i));    

    m_regs.clear();
    if (m_stack.size() == 0)
    {
        updateStatus(Status_none);
        m_statusCommand = Command_none;
        m_breakStep.clear();
    }
    return true;
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

Stack *JZNodeEngine::stack()
{
    return &m_stack;
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
    if (name == "this")
    {
        m_this = value;
        return;
    }

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

    // event
    auto script = getObjectScript(className);
    if (script)
    {
        for (int i = 0; i < script->events.size(); i++)
        {
            JZEventHandle &event = script->events[i];
            if (event.type >= Event_paint && event.type <= Event_mouseRelease)
            {
                obj->meta()->super()->cMeta.addEventFilter(obj, event.type);
            }
        }
    }

    // single 
    for (int i = 0; i < m_connectInfo.size(); i++)
    {
        auto &info = m_connectInfo[i];
        if (info.recv == className)
        {
            if (info.eventType == Event_paramChanged)
                continue;

            JZNodeObject *sender = nullptr;
            auto ref = getVariableRef(info.sender);

            if (ref && (sender = toJZObject(*ref)))
            {                                
                JZObjectConnect connect;
                connect.sender = sender;
                connect.eventType = info.eventType;
                connect.receiver = obj;
                connect.handle = info.handle;
                connectObject(connect);
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
/*
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
*/

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
            if(v)
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

    if (connect.sender->isInherits("Object"))     
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
    for(int event_idx = 0; event_idx < script->paramChangeEvents.size(); event_idx++)
    {
        JZParamChangeHandle &handle = script->paramChangeEvents[event_idx];
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

void JZNodeEngine::setDebug(bool flag)
{
    m_debug = flag;
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
    if (m_status != Status_none && m_status != Status_running)
        return;

    if (m_status == Status_none) 
    {
        JZEvent *event = new JZEvent();
        event->eventType = Event_idlePause;
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
    if(m_status == Status_error || m_status == Status_none)
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
    if (m_status != Status_pause)
        return;

    int node_id = -1;
    if (m_breakNodeId != -1)
        node_id = m_breakNodeId;
    else
        node_id = nodeIdByPc(m_pc);

    m_breakStep.type = BreakPoint::stepOver;
    m_breakStep.file = m_script->file;
    m_breakStep.nodeId = node_id;
    m_breakStep.stack = m_stack.size();    

    int rng_idx = m_script->nodeInfo[node_id].indexOfRange(m_pc);    
    m_breakStep.range = m_script->nodeInfo[node_id].pcRanges[rng_idx];

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
    m_breakStep.stack = m_stack.size() - 1;

    m_statusCommand = Command_resume;
    lock.unlock();
    m_waitCond.wakeOne();
    waitCommand();
}

void JZNodeEngine::checkFunction(const FunctionDefine *func)
{
    // get input
    auto &inList = func->paramIn;
    for (int i = 0; i < inList.size(); i++)
    {
        QVariant v = getReg(Reg_Call + i);
        int inType = JZNodeType::variantType(v);
        bool ret = JZNodeType::canConvert(inType, func->paramIn[i].dataType);
        if (!ret)            
            throw std::runtime_error(qUtf8Printable(func->paramIn[i].name + " type node match"));
        if (JZNodeType::isObject(func->paramIn[i].dataType) && JZNodeType::isNullObject(v))
            throw std::runtime_error(qUtf8Printable(func->paramIn[i].name + " object is nullptr"));
    }
}

void JZNodeEngine::callCFunction(const FunctionDefine *func)
{    
    QVariantList paramIn, paramOut;
    // get input
    auto &inList = func->paramIn;
    for (int i = 0; i < inList.size(); i++)    
        paramIn.push_back(getReg(Reg_Call + i));  

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

void JZNodeEngine::unSupportOp(int a, int b, int op)
{    
    QString error = QString("不支持的操作,操作符%1,数据类型%2,%3").arg(JZNodeType::opName(op), 
        JZNodeType::typeToName(a), JZNodeType::typeToName(b));

    throw std::runtime_error(qUtf8Printable(error));
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
            if (JZNodeType::isObject(dataType1) && JZNodeType::isObject(dataType2))
            {
                bool ret = (toJZObject(a) == toJZObject(b));
                if (op == OP_eq)
                    return ret;
                else
                    return !ret;
            }
            else
                unSupportOp(dataType1, dataType2, op);
        }
        else if (op == OP_not)
        {
            if (JZNodeType::isObject(dataType1))
            {
                auto obj = toJZObject(a);
                return !obj;
            }
            else
                unSupportOp(dataType1, dataType2, op);
        }
        else
            unSupportOp(dataType1, dataType2, op);
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
    else 
    {
        int node_id = -1;
        if (op_list[m_pc]->type == OP_nodeId)
        {
            JZNodeIRNodeId *ir_node = dynamic_cast<JZNodeIRNodeId*>(op_list[m_pc].data());
            node_id = ir_node->id;
        }

        int stack = m_stack.size();
        auto breakTriggred = [this](BreakPoint &pt,const QString &filepath,int stack,int pc,int node_id)->bool
        {
            if(pt.type == BreakPoint::nodeEnter && pt.file == filepath && node_id == pt.nodeId)
                return true;
            else if (pt.type == BreakPoint::stepOver)
            {                
                if (stack < pt.stack)
                    return true;
                else if (pt.file == filepath)
                {
                    if (node_id >= 0 && node_id != pt.nodeId)
                    {
                        m_breakNodeId = node_id;
                        return true;
                    }
                    else if(m_pc < pt.range.start || m_pc >= pt.range.end)
                    {             
                        m_breakNodeId = breakNodeId(pc, pt.nodeId);
                        return true;
                    }
                }                                         
            }
            else if(pt.type == BreakPoint::stackEqual && stack == pt.stack)
                return true;

            return false;
        };
        
        if(m_breakStep.type != BreakPoint::none 
            && breakTriggred(m_breakStep, m_script->file, stack, m_pc, node_id))
        {
            m_breakStep.clear();
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
        m_stack.env().pc = m_pc;
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
        case OP_jmp:
        case OP_je:
        case OP_jne:
        {
            JZNodeIRJmp *ir_jmp = (JZNodeIRJmp*)op;
            int jmpPc = getParam(ir_jmp->jmpPc).toInt();
            Q_ASSERT(jmpPc > 0 && jmpPc < op_list.size());
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
            QString function_name = getParam(ir_call->function).toString();
            auto func = function(function_name);
            Q_ASSERT(func);            

            if(func->isCFunction)
                callCFunction(func);
            else
            {
                checkFunction(func);
                pushStack(func);
                continue;
            }
            break;
        }
        case OP_return:
        {
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
