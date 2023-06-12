#ifndef JZNODE_ENGINE_H_
#define JZNODE_ENGINE_H_

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QThread>
#include <QSemaphore>
#include <QException>
#include "JZProject.h"
#include "JZNodeProgram.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeObject.h"

enum{
    Status_none,
    Status_running,
    Status_pause,
    Status_stop,
};

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

class JZNodeRuntimeError
{
public:    
    QString info;
    QString script;
    int pc;
};
QDataStream &operator<<(QDataStream &s, const JZNodeRuntimeError &param);
QDataStream &operator>>(QDataStream &s, JZNodeRuntimeError &param);


class JZNodeRuntimeInfo
{
public:
    JZNodeRuntimeInfo();
    
    enum{
        None,
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
class JZNodeEngine : public QObject
{
    Q_OBJECT

public:
    JZNodeEngine();
    virtual ~JZNodeEngine();

    void setProgram(JZNodeProgram *program);
    JZNodeProgram *program();    
    void init();    

    JZNodeRuntimeInfo runtimeInfo();    

    void addWatch();
    void clearWatch();

    int addBreakPoint(QString filepath,int nodeId);
    void removeBreakPoint(int id);
    void clearBreakPoint();    

    void pause();
    void resume();    
    void stop();
    void stepIn();
    void stepOver();
    void stepOut();        

    QVariant getThis();
    void setThis(QVariant var);

    QVariant getVariable(QString name);
    void setVariable(QString name, const QVariant &value);    

    QVariant getReg(int id);
    void setReg(int id, const QVariant &value);
     
    void dealEvent(JZEvent *event);
    bool call(const QString &function,const QVariantList &in,QVariantList &out);    
    bool call(const FunctionDefine *func,const QVariantList &in,QVariantList &out);

    void objectChanged(const QString &name);

signals:
    void sigParamChanged();
    void sigRuntimeError(JZNodeRuntimeError error);

protected:
    class JZObjectConnect
    {
    public:        
        JZNodeObject *sender;
        JZNodeObject *receiver;
        int eventType;
        FunctionDefine *handle;
    };

    class ParamChangeHandle
    {
    public:
        QString name;
        JZNodeObject *env;
        FunctionDefine *define;
    };

    bool run();         

    const FunctionDefine *function(QString name);       
    void callCFunction(const FunctionDefine *func);    
    bool setCommand(int cmd);    
    QVariant dealExpr(QVariant &a,QVariant &b,int op);    
    void pushStack(const FunctionDefine *define);
    void popStack();
    int indexOfBreakPoint(int id);   
    void waitStatus(int status);
    
    QString variableToString(const QVariant &v);
    QVariant getParam(const JZNodeIRParam &param);
    void setParam(const JZNodeIRParam &param,const QVariant &value);        
        
    int nodeIdByPc(int pc);        
    JZNodeScript *getScript(QString path);
    JZNodeScript *getObjectScript(QString objName);
    void connectScript(QString objName,JZNodeObject *obj,JZNodeScript *script);
    void connectSelf(JZNodeObject *obj);
    void connectQObject(QObject *obj);
    void disconnectObject(JZNodeObject *obj);
    JZNodeObject *getObject(QString name);
    bool isWatch();        

    int m_pc;            
    JZNodeProgram *m_program;    
    JZNodeScript *m_script;
    QWidget *m_window;    
        
    QList<BreakPoint> m_breakPoints;
    BreakPoint m_breakStep;    
    int m_breaknodeId;      // 断点中断位置的nodeid

    QList<int> m_watchParam;

    Stack m_stack;
    QMap<QString,QVariant> m_global;            
    QMap<int,QVariant> m_regs;        
    QVariant m_this;
           
    int m_statusCommand;
    int m_status;     
    QMutex m_mutex;    
    QSemaphore m_waitCond;

    QMap<JZNodeObject*,QList<JZObjectConnect>> m_connects;
    QList<ParamChangeHandle> m_paramChangeHandle;
};
extern JZNodeEngine *g_engine;

#endif
