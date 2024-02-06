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

QVariant JZConvert(const QVariant &in, int type)
{
    return JZNodeType::matchValue(type,in);
}

void JZObjectDelete(JZNodeObject *obj)
{
    if(g_engine)
        g_engine->objectDelete(obj);
}

QVariant JZObjectCreate(QString name)
{
    int id = JZNodeObjectManager::instance()->getClassId(name);
    JZNodeObject *obj = JZNodeObjectManager::instance()->create(id);
    g_engine->objectCreate(obj);    

    JZNodeObjectPtr ptr = JZNodeObjectPtr(obj, JZObjectDelete);
    return QVariant::fromValue(ptr);
}

void JZNodeInitGlobal(QString name, const QVariant &v)
{
    g_engine->initGlobal(name, v);
}

void JZNodeInitLocal(QString name, const QVariant &v)
{
    g_engine->initLocal(name, v);
}

void JZObjectSlot(JZEvent *event)
{
    if (g_engine)
        g_engine->dealSlot(event);
}

void JZObjectEvent(JZEvent *event)
{
    if(g_engine)
        g_engine->dealEvent(event);
}

void JZObjectConnect(JZNodeObject *sender, int single, JZNodeObject *recv, QString function)
{
    g_engine->connectEvent(sender, single, recv, function);
}

void JZObjectDisconnect(JZNodeObject *sender, int single, JZNodeObject *recv, QString function)
{
    g_engine->disconnectEvent(sender, single, recv, function);
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

void JZWidgetValueChanged(QWidget *w)
{
    g_engine->widgetValueChanged(w);
}

void JZWidgetBind(JZNodeObject *obj, QString param_name)
{
    auto w = JZObjectCast<QWidget>(obj);
    auto ref = g_engine->getVariableRef(param_name);
    g_engine->widgetBind(w, ref);
}

void JZWidgetUnBind(JZNodeObject *obj)
{
    auto w = JZObjectCast<QWidget>(obj);    
    g_engine->widgetUnBind(w);
}

void JZWidgetUnBindNotify(QWidget *w)
{
    g_engine->widgetUnBindNotify(w);
}

QVariant JZUiToData(JZNodeObject *sender)
{    
    QVariant v;
    if (sender->isInherits("Widget"))
    {
        QWidget *w = (QWidget*)(sender->cobj());
        JZNodeQtBind::uiToData(w, v);
    }
    return v;
}

void JZDataToUi(const QVariant &v,JZNodeObject *sender)
{    
    if (sender->isInherits("Widget"))
    {
        QWidget *w = (QWidget*)(sender->cobj());
        JZNodeQtBind::dataToUi(v,w);
    }        
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
    locals[name] = JZVariantPtr(new QVariant(value));
}

void RunnerEnv::initVariable(int id, const QVariant &value)
{
    stacks[id] = JZVariantPtr(new QVariant(value));
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
JZNodeEngine::ConnectInfo::ConnectInfo()
{
    eventType = Event_none;
    sender = nullptr;
    receiver = nullptr;
    handle = nullptr;    
}

//JZObjectInfo
JZNodeEngine::JZObjectInfo::JZObjectInfo()
{
    connectQueue = 0;
}

//ParamChangeEvent
JZNodeEngine::ParamChangeInfo::ParamChangeInfo()
{
    recv = nullptr;    
}

//VariantInfo
JZNodeEngine::VariantInfo::VariantInfo()
{
    bindWidget = nullptr;
    bindValueCache = nullptr;
}

// JZNodeEngine
JZNodeEngine *g_engine = nullptr;

void JZNodeEngine::regist()
{
    JZNodeFunctionManager::instance()->registCFunction("convert", true, jzbind::createFuncion(JZConvert));
    JZNodeFunctionManager::instance()->registCFunction("createObject", true, jzbind::createFuncion(JZObjectCreate));

    JZNodeFunctionManager::instance()->registCFunction("JZNodeInitGlobal",true, jzbind::createFuncion(JZNodeInitGlobal));
    JZNodeFunctionManager::instance()->registCFunction("JZNodeInitLocal", true, jzbind::createFuncion(JZNodeInitLocal));
    
    JZNodeFunctionManager::instance()->registCFunction("JZDataToUi", true, jzbind::createFuncion(JZDataToUi));
    JZNodeFunctionManager::instance()->registCFunction("JZUiToData", true, jzbind::createFuncion(JZUiToData));
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

    m_objectInfo.clear();
    m_variantInfo.clear();

    m_sender = nullptr;
    m_statusCommand = Status_none;
    m_status = Status_none;
    m_breakNodeId = -1;
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
    
    auto function_list = m_program->functionDefines();
    for (int i = 0; i < function_list.size(); i++)
        JZNodeFunctionManager::instance()->registFunction(function_list[i]);

    auto define_list = m_program->objectDefines();    
    for(int i = 0; i < define_list.size(); i++)
        JZNodeObjectManager::instance()->regist(define_list[i]);    

    Q_ASSERT(!g_engine);
    g_engine = this;
    QVariantList in, out;
    auto init_func = function("__init__");    
    call(init_func, in,out);

    //init global connect cache    
    auto script_list = m_program->scriptList();
    for (int sc_idx = 0; sc_idx < script_list.size(); sc_idx++)
    {
        if (!script_list[sc_idx]->className.isEmpty())
            continue;

        auto list = script_list[sc_idx]->functionList;
        for (int i = 0; i < list.size(); i++)
        {
            auto &func_def = list[i];
            QString &func = func_def.name;
            if (func.startsWith("on_"))
            {
                int index = func.indexOf("_", 3);
                if (index == -1)
                    continue;

                QString param = func.mid(3, index - 3);
                QString single = func.mid(index + 1);
                Q_ASSERT(m_global.contains(param));
                auto ref = m_global[param].data();                

                auto sender_type = JZObjectType(*ref);
                auto sender_meta = JZNodeObjectManager::instance()->meta(sender_type);
                Q_ASSERT(sender_meta);

                auto s = sender_meta->single(single);

                ConnectCache info;
                info.eventType = s->eventType;
                info.sender = ref;
                info.recv = nullptr;
                info.handle = func_def.fullName();
                m_variantInfo[ref].connectQueue.push_back(info);
            }
        }
    }
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

void JZNodeEngine::pushStack(const FunctionDefine *func)
{
    if (m_stack.size() > 0)    
        m_stack.currentEnv()->pc = m_pc;     
        
    m_stack.push();    
    m_stack.currentEnv()->func = func;
    if(!func->isCFunction)
    {                
        m_pc = func->addr;
        m_script = getScript(func->file);
        Q_ASSERT(m_script);

        m_stack.currentEnv()->pc = m_pc;
        m_stack.currentEnv()->script = m_script;
        if (func->isMemberFunction())
            m_stack.currentEnv()->object = *m_regs[Reg_Call];        
    }
    else
    {
        m_pc = -1;
        m_script = nullptr;
    }
}

void JZNodeEngine::popStack()
{       
    for (auto &ptr : m_stack.currentEnv()->locals)
        varaiantDeleteNotify(ptr.data());
    for (auto &ptr : m_stack.currentEnv()->stacks)
        varaiantDeleteNotify(ptr.data());

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

void JZNodeEngine::connectEvent(JZNodeObject *sender, int event, JZNodeObject *recv, QString handle)
{    
    ConnectInfo connect;
    connect.sender = sender;
    connect.receiver = recv;
    connect.eventType = event;
    connect.handle = handle;
    m_objectInfo[sender].connects.push_back(connect);

    if (connectCount(sender, event) == 1)
    {
        auto s = sender->single(event);
        if (s->csingle)
            s->csingle->connect(sender);
    }
}

void JZNodeEngine::disconnectEvent(JZNodeObject *sender, int event, JZNodeObject *recv, QString handle)
{    
    auto it = m_objectInfo.find(sender);
    if (it == m_objectInfo.end())
        return;

    auto &list = it->connects;
    for (int i = 0; i < list.size(); i++)
    {
        auto &c = list[i];
        if (c.sender == sender && c.eventType == event
            && (recv == nullptr || recv == c.receiver)
            && (handle.isEmpty() || handle == c.handle))
        {
            list.removeAt(i);
        }
    }

    if (connectCount(sender, event) == 0)
    {
        auto s = sender->single(event);
        if (s->csingle)
            s->csingle->disconnect(sender);
    }    
}

int JZNodeEngine::connectCount(JZNodeObject *sender, int event)
{
    auto it = m_objectInfo.find(sender);
    if (it == m_objectInfo.end())
        return 0;

    int count = 0;
    auto &list = it->connects;
    for (int i = 0; i < list.size(); i++)
    {
        auto &c = list[i];
        if (c.sender == sender && c.eventType == event)
        {
            count++;
        }
    }
    return count;
}

void JZNodeEngine::connectSelf(JZNodeObject *object)
{    
    //bind
    auto bindInfo = m_program->bindInfo(object->className());
    auto it = bindInfo.begin();
    while (it != bindInfo.end())
    {
        QString var_name = it.key();
        QString widget_name = it.value();
        auto v = object->paramRef(var_name);
        auto w = object->paramRef(widget_name);
        Q_ASSERT(v && w && JZNodeType::isWidget(*w));
        
        m_variantInfo[v].bindWidget = w;
        if (!JZNodeType::isNullptr(*w))
        {
            auto jz_w = toJZObject(*w);
            JZNodeQtBind::bind(JZObjectCast<QWidget>(jz_w),v);
        }
        else
        {            
            m_variantInfo[w].bindValueCache = v;
        }

        it++;
    }
    
    //single
    auto script_list = m_program->scriptList();
    for (int sc_idx = 0; sc_idx < script_list.size(); sc_idx++)
    {
        if (script_list[sc_idx]->className != object->className())
            continue;

        auto list = script_list[sc_idx]->functionList;
        for (int i = 0; i < list.size(); i++)
        {
            auto &func_def = list[i];
            QString &func = func_def.name;
            if (func.startsWith("event_"))
            {
                QString event = func.mid(6);
                auto meta = object->meta();
                while (meta && !meta->isCObject)
                    meta = meta->super();

                if (meta && meta->cMeta.addEventFilter)
                {
                    auto define = meta->event(event);
                    meta->cMeta.addEventFilter(object, define->eventType);
                }
            }
            else if (func.startsWith("on_"))
            {
                int index = func.indexOf("_", 3);
                if (index == -1)
                    continue;

                QString param = func.mid(3, index - 3);
                QString single = func.mid(index + 1);
                auto ref = object->paramRef(param);
                if (!ref && isJZObject(*ref))
                {
                    qDebug() << "no such element: " + param;
                    continue;
                }

                auto sender_type = JZObjectType(*ref);
                auto sender_meta = JZNodeObjectManager::instance()->meta(sender_type);
                Q_ASSERT(sender_meta);

                auto s = sender_meta->single(single);
                if (!s)
                {
                    qDebug() << "no such single: " + single;
                    continue;
                }

                JZNodeObject *sender = toJZObject(*ref);                
                if (sender)
                    connectEvent(sender, s->eventType, object, func_def.fullName());
                else
                {
                    ConnectCache info;
                    info.eventType = s->eventType;
                    info.sender = ref;                    
                    info.recv = object;
                    info.handle = func_def.fullName();
                    m_variantInfo[ref].connectQueue.push_back(info);

                    m_objectInfo[object].connectQueue++;
                }
            }
        }
    }
}

void JZNodeEngine::connectSingleLater(QVariant *v)
{   
    auto sender = toJZObject(*v);
    if (!sender)
        return;

    auto it = m_variantInfo.find(v);
    if (it == m_variantInfo.end())
        return;

    if (it->bindValueCache)
    {
        auto w = JZObjectCast<QWidget>(sender);
        widgetBind(w, it->bindValueCache);        
        it->bindValueCache = nullptr;
    }
    
    auto &list = it->connectQueue;
    for (int i = list.size() - 1; i >= 0; i--)
    {
        auto &c = list[i];
        if (c.sender == v)
        {
            connectEvent(sender, c.eventType, c.recv, c.handle);
            if (c.recv)
            {
                m_objectInfo[c.recv].connectQueue--;
                Q_ASSERT(m_objectInfo[c.recv].connectQueue >= 0);
            }
            list.removeAt(i);           
        }
    }
}

void JZNodeEngine::widgetBind(QWidget *w, QVariant *ref)
{
    JZNodeQtBind::bind(w, ref);
}

void JZNodeEngine::widgetUnBind(QWidget *w)
{
    JZNodeQtBind::unbind(w);
}

void JZNodeEngine::widgetUnBindNotify(QWidget *w)
{
    auto it = m_variantInfo.begin();
    while (it != m_variantInfo.end())
    {        
        if (it->bindWidget && JZObjectCast<QWidget>(toJZObject(*it->bindWidget)) == w)
            it->bindWidget = nullptr;
        it++;
    }
}

void JZNodeEngine::dealEvent(JZEvent *event)
{
    Q_ASSERT(event->sender);

    JZNodeObject *obj = event->sender;
    auto func_name = "event_" + obj->meta()->event(event->eventType)->name;
    func_name = obj->className() + "." + func_name;

    QVariantList out;
    call(func_name, event->params, out);
}

void JZNodeEngine::dealSlot(JZEvent *event)
{
    JZNodeObject *obj = event->sender;
    auto it = m_objectInfo.find(obj);
    if (it == m_objectInfo.end())
        return;
    
    auto &list = it->connects;
    for (int i = 0; i < list.size(); i++)
    {
        auto &connect = list[i];
        if (connect.eventType == event->eventType)
        {
            auto back = m_sender;
            m_sender = event->sender;

            QVariantList out;
            QVariantList in = event->params;
            if (connect.receiver)
                in.insert(0, QVariant::fromValue(connect.receiver));

            call(connect.handle, in, out);

            m_sender = back;
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
        m_stack.currentEnv()->pc = m_pc;                                
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

            return *ref;
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
            QVariant *ref = m_stack.currentEnv()->getRef(param.id());
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
		m_global[name] = JZVariantPtr(new QVariant());
	
	*m_global[name] = v;
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
    auto env = (stack_level == -1) ? m_stack.currentEnv() : m_stack.env(stack_level);

    auto it = env->stacks.find(id);
    if (it == env->stacks.end())
        return nullptr;

    return it->data();
}

QVariant JZNodeEngine::getVariable(int id)
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
        QVariant *obj = nullptr;
        if (obj_list[0] == "this")
            obj = env? &env->object : nullptr;
        else
            obj = getVariableRefSingle(env,obj_list[0]);

        if (!obj)
            return nullptr;

        for (int i = 1; i < obj_list.size(); i++)
            obj = toJZObject(*obj)->paramRef(obj_list[i]);

        JZNodeObject *ptr = toJZObject(*obj);
        return ptr->paramRef(param_name);
    }
    else
    {
        return getVariableRefSingle(env, param_name);
    }
}

QVariant JZNodeEngine::getVariable(const QString &name)
{    
    QVariant *ref = getVariableRef(name);
    if(!ref)
        throw std::runtime_error("no such variable " + name.toStdString());

    return *ref;        
}

void JZNodeEngine::setVariable(const QString &name, const QVariant &value)
{    
    QVariant *ref = getVariableRef(name);
    if(!ref)
        throw std::runtime_error("no such variable " + name.toStdString());

    dealSet(ref, value);
}

void JZNodeEngine::dealSet(QVariant *ref, const QVariant &value)
{
    bool isChange = false;
    if (JZNodeType::isBaseOrEnum(JZNodeType::variantType(value)) && value != *ref)
        isChange = true;

    Q_ASSERT(m_anyVariants.contains(ref) || JZNodeType::variantType(*ref) == JZNodeType::variantType(value));
    *ref = value;

    if (isJZObject(value))
        connectSingleLater(ref);
    if (isChange)
        valueChanged(ref); 
}

QVariant JZNodeEngine::getThis()
{    
    if (m_stack.size() == 0)
        return QVariant::fromValue(JZObjectNull());

    return m_stack.currentEnv()->object;
}

QVariant JZNodeEngine::getSender()
{
    return QVariant::fromValue(m_sender);
}

void JZNodeEngine::objectCreate(JZNodeObject *object)
{
    m_objectInfo.insert(object, JZObjectInfo());
    connectSelf(object);
}

void JZNodeEngine::objectDelete(JZNodeObject *object)
{
    auto it = m_objectInfo.find(object);
    Q_ASSERT(it != m_objectInfo.end());
    if (it->connectQueue != 0)
    {
        auto it_v = m_variantInfo.begin();
        while (it_v != m_variantInfo.end())
        {
            for (int i = it_v->connectQueue.size() - 1; i >= 0; i--)
            {
                if (it_v->connectQueue[i].recv == object)
                    it_v->connectQueue.removeAt(i);
            }
            it_v++;
        }
    }
    m_objectInfo.erase(it);
    delete object;
}

void JZNodeEngine::varaiantDeleteNotify(QVariant *v)
{    
    m_anyVariants.remove(v);
    auto it = m_variantInfo.find(v);
    if(it != m_variantInfo.end())
    {
        if (it->bindWidget)
        {
            auto w = JZObjectCast<QWidget>(toJZObject(*it->bindWidget));
            JZNodeQtBind::unbind(w);
        }
        m_variantInfo.erase(it);
    }
}

void JZNodeEngine::widgetValueChanged(QWidget *w)
{
    QVariant id = w->property("BindValue");
    Q_ASSERT(id.isValid());
        
    QVariant *v = (QVariant*)id.value<void*>();
    QVariant old = *v;
    JZNodeQtBind::uiToData(w,*v);
    valueChanged(v);
}

void JZNodeEngine::valueChanged(QVariant *v)
{
    auto it = m_variantInfo.find(v);
    if (it == m_variantInfo.end())
        return;

    if (it->bindWidget)
    {
        auto w = JZObjectCast<QWidget>(toJZObject(*it->bindWidget));
        JZNodeQtBind::dataToUi(*v,w);
    }

    auto &list = it->paramChanges;
    for (int i = 0; i < list.size(); i++)
    {
        auto &info = list[i];
        QVariantList in,out;
        in << QVariant::fromValue(info.recv);
        call(info.handle, in, out);
    }
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

    return *ref;
}

QVariant *JZNodeEngine::getRegRef(int id)
{
    Q_ASSERT(id >= Reg_Start);
    
    auto it = m_regs.find(id);
    if (it == m_regs.end())
        return nullptr;

    return it->data();    
}

void JZNodeEngine::setReg(int id, const QVariant &value)
{    
    m_regs[id] = JZVariantPtr(new QVariant(value));
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
        if (i == 0 && func->isMemberFunction() && JZNodeType::isNullptr(v))
            throw std::runtime_error("object is nullptr");
    }
}

void JZNodeEngine::callCFunction(const FunctionDefine *func)
{    
    QVariantList paramIn, paramOut;
    // get input
    auto &inList = func->paramIn;
    for (int i = 0; i < inList.size(); i++)    
        paramIn.push_back(getReg(Reg_Call + i));  

    checkFunction(func);

    // call function
    pushStack(func);
    func->cfunc->call(paramIn,paramOut);
    popStack();

    // set output
    auto &outList = func->paramOut;
    for (int i = 0; i < outList.size(); i++)
        setReg(Reg_Call + i,paramOut[i]);
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
    else if(m_script->canBreak[m_pc])
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
            if (pt.type == BreakPoint::nodeEnter && pt.file == filepath)
            {
                if (m_breakStep.nodeId == pt.nodeId)
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
            auto value = JZNodeType::matchValue(ir_alloc->dataType, getParam(ir_alloc->value));
            if(ir_alloc->allocType == JZNodeIRAlloc::Heap)
                initGlobal(ir_alloc->name,value);
            else if (ir_alloc->allocType == JZNodeIRAlloc::Stack)
                initLocal(ir_alloc->name, value);
            else
                initLocal(ir_alloc->id, value);

            if (ir_alloc->dataType = Type_any)
            {
                if (ir_alloc->allocType == JZNodeIRAlloc::Heap
                    || ir_alloc->allocType == JZNodeIRAlloc::Stack)
                    m_anyVariants << getVariableRef(ir_alloc->name);
                else
                    m_anyVariants << m_stack.currentEnv()->getRef(ir_alloc->id);
            }
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
