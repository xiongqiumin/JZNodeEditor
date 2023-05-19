#ifndef JZNODE_ENGINE_H_
#define JZNODE_ENGINE_H_

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

class JZNodeRuntimeInfo
{
public:
    JZNodeRuntimeInfo();
    
    enum{
        Running,
        Paused,
    };

    int status;
    QString file;
    int nodeId;
    int pc;
};
QDataStream &operator<<(QDataStream &s, const JZNodeRuntimeInfo &param);
QDataStream &operator>>(QDataStream &s, JZNodeRuntimeInfo &param);

class BreakPoint
{
public: 
    enum{
        none,
        nodeEnter,
        nodeExit,     //step over
        stackEqual,   //step in/out     
    };

    BreakPoint();
    void clear();

    int id;
    int type;
    QString file;
    int nodeId;
    int stack; 
    int pcStart,pcEnd;   
};

//JZNodeEngine
class JZNodeEngine
{
public:
    JZNodeEngine();
    ~JZNodeEngine();  

    void setProgram(JZNodeProgram *program);
    JZNodeProgram *program();    
    void init();    

    JZNodeRuntimeInfo runtimeInfo();

    void addBreakPoint(QString filepath,int nodeId);
    void removeBreakPoint(int id);
    void clearBreakPoint();    
    void pause();
    void resume();    
    void stop();
    void stepIn();
    void stepOver();
    void stepOut();    

    QVariant getVariable(QString name);
    void setVariable(QString name, const QVariant &value);

    QVariant getReg(int id);
    void setReg(int id, const QVariant &value);
     
    bool call(QString function,QVariantList &in,QVariantList &out);    
    bool call(const FunctionDefine *func,QVariantList &in,QVariantList &out);

protected:
    enum{
        Status_none,
        Status_running,
        Status_pause,
        Status_stop,
    };
    void notify(int id, const QVariant &data);       
    bool run();     

    const FunctionDefine *function(QString name);       
    void callCFunction(const FunctionDefine *func);        
    bool setCommand(int cmd);    
    QVariant dealExpr(QVariant &a,QVariant &b,int op);    
    void pushStack(const FunctionDefine *define);
    void popStack();
    int indexOfBreakPoint(int id);   
    void waitStatus(int status);
    
    QVariant getParam(const JZNodeIRParam &param);
    void setParam(const JZNodeIRParam &param,const QVariant &value);

    int m_pc;            
    JZNodeProgram *m_program;    
    JZNodeScript *m_script;
    QWidget *m_window;    
        
    QList<BreakPoint> m_breakPoints;
    BreakPoint m_breakStep;
    int m_breadPointId;

    Stack m_stack;
    QMap<QString,QVariant> m_global;            
    QMap<int,QVariant> m_regs;
    QVariantList m_outList;    
    int m_nodeId;    
    int m_statusCommand;
    int m_status;     
    QMutex m_mutex;    
    QSemaphore m_waitCond;
};


#endif
