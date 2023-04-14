#ifndef JZNODE_RUNNER_H_
#define JZNODE_RUNNER_H_

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QThread>
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

enum
{
    Runner_stoped,
    Runner_running,
    Runner_paused,
};

class JZNodeRuntimeError{
public:
    QString message;
};

class JZNodeEngine : public QObject
{
    Q_OBJECT

public:
    JZNodeEngine();
    ~JZNodeEngine();

    void setProgram(JZNodeProgram *program);
    JZNodeProgram *program();
    void initProgram();

    QVariant getVariable(int id);
    void setVariable(int id, const QVariant &value);    

    bool start();
    bool stop();
    bool pause();
    bool resume();    
    bool isRunning();
    bool call(QString name,QVariantList &in,QVariantList &out);    

signals:
    void sigStateChanged(int status);
    void sigValueChanged(int id, QVariant param);
    void sigRuntimeError(JZNodeRuntimeError error);

protected slots:    

protected:
    enum{
        cmd_none,
        cmd_stop,
        cmd_pause,
        cmd_resume,
    };    
    virtual void customEvent(QEvent *event);    

    void notify(int name, QVariant data);    
    
    void run();     
    const FunctionDefine *function(QString name);   
    void callFunction(const FunctionDefine *func);
    void callCFunction(const FunctionDefine *func);        
    bool setCommand(int cmd);
    void setState(int state);
    QVariant dealExpr(int op,QVariant &a,QVariant &b);    
    void pushStack(const FunctionDefine *define);
    void popStack();

    int m_pc;
    int m_state;
    int m_command;
    QWidget *m_window;    
        
    Stack m_stack;
    QMap<int,QVariant> m_global;            
    QMap<int,QVariant> m_regCall;
    QVariantList m_outList;
    
    JZNodeProgram *m_program;      
    JZNodeRuntimeError m_error;
    bool m_autoRun;    
};

#endif
