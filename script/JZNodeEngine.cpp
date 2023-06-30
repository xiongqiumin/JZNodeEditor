#include "JZNodeEngine.h"
#include "JZNodeFunctionManager.h"
#include "JZEvent.h"
#include <QPushButton>
#include <QApplication>
#include "JZNodeVM.h"
#include <math.h>

void JZObjectEvent(JZEvent *event)
{
    g_engine->dealEvent(event);
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

int Stack::size()
{
    return m_stack.size();
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
    m_stack.push_back(QMap<int,QVariant>());
    m_env.push_back(RunnerEnv());
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
    return m_stack[level].value(id,QVariant());
}

void Stack::setVariable(int level,int id, const QVariant &value)
{
    m_stack[level][id] = value;
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
    status = None;
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
    m_script = nullptr;
    m_pc = -1;
    m_breaknodeId = -1;
    m_status = Status_none;
    m_statusCommand = Status_none;    
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

void JZNodeEngine::init()
{
    JZNodeFunctionManager::instance()->clearUserReigst();
    JZNodeObjectManager::instance()->clearUserReigst();

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
            m_global[it.key()] = QVariant();
        }
        it++;
    }        

    // connect events
    auto script_list = m_program->scriptList();
    for(int i = 0; i < script_list.size(); i++)
    {
        connectScript("",nullptr,script_list[i]);
        connectParamChanged(nullptr,script_list[i]);
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
    for(int i = m_pc; i >=0 ; i--)
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
        if(m_breaknodeId != -1)
            info.nodeId = m_breaknodeId;
        else
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
    Q_ASSERT(func->paramIn.size() == in.size());
    g_engine = this;
    m_status = Status_running;               
    for (int i = 0; i < in.size(); i++)
    {
        Q_ASSERT(JZNodeType::canConvert(JZNodeType::variantType(in[i]),func->paramIn[i].dataType));
        setReg(Reg_Call + i,in[i]);
    }

    if(func->isCFunction)
        callCFunction(func);
    else
    {
        pushStack(func);
        if(!run())
            return false;
    }
    out.clear();
    for (int i = 0; i < func->paramOut.size(); i++)
        out.push_back(getReg(Reg_Call + i));
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
    if(splitMember(name,obj_name,param_name))
    {
        JZNodeObject *obj = nullptr;
        if(obj_name == "this")
        {
            obj = toJZObject(m_this);
        }
        else
        {
            auto it = m_global.find(obj_name);
            if(it == m_global.end())
                return nullptr;
            obj = toJZObject(it.value());
        }
        Q_ASSERT(obj);
        return obj->paramRef(param_name);
    }
    else
    {
        if(name == "this")
        {
            Q_ASSERT(m_this.isValid());
            return &m_this;
        }

        auto it = m_global.find(name);
        if(it != m_global.end())
            return &it.value();
        else
            return nullptr;
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
        QStringList list = name.split(".");
        JZNodeObject *parent = getObject(list[0]);
        for(int i = 0; i < list.size() - 1; i++)
        {
            auto script = getObjectScript(parent->className());
            connectParent(obj,name,parent,script);
            parent = toJZObject(parent->param(list[i+1]));
        }
        connectSender(name,obj);
    }
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

void JZNodeEngine::connectScript(QString objName,JZNodeObject *obj,JZNodeScript *script)
{
    qDebug() << objName << "recv:" << obj << "script:" << script->file;
    auto getParamObject = [this,obj](const QString &name)->JZNodeObject*
    {                
        if(!obj)
            return getObject(name);
        else
        {
            if(name == "this")
                return obj;
            if(!name.startsWith("this."))
                return nullptr;

            QString param_name = name.mid(5);
            QVariant *v = obj->paramRef(param_name);
            if(v)
                return toJZObject(*v);
            else
                return nullptr;
        }
    };

    for(int event_idx = 0; event_idx < script->events.size(); event_idx++)
    {
        auto &handle = script->events[event_idx];                
        if((objName.isEmpty() && handle.sender.isEmpty())                
                || (handle.sender == objName))
        {            
            JZObjectConnect connect;
            if(!handle.sender.isEmpty())
            {
                connect.sender = getParamObject(handle.sender);
                if(!connect.sender)
                    continue;
                if(connect.sender->isInherits("object"))
                    connect.sender->connectSingles(handle.type);
            }
            else
            {
                connect.sender = nullptr;
            }
            connect.receiver = obj;
            connect.handle = &handle.function;
            connect.eventType = handle.type;

            QString recv_name = obj? obj->className() : "none";
            qDebug() << "event:" << handle.function.name << "sender " << objName << ",recv" << recv_name;

            m_connects[connect.sender].push_back(connect);
        }
    }
}

void JZNodeEngine::connectRecv(QString objName,JZNodeObject *obj,JZNodeObject *recv)
{
    auto script = getObjectScript(recv->className());
    if(script)
        connectScript(objName,recv,script);

    auto it = recv->params.begin();
    while(it != recv->params.end())
    {
        if(isJZObject(it.value()))
        {
            JZNodeObject *sub = toJZObject(it.value());
            connectRecv(objName,obj,sub);
        }
        it++;
    }
}


void JZNodeEngine::connectSender(QString objName,JZNodeObject *obj)
{       
    //global as send
    auto script_list = m_program->scriptList();
    for(int i = 0; i < script_list.size(); i++)
        connectScript(objName,nullptr,script_list[i]);

    auto it = m_global.begin();
    while(it != m_global.end())
    {
        if(isJZObject(it.value()))
        {
            JZNodeObject *recv = toJZObject(it.value());
            connectRecv(objName,obj,recv);
        }
        it++;
    }

    //connect child
    it = obj->params.begin();
    while(it != obj->params.end())
    {
        if(isJZObject(it.value()))
        {
            JZNodeObject *sub = toJZObject(it.value());
            connectSender(objName + "." + it.key(),sub);
        }
        it++;
    }
}

void JZNodeEngine::connectSelf(JZNodeObject *obj)
{
    auto script = getObjectScript(obj->className());
    if(script)
        connectParent(obj,"this",obj,script);
}

void JZNodeEngine::connectParent(JZNodeObject *obj,QString name,JZNodeObject* parent,JZNodeScript *script)
{
    connectScript(name,parent,script);

    auto it = obj->params.begin();
    while(it != obj->params.end())
    {
        if(isJZObject(it.value()))
        {
            auto sub_obj = toJZObject(it.value());
            connectParent(sub_obj,name + "." + it.key(), parent,script);
        }
        it++;
    }
}

JZNodeObject* JZNodeEngine::getObject(QString name)
{    
    QStringList list = name.split(".");
    auto it = m_global.find(list[0]);
    if(it == m_global.end())
        return nullptr;
    if(!isJZObject(it.value()))
        return nullptr;

    JZNodeObject *obj = toJZObject(it.value());
    if(list.size() == 1)
        return obj;

    QString param_name = name.mid(list[0].size() + 1);
    auto ref = obj->paramRef(param_name);
    if(ref)
        return toJZObject(*ref);
    else
        return nullptr;
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

void JZNodeEngine::waitStatus(int status)
{
    while(true)
    {   
        {
            QMutexLocker locker(&m_mutex);
            if(m_status == status || m_status == Status_none)
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
    if(m_status != Status_running)
        return;
    
    m_statusCommand = Status_pause;
    lock.unlock();
    waitStatus(Status_pause);
}

void JZNodeEngine::resume()
{
    QMutexLocker lock(&m_mutex);
    if(m_status != Status_pause)
        return;
    
    m_waitCond.release();   
    waitStatus(Status_running);
}

void JZNodeEngine::stop()
{
    QMutexLocker lock(&m_mutex);
    if(m_status != Status_running && m_status != Status_pause)
        return;
        
    m_statusCommand = Status_stop;
    if(m_status == Status_pause)
        m_waitCond.release();    
    lock.unlock();
    waitStatus(Status_none);
}

void JZNodeEngine::stepIn()
{
    QMutexLocker lock(&m_mutex);
    if(m_status != Status_pause)
        return;

    auto info = m_script->nodeInfo[m_breaknodeId];
    if(info.node_type == Node_function)
    {
        m_breakStep.type = BreakPoint::stackEqual;
        m_breakStep.stack = m_stack.size() + 1;
    }
    else
    {
        lock.unlock();
        stepOver();
        return;
    }
    m_waitCond.release();
    waitStatus(Status_running);
}

void JZNodeEngine::stepOver()
{
    QMutexLocker lock(&m_mutex);    
    if(m_status != Status_pause)
        return;

    m_breakStep.type = BreakPoint::nodeExit;   
    m_breakStep.file = m_script->file;
    m_breakStep.nodeId = m_breaknodeId;
    m_waitCond.release();
    waitStatus(Status_running);
}

void JZNodeEngine::stepOut()
{
    QMutexLocker lock(&m_mutex);
    if(m_status != Status_pause)
        return;

    m_breakStep.type = BreakPoint::stackEqual;
    m_breakStep.stack = m_stack.size() - 1;
    m_waitCond.release();
    waitStatus(Status_running);
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
        connectSelf(obj);
    }
}

const FunctionDefine *JZNodeEngine::function(QString name)
{
    FunctionDefine *func = m_program->function(name);
    if(func)
        return func;        
    return JZNodeFunctionManager::instance()->function(name);
}

QVariant JZNodeEngine::dealExpr(QVariant &a,QVariant &b,int op)
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
        switch (op)
        {
        case OP_add:
            return a.toInt() + b.toInt();
        case OP_sub:
            return a.toInt() - b.toInt();
        case OP_mul:
            return a.toInt() * b.toInt();
        case OP_div:
            return a.toInt() / b.toInt();
        case OP_mod:
            return a.toInt() % b.toInt();
        case OP_eq:
            return a.toInt() == b.toInt();
        case OP_ne:
            return a.toInt() != b.toInt();
        case OP_le:
            return a.toInt() <= b.toInt();
        case OP_ge:
            return a.toInt() >= b.toInt();
        case OP_lt:
            return a.toInt() < b.toInt();
        case OP_gt:
            return a.toInt() > b.toInt();
        case OP_and:
            return a.toInt() && b.toInt();
        case OP_or:
            return a.toInt() || b.toInt();
        case OP_bitand:
            return a.toInt() & b.toInt();
        case OP_bitor:
            return a.toInt() | b.toInt();
        case OP_bitxor:
            return a.toInt() ^ b.toInt();
        default:
            throw std::runtime_error("un support operator");
            break;
        }
    }
    else if(JZNodeType::isNumber(dataType1) && JZNodeType::isNumber(dataType2))
    {
        switch (op)
        {
        case OP_add:
            return a.toDouble() + b.toDouble();
        case OP_sub:
            return a.toDouble() - b.toDouble();
        case OP_mul:
            return a.toDouble() * b.toDouble();
        case OP_div:
            return a.toDouble() / b.toDouble();
        case OP_mod:
            return fmod(a.toDouble(),b.toDouble());
        case OP_eq:
            return a.toDouble() == b.toDouble();
        case OP_ne:
            return a.toDouble() != b.toDouble();
        case OP_le:
            return a.toDouble() <= b.toDouble();
        case OP_ge:
            return a.toDouble() >= b.toDouble();
        case OP_lt:
            return a.toDouble() < b.toDouble();
        case OP_gt:
            return a.toDouble() > b.toDouble();
        case OP_and:
            return a.toDouble() && b.toDouble();
        case OP_or:
            return a.toDouble() || b.toDouble();
        default:
            throw std::runtime_error("un support operator");
            break;
        }
    }
    else
    {
        throw std::runtime_error("un support operator");
    }

    return QVariant();
}

bool JZNodeEngine::run()
{    
    while (true)
    {                   
        Q_ASSERT(m_script);
        
        bool wait = false;                  
        auto &op_list = m_script->statmentList;  
        // check pause
        {
            QMutexLocker lock(&m_mutex);            
            if(m_statusCommand == Status_pause)
            {                
                m_statusCommand = Status_none;
                wait = true;
            }
            else
            {                
                int node_id = op_list[m_pc]->nodeId;
                int stack = m_stack.size();
                auto breakTriggred = [](BreakPoint &pt,const QString &filepath,int pc,int node_id,int stack)->bool
                {                    
                    if(pt.type == BreakPoint::nodeEnter && pt.file == filepath && node_id == pt.nodeId)
                        return true;
                    else if(pt.type == BreakPoint::nodeExit && (pt.file != filepath || node_id == pt.nodeId))
                        return true;
                    else if(pt.type == BreakPoint::stackEqual && stack == pt.stack)
                        return true;        

                    return false;
                };
                       
                if(breakTriggred(m_breakStep,m_script->file,m_pc,node_id,stack))
                {                    
                    m_breakStep.clear();
                    wait = true;
                }
                else
                {
                    for(int i = 0; i < m_breakPoints.size(); i++)
                    {
                        auto pt = m_breakPoints[i];
                        if(breakTriggred(pt,m_script->file,m_pc,node_id,stack))
                        {
                            wait = true;
                            break;       
                        }                                                            
                    }
                }
                if(wait)            
                    m_breaknodeId = node_id;
            }            
        }
        // check stop
        if(m_statusCommand == Status_stop)
        {
            m_statusCommand = Status_none;
            m_status = Status_none;
            return false;
        }
        if(wait)
        {
            if(m_breaknodeId == -1)
                m_breaknodeId = nodeIdByPc(m_pc);

            m_status = Status_pause;
            m_waitCond.acquire();
            if(m_statusCommand == Status_stop)
            {
                m_statusCommand = Status_none;
                m_status = Status_none;
                return false;
            }
            m_status = Status_running;
            m_breaknodeId = -1;
        }

        try
        {
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
        catch(const std::exception& e)
        {
            JZNodeRuntimeError error;
            error.info = e.what();
            error.script = m_script->file;
            error.pc = m_pc;
            emit sigRuntimeError(error);
            goto RunEnd;
        }
    }

RunEnd:
    m_breakStep.clear();
    m_status = Status_none;
    return true;
}
