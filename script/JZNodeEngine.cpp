#include "JZNodeEngine.h"
#include "JZNodeFunctionManager.h"
#include "JZEvent.h"
#include <QPushButton>
#include <QApplication>

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

// JZNodeEngine
JZNodeEngine::JZNodeEngine()
{
    m_command = cmd_none;
    m_program = nullptr;    
    m_window = nullptr;        
    m_state = Runner_stoped;
    m_pc = -1;
}

JZNodeEngine::~JZNodeEngine()
{
    stop();
}

void JZNodeEngine::setProgram(JZNodeProgram *program)
{
    m_program = program;
}

JZNodeProgram *JZNodeEngine::program()
{
    return m_program;
}

void JZNodeEngine::pushStack(const FunctionDefine *func)
{
    if(m_stack.size() > 0)        
        m_stack.env().pc = m_pc;
        
    m_stack.push();
    m_stack.env().pc = func->addr;
    m_stack.env().func = func;
    m_pc = func->addr;
}

void JZNodeEngine::popStack()
{   
    m_stack.pop();
    if(m_stack.size() > 0)
        m_pc = m_stack.env().pc;
    else
        m_pc = -1;
}

bool JZNodeEngine::call(QString name,QVariantList &in,QVariantList &out)
{
    auto inst = JZNodeFunctionManager::instance();    
    auto *func = inst->function(name);
    if(func->isCFunction)
    {
        JZNodeFunctionManager::instance()->callCFunction(func, in, out);
    }
    else
    {
        for (int i = 0; i < in.size(); i++)    
            setVariable(Reg_Call + i,in[i]);
        pushStack(func);        
        run();
        out = m_outList;
    }       

    return true;
}

QVariant JZNodeEngine::getVariable(int id)
{       
    if(id >= Reg_Call)    
        return m_regCall.value(id,QVariant());
    else
        return m_stack.getVariable(id);    
}

void JZNodeEngine::setVariable(int id, const QVariant &value)
{
    if(id >= Reg_Call)
        m_regCall.insert(id,value);
    else    
        m_stack.setVariable(id, value);            
}

void JZNodeEngine::notify(int id, const QVariant &data);
{

}

void JZNodeEngine::initProgram()
{
}   

void JZNodeEngine::callCFunction(const FunctionDefine *func)
{    
    QVariantList paramIn, paramOut;
    // get input
    auto inList = func->paramIn;
    for (int i = 0; i < inList.size(); i++)    
        paramIn.push_back(getVariable(Reg_Call + i));

    // call function
    pushStack(func);
    JZNodeFunctionManager::instance()->callCFunction(func,paramIn, paramOut);
    popStack();

    // set output
    auto outList = func->paramOut;
    for (int i = 0; i < inList.size(); i++)    
        setVariable(Reg_Call + i,paramOut[i]);
}

const FunctionDefine *JZNodeEngine::function(QString name)
{
    auto &list = m_program->eventHandleList();
    for(int i = 0; i < list.size(); i++)
    {
        if(list[i].function.name == name)
            return &list[i].function;
    }

    return JZNodeFunctionManager::instance()->function(name);
}

void JZNodeEngine::callFunction(const FunctionDefine *func)
{
    pushStack(func);    
}

QVariant JZNodeEngine::dealExpr(int op,QVariant &a,QVariant &b)
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
    default:
        break;
    }
    return QVariant();
}

void JZNodeEngine::run()
{
    auto &op_list = m_program->opList;
    while (m_pc < op_list.size())
    {        
        m_vm->onStep(m_pc);

        try
        {
            //deal op
            JZNodeIR &op = op_list[m_pc];
            switch (op.type)
            {   
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
            case OP_xor:
            {
                QVariant a,b,c;
                a = getVariable(op.params[1].toInt());
                b = getVariable(op.params[2].toInt());
                c = dealExpr(op.type,a,b);
                setVariable(op.params[0].toInt(),c);
                break;
            }
            case OP_cmp:
            {
                QVariant cond = getVariable(op.params[0].toInt());
                if(cond.toBool())
                    m_pc = op.params[1].toInt();
                else
                    m_pc = op.params[2].toInt();
                break;
            }
            case OP_jmp:
            {
                m_pc = op.params[0].toInt();
                break;
            }
            case OP_set:
            {
                int dst = op.params[0].toInt();
                int src = op.params[1].toInt();
                QVariant tmp = getVariable(src);
                setVariable(dst,tmp);
                notify(dst,data);
                break;
            }
            case OP_setValue:
            {
                int dst = op.params[0].toInt();            
                setVariable(dst,op.params[1]);
                notify(dst,data);
                break;
            }
            case OP_get:
            {            
                break;
            }
            case OP_call:
            {            
                QString func_name = op.params[0].toString();
                auto func = JZNodeFunctionManager::instance()->function(func_name);
                if(func->isCFunction)            
                    callCFunction(func);            
                else            
                    callFunction(func);                             
                break;
            }
            case OP_return:
            {               
                popStack();
                break;
            }
            case OP_exit:
                break;
            default:
                Q_ASSERT(0);
                break;
            }
            m_pc++;
        }
        catch(const std::exception& e)
        {
            JZNodeRuntimeError error;
            onRuntimeError(error); 
        }            
    }            
}

//JZNodeVM
JZNodeVM::JZNodeVM()
{
    moveToThread(&m_thread);
    m_thread.start();
}

JZNodeVM::~JZNodeVM()
{
    m_thread.quit();
    m_thread.wait();
}

bool JZNodeVM::start()
{        
    QMutexLocker lock(&m_mutex);
    if(m_state != Runner_stoped)
        return false;

    m_state = Runner_running;
    sigStateChanged(Runner_running);    
    return true;
}

bool JZNodeVM::stop()
{    
    QMutexLocker lock(&m_mutex);
    if(m_state != Runner_stoped)
        return false;

    m_state = Runner_stoped;
    sigStateChanged(Runner_stoped);
    return true;
}

bool JZNodeVM::pause()
{
    QMutexLocker lock(&m_mutex);
    if(m_state != Runner_stoped)
        return false;

    m_state = Runner_paused;
    sigStateChanged(Runner_paused);
    return true;
}

bool JZNodeVM::resume()
{
    QMutexLocker lock(&m_mutex);
    if(m_state != Runner_paused)
        return false;

    m_state = Runner_running;
    sigStateChanged(Runner_running);
    return true;
}

bool JZNodeVM::isRunning()
{
    QMutexLocker lock(&m_mutex);
    return (m_state == Runner_running);
}

void JZNodeVM::customEvent(QEvent *event)
{
    if(m_state != Runner_running)
        return;

    JZEvent *e = (JZEvent *)event;
    auto &list = m_program->eventHandleList();
    for(int i = 0; i < list.size(); i++)
    {
        const JZEventHandle &handle = list[i];
        if(handle.match(e))
        {
            QVariantList in,out;
            auto func = handle.function;
            auto inList = func.paramIn;
            for (int i = 0; i < inList.size(); i++) 
            {
                //int id = m_program->paramId(func->n)  
                //setVariable(Reg_User + 1,paramIn[i]);    
            }                             
            call(func.name,in,out);
        }            
    }
}

void JZNodeVM::setState(int state)
{    
    if(m_state == state)
        return;

    m_state = state;
    sigStateChanged(m_state);
}

void JZNodeVM::addBreakPoint(BreakPoint pt)
{
    QMutexLocker lock(&m_mutex);
    m_breakPoints.push_back(pt);
}

void JZNodeVM::removeBreakPoint(BreakPoint pt)
{
    QMutexLocker lock(&m_mutex);
}

QVariant JZNodeVM::getVariable(int id)
{
    return m_engine.getVariable(id);
}

void JZNodeVM::setVariable(int id, const QVariant &value)
{    
    return m_engine.setVariable(id,value);
}

void JZNodeVM::createWindow()
{
    QList<QPushButton*> btn_list;
    for(int i = 0; i < btn_list.size(); i++)
    {
        connect(btn_list[i],&QPushButton::clicked,this,JZNodeVM::onButtonClicked);
    }
}

void JZNodeVM::onStep(int pc)
{
    if(m_state == Runner_paused)
        return;
        
    setState(Runner_paused);
}

void JZNodeVM::onValueNotify(int id,QVariant &value)
{

}

void JZNodeVM::onIntValueChanged(int value)
{
    JZEvent *event = new JZEvent();
    qApp->postEvent(event);
}

void JZNodeVM::onStringValueChanged(const QString &value)
{
    JZEvent *event = new JZEvent();
    qApp->postEvent(event);
}

void JZNodeVM::onDoubleValueChanged(double value)
{ 
    JZEvent *event = new JZEvent();
    qApp->postEvent(event);
}

void JZNodeVM::onButtonClicked()
{    
    JZEvent *event = new JZEvent();
    qApp->postEvent(event);    
}