#ifndef JZNODE_RUNNER_H_
#define JZNODE_RUNNER_H_

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QThread>
#include <QSemaphore>
#include "JZProject.h"
#include "JZNodeProgram.h"
#include "JZNodeFunctionManager.h"

class RunnerEnv
{
public:
    RunnerEnv();
    
    const FunctionDefine *func;
    int pc;
};

class Stack
{
public:
    Stack();
    ~Stack();

    void clear();
    int size();

    RunnerEnv &env();
    void pop();
    void push();

    QVariant getVariable(int id);
    void setVariable(int id, const QVariant &value);

    QVariant getVariable(int level,int id);
    void setVariable(int level,int id, const QVariant &value);

public:
    QList<QMap<int,QVariant>> m_stack;    
    QList<RunnerEnv> m_env;
};

class JZNodeRuntimeError{
public:
    QString message;
};

class BreakPoint
{
public: 
    BreakPoint();

    int pc;
    bool once;
};

class JZNodeVM;
//JZNodeEngine
class JZNodeEngine
{
public:
    JZNodeEngine();
    ~JZNodeEngine();  

    void setProgram(JZNodeProgram *program);
    JZNodeProgram *program();
    void init();    

    void addBreakPoint(BreakPoint pt);
    void removeBreakPoint(BreakPoint pt);    
    void pause();
    void resume();
    void stepIn(int nodeId);
    void stepOver(int nodeId);
    void stepOut(int nodeId);

    void setVm(JZNodeVM *vm);

    QVariant getVariable(int id);
    void setVariable(int id, const QVariant &value);
     
    bool call(QString name,QVariantList &in,QVariantList &out);    

protected:            
    void notify(int id, const QVariant &data);       
    void run();     

    const FunctionDefine *function(QString name);   
    void callFunction(const FunctionDefine *func);
    void callCFunction(const FunctionDefine *func);        
    bool setCommand(int cmd);    
    QVariant dealExpr(int op,QVariant &a,QVariant &b);    
    void pushStack(const FunctionDefine *define);
    void popStack();

    int m_pc;    
    JZNodeVM *m_vm;
    JZNodeProgram *m_program;
        
    Stack m_stack;
    QMap<int,QVariant> m_global;            
    QMap<int,QVariant> m_regCall;
    QVariantList m_outList;
    QList<BreakPoint> m_breakPoints;
    bool m_needPause;
    bool m_pause;     
    QMutex m_mutex;    
    QSemaphore m_waitCond;
};


#endif
