#ifndef JZNODE_RUNNER_H_
#define JZNODE_RUNNER_H_

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QThread>
#include "JZProject.h"
#include "JZNodeProgram.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeDebugServer.h"

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

//JZNodeEngine
class JZNodeVM;
class JZNodeEngine
{
public:
    JZNodeEngine();
    ~JZNodeEngine();  

    void setProgram(JZNodeProgram *program);
    JZNodeProgram *program();
    void initProgram();    

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
    int m_state;
    int m_command;
    JZNodeVM *m_vm;
        
    Stack m_stack;
    QMap<int,QVariant> m_global;            
    QMap<int,QVariant> m_regCall;
    QVariantList m_outList;              
};

//JZNodeVM
class BreakPoint
{
public:
    enum{
        Node,
        DataWatch,
    };

    int id;
};

class JZNodeDebugServer
{
public:

};

class JZNodeDebugClient
{
public:

};

class JZNodeVM : public QObject
{
    Q_OBJECT

public:
    JZNodeVM();
    ~JZNodeVM();

    QWidget *mainWindow();
    
    bool load(QString path);    

    bool start();
    bool stop();
    bool pause();
    bool resume();       

    void addBreakPoint(BreakPoint pt);
    void removeBreakPoint(BreakPoint pt);

    QVariant getVariable(int id);
    void setVariable(int id, const QVariant &value);    

    void onStep(int pc);
    void onValueNotify(int id,QVariant &value);

signals:
    void sigStateChanged(int status);
    void sigRuntimeError(JZNodeRuntimeError );

protected:    
    void onIntValueChanged(int value);
    void onStringValueChanged(const QString &value);
    void onDoubleValueChanged(double value);
    void onButtonClicked();    
    
protected:    
    enum{
        cmd_none,
        cmd_stop,
        cmd_pause,
        cmd_resume,
    };

    enum
    {
        Runner_stoped,
        Runner_running,
        Runner_paused,
    };

    virtual void customEvent(QEvent *event);    
    void setState(int state);
    void createWindow();
    bool busy();    
    JZNodeEngine m_engine;
    JZNodeProgram m_program;

    QList<BreakPoint> m_breakPoints;
    QMutex m_mutex;
    QThread m_thread;    
    JZNodeDebugServer m_debugServer;
};



#endif
