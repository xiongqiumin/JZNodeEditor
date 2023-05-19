#include "JZNodeEngine.h"
#include "JZNodeFunctionManager.h"
#include "JZEvent.h"
#include <QPushButton>
#include <QApplication>
#include "JZNodeVM.h"

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

//JZNodeRuntimeInfo
JZNodeRuntimeInfo::JZNodeRuntimeInfo()
{

}

QDataStream &operator<<(QDataStream &s, const JZNodeRuntimeInfo &param)
{        
    s << param.file << param.nodeId;
    return s;    
}

QDataStream &operator>>(QDataStream &s, JZNodeRuntimeInfo &param)
{    
    s >> param.file >> param.nodeId;
    return s;
}

//BreakPoint
BreakPoint::BreakPoint()
{    
    clear();        
}

void BreakPoint::clear()
{
    id = -1;
    type = BreakPoint::none;
    file.clear();
    nodeId = -1;
    stack = -1;
    pcStart = -1;
    pcEnd = -1;
}

// JZNodeEngine
JZNodeEngine::JZNodeEngine()
{    
    m_program = nullptr;
    m_script = nullptr;
    m_pc = -1;
    m_nodeId = -1;
    m_status = Status_none;
    m_statusCommand = Status_none;
    m_breadPointId = 0;
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
    m_global = m_program->variables();
}   

JZNodeRuntimeInfo JZNodeEngine::runtimeInfo()
{   
    JZNodeRuntimeInfo info;
    QMutexLocker lock(&m_mutex);
    info.status = m_status;
    if(m_script)
    {
        info.file = m_script->file;
        info.nodeId = m_nodeId;
        info.pc = m_pc;
    }
    return info;
}

void JZNodeEngine::pushStack(const FunctionDefine *func)
{
    if(m_stack.size() > 0)        
        m_stack.env().pc = m_pc;
        
    m_stack.push();
    m_stack.env().pc = func->addr;
    m_stack.env().func = func;
    m_pc = func->addr;
    m_script = func->script;
}

void JZNodeEngine::popStack()
{   
    m_stack.pop();
    if(m_stack.size() > 0)
    {
        m_pc = m_stack.env().pc;
        m_script = m_stack.env().func->script;
    }
    else
    {
        m_pc = -1;
        m_script = nullptr;
    }
}

bool JZNodeEngine::call(QString name,QVariantList &in,QVariantList &out)
{    
    auto *func = function(name);
    return call(func,in,out);
}

bool JZNodeEngine::call(const FunctionDefine *func,QVariantList &in,QVariantList &out)
{
    if(Status_none)
    
    m_status = Status_running;
    if(func->isCFunction)
    {
        func->cfunc->call(in,out);
    }
    else
    {
        for (int i = 0; i < in.size(); i++)    
            setReg(Reg_Call + i,in[i]);
        pushStack(func);        
        if(!run())
            return false;            
        out = m_outList;
    }           
    return true;
}

QVariant JZNodeEngine::getParam(const JZNodeIRParam &param)
{       
    if(param.isLiteral())    
        return param.value;
    else if(param.isRef())
        return getVariable(param.ref());
    else
        return getReg(param.id());
}

void JZNodeEngine::setParam(const JZNodeIRParam &param,const QVariant &value)
{
    if(param.isId())    
        return setReg(param.id(),value);
    else if(param.isRef())
        return setVariable(param.ref(),value);
    else
    {
        Q_ASSERT(0);
    }        
}

QVariant JZNodeEngine::getVariable(QString name)
{
    return m_global[name];
}

void JZNodeEngine::setVariable(QString name, const QVariant &value)
{
    m_global[name] = value;
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

void JZNodeEngine::notify(int id, const QVariant &data)
{

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
    pt.id = m_breadPointId++;
    pt.type = BreakPoint::nodeEnter;
    pt.file = filepath;
    pt.nodeId = nodeId;
    m_breakPoints.push_back(pt);
}

void JZNodeEngine::removeBreakPoint(int id)
{
    QMutexLocker lock(&m_mutex);
    int idx = indexOfBreakPoint(id);
    if(idx == -1)
        return;
    
    m_breakPoints.removeAt(idx);
}

int JZNodeEngine::indexOfBreakPoint(int id)
{
    for(int i = 0; i < m_breakPoints.size(); i++)
    {
        if(m_breakPoints[i].id == id)
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
            if(status == Status_pause)
            {
                if(m_status == Status_pause || m_status == Status_stop)
                    return;
            }
            else
            {
                if(m_status == Status_stop)
                    return;
            }
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
    waitStatus(Status_pause);
}

void JZNodeEngine::resume()
{
    QMutexLocker lock(&m_mutex);
    if(m_status != Status_pause)
        return;
    
    m_waitCond.release();   
    //这里不能wait,存在启动后马上返回或者程序结束的情况 
}

void JZNodeEngine::stop()
{
    QMutexLocker lock(&m_mutex);
    if(m_status == Status_running || m_status == Status_pause)
        return;
        
    m_statusCommand = Status_stop;
    if(m_status == Status_pause)
        m_waitCond.release();
    waitStatus(Status_stop);
}

void JZNodeEngine::stepIn()
{
    QMutexLocker lock(&m_mutex);
    if(m_status != Status_pause)
        return;

    auto info = m_script->nodeInfo[m_nodeId];
    if(info.node->type() == Node_function)
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
}

void JZNodeEngine::stepOver()
{
    QMutexLocker lock(&m_mutex);    
    if(m_status != Status_pause)
        return;

    auto &info = m_script->nodeInfo[m_nodeId];
    m_breakStep.type = BreakPoint::nodeExit;   
    m_breakStep.file = m_script->file;     
    m_breakStep.pcStart = info.start;
    m_breakStep.pcEnd = info.end;
    m_waitCond.release();
}

void JZNodeEngine::stepOut()
{
    QMutexLocker lock(&m_mutex);
    if(m_status != Status_pause)
        return;

    m_breakStep.type = BreakPoint::stackEqual;
    m_breakStep.stack = m_stack.size() - 1;
    m_waitCond.release();
}

void JZNodeEngine::callCFunction(const FunctionDefine *func)
{    
    QVariantList paramIn, paramOut;
    // get input
    auto inList = func->paramIn;
    for (int i = 0; i < inList.size(); i++)    
        paramIn.push_back(getReg(Reg_Call + i));

    // call function
    pushStack(func);
    func->cfunc->call(paramIn,paramOut);
    popStack();

    // set output
    auto outList = func->paramOut;
    for (int i = 0; i < inList.size(); i++)    
        setReg(Reg_Call + i,paramOut[i]);
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
        return a.toInt() / b.toInt();
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
        Q_ASSERT(0);
        break;
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
            else if(op_list[m_pc]->type == OP_nodeId)
            {
                JZNodeIRNodeId *ir_node = (JZNodeIRNodeId *)op_list[m_pc].data();
                int node_id = ir_node->id;
                int stack = m_stack.size();
                auto breakTriggred = [](BreakPoint &pt,const QString &filepath,int pc,int node_id,int stack)->bool
                {                    
                    if(pt.type == BreakPoint::nodeEnter && pt.file == filepath && node_id == pt.nodeId)
                        return true;
                    else if(pt.type == BreakPoint::nodeExit && (pt.file != filepath || pc < pt.pcStart || pc >= pt.pcEnd))
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
            m_waitCond.acquire();
            if(m_statusCommand == Status_stop)
            {
                m_statusCommand = Status_none;
                m_status = Status_none;
                return false;
            }
            m_status = Status_running;
        }

        try
        {
            //deal op
            JZNodeIR *op = op_list[m_pc].data();
            int op_type = op->type;
            switch (op_type)
            {   
            case OP_nodeId:
            {                
                JZNodeIRNodeId *ir_node = (JZNodeIRNodeId*)op;
                m_nodeId = ir_node->id;
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
                JZNodeIRExpr *ir_expr = (JZNodeIRExpr*)op;
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
                auto func = JZNodeFunctionManager::instance()->function(ir_call->function);
                if(func->isCFunction)            
                    callCFunction(func);            
                else            
                    pushStack(func);
                break;
            }
            case OP_return:
            {               
                popStack();
                if(m_stack.size() != 0)
                    continue;
                else
                    goto RunEnd;
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

        }            
    }

RunEnd:
    m_breakStep.clear();
    m_status = Status_none;
    return true;
}
