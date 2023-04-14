#include "JZNodeEngine.h"
#include "JZNodeFunctionManager.h"
#include "JZEvent.h"

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
    m_autoRun = 0;
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

bool JZNodeEngine::start()
{        
    if(m_state != Runner_stoped)
        return false;

    m_state = Runner_running;
    sigStateChanged(Runner_running);    
    return true;
}

bool JZNodeEngine::stop()
{    
    if(m_state != Runner_stoped)
        return false;

    m_state = Runner_stoped;
    sigStateChanged(Runner_stoped);
    return true;
}

bool JZNodeEngine::pause()
{
    if(m_state != Runner_stoped)
        return false;

    m_state = Runner_paused;
    sigStateChanged(Runner_paused);
    return true;
}

bool JZNodeEngine::resume()
{
    if(m_state != Runner_paused)
        return false;

    m_state = Runner_running;
    sigStateChanged(Runner_running);
    return true;
}

bool JZNodeEngine::isRunning()
{
    return (m_state == Runner_running);
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
    {
        m_stack.setVariable(id, value);
        if(id < STACK_ID_START)
            emit sigValueChanged(id,value);
    }
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

void JZNodeEngine::setState(int state)
{    
    if(m_state == state)
        return;
    m_state = state;
    sigStateChanged(m_state);
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

void JZNodeEngine::customEvent(QEvent *event)
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

void JZNodeEngine::run()
{
    auto &op_list = m_program->opList;
    while (m_pc < op_list.size())
    {
        int state = m_state;
        if(state == Runner_stoped)
        {
            setState(Runner_stoped);
            break;
        }
        else if(state == Runner_paused)
        {
            setState(Runner_paused);
            break;
        }
        if(!m_error.message.isEmpty()){
            sigRuntimeError(m_error);
            break;
        }

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
            break;
        }
        case OP_setValue:
        {
            int dst = op.params[0].toInt();            
            setVariable(dst,op.params[1]);
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
}
