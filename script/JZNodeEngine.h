#ifndef JZNODE_ENGINE_H_
#define JZNODE_ENGINE_H_

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QThread>
#include <QException>
#include <QWaitCondition>
#include "JZProject.h"
#include "JZNodeProgram.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeObject.h"

enum{
    Status_none,
    Status_running,
    Status_pause,
    Status_idlePause,
    Status_error,
};
bool statusIsPause(int status);

class RunnerEnv
{
public:
    RunnerEnv();
    
    const FunctionDefine *func;
    JZNodeObject *object; //this    
    JZNodeScript *script;    
    int pc;
};

class Stack
{
public:
    struct StackVariant {
        QVariantMap locals;
        QMap<int, QVariant> stacks;
    };

    Stack();
    ~Stack();

    void clear();
    int size() const;
    bool isEmpty() const;

    void pop();
    void push();

    RunnerEnv &env();
    RunnerEnv &env(int index);    

    StackVariant *stackVariable(int index);

    QVariant *getVariableRef(QString name);
    QVariant getVariable(const QString &name);
    void setVariable(const QString &name, const QVariant &value);

    QVariant getVariable(int id);
    void setVariable(int id, const QVariant &value);

    QVariant getVariable(int level,int id);
    void setVariable(int level,int id, const QVariant &value);

protected:   
    QList<StackVariant> m_stack;    
    QList<RunnerEnv> m_env;
};

class JZNodeRuntimeInfo
{
public:
    struct Stack 
    {
        Stack();

        QString function;
        QString file;
        int nodeId;
        int pc;
    };
    JZNodeRuntimeInfo();   

    int status;
    QVector<Stack> stacks;    
};
QDataStream &operator<<(QDataStream &s, const JZNodeRuntimeInfo &param);
QDataStream &operator>>(QDataStream &s, JZNodeRuntimeInfo &param);

class JZNodeRuntimeError
{
public:
    QString error;
    JZNodeRuntimeInfo info;
};
QDataStream &operator<<(QDataStream &s, const JZNodeRuntimeError &param);
QDataStream &operator>>(QDataStream &s, JZNodeRuntimeError &param);

class BreakPoint
{
public: 
    enum{
        none,
        nodeEnter,
        stepOver,     //step over
        stackEqual,   //step in/out     
    };

    BreakPoint();
    void clear();

    int type;
    QString file;
    int nodeId;    
    int stack;
    NodeRange range;
};

//JZNodeEngine
class JZNodeDebugServer;
class JZNodeEngine : public QObject
{
    Q_OBJECT

public:
    JZNodeEngine();
    virtual ~JZNodeEngine();

    void init();

    void setProgram(JZNodeProgram *program);
    JZNodeProgram *program();        

    JZNodeRuntimeInfo runtimeInfo();    

    void setDebug(bool flag);
    void addWatch();
    void clearWatch();

    void addBreakPoint(QString filepath,int nodeId);
    void removeBreakPoint(QString filepath,int nodeId);
    void clearBreakPoint();    

    void pause();
    void resume();    
    void stop();
    void stepIn();
    void stepOver();
    void stepOut();        

    QVariant getThis();
    void setThis(QVariant var);

    Stack *stack();

    QVariant *getVariableRef(QString name);
    JZNodeObject *getObject(QString name);
    QVariant getVariable(QString name);
    void setVariable(QString name, const QVariant &value);    

    QVariant getReg(int id);
    void setReg(int id, const QVariant &value);
     
    void dealEvent(JZEvent *event);
    bool call(const QString &function,const QVariantList &in,QVariantList &out);    
    bool call(const FunctionDefine *func,const QVariantList &in,QVariantList &out);

    void objectChanged(JZNodeObject *sender,const QString &name);    

    void print(const QString &log);

signals:
    void sigParamChanged();
    void sigRuntimeError(JZNodeRuntimeError error);
    void sigLog(const QString &log);
    void sigStatusChanged(int status);

protected:
    enum{
        Command_none,
        Command_pause,
        Command_resume,
        Command_stop,
    };
    

    class ConnectInfo
    {
    public:
        QString sender;        
        int eventType;
        QString recv;
        FunctionDefine *handle;
    };

    class ConnectCache
    {
    public:
        QString sender;
        JZNodeObject *parentObject;
        int eventType;
        JZNodeObject *recvObject;
        FunctionDefine *handle;
    };

    class JZObjectConnect
    {
    public:        
        JZObjectConnect();

        int eventType;
        JZNodeObject *sender;
        JZNodeObject *receiver;
        FunctionDefine *handle;
    };

    class ParamChangeEvent
    {
    public:
        ParamChangeEvent();

        QString name;        
        JZNodeObject *receiver;
        FunctionDefine *handle;
    };

    virtual void customEvent(QEvent *event) override;
    void clear();
    bool checkIdlePause(const FunctionDefine *func);  //return is stop
    bool checkPauseStop();  //return is stop
    bool run();         
    void updateStatus(int status);

    const FunctionDefine *function(QString name);       
    void callCFunction(const FunctionDefine *func);        
    QVariant dealExpr(const QVariant &a, const QVariant &b,int op);
    QVariant dealExprInt(const QVariant &a, const QVariant &b, int op);
    QVariant dealExprDouble(const QVariant &a, const QVariant &b, int op);

    void dealQtEvent(JZEvent *event);
    void dealSingleEvent(JZEvent *event);

    void pushStack(const FunctionDefine *define);
    void popStack();
    int indexOfBreakPoint(QString filepath,int nodeId);
    void waitCommand();
        
    QVariant getParam(const JZNodeIRParam &param);
    void setParam(const JZNodeIRParam &param,const QVariant &value);        
        
    int nodeIdByPc(int pc);        
    int nodeIdByPc(JZNodeScript *script,int pc);
    int breakNodeId(int pc, int skip_id);

    JZNodeScript *getScript(QString path);
    JZNodeScript *getObjectScript(QString objName);
    /*
     1. 赋值变量时作为 sender 连接所有
     2. 赋值变量时作为 this.xxx 连接父类
     3. CreateObject 连接自身 this 部分
     */
    void connectParamChanged(JZNodeObject *obj,JZNodeScript *script);
    void connectClassEvent(JZNodeObject *obj);
    void connectVariableEvent(QString name,JZNodeObject *obj);
    void connectVariableThis(QString name, JZNodeObject *obj, JZNodeObject *recv);
    void connectObject(JZObjectConnect connect);
    
    bool isWatch();        
    bool splitMember(const QString &fullName,QString &objName,QString &memberName);
    void unSupportOp(int a,int b,int op);

    int m_pc;
    int m_breakNodeId;
    JZNodeProgram *m_program;    
    JZNodeScript *m_script;
    QWidget *m_window;    
        
    QList<BreakPoint> m_breakPoints;
    BreakPoint m_breakStep;        

    QList<int> m_watchParam;

    Stack m_stack;
    QMap<QString,QVariant> m_global;            
    QMap<int,QVariant> m_regs;        
    QVariant m_this;
           
    FunctionDefine m_idleFunc;
    int m_statusCommand;
    int m_status;     
    QMutex m_mutex;    
    QWaitCondition m_waitCond;
    bool m_debug;

    QList<ConnectInfo> m_connectInfo;
    QMap<JZNodeObject*,QList<JZObjectConnect>> m_connects;
    QList<ConnectCache> m_connectCache;
    QList<ParamChangeEvent> m_paramChangeHandle;        
};
extern JZNodeEngine *g_engine;

void JZScriptLog(const QString &name);
void JZScriptInvoke(const QString &function, const QVariantList &in, QVariantList &out);

#endif
