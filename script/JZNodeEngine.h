#ifndef JZNODE_ENGINE_H_
#define JZNODE_ENGINE_H_

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QThread>
#include <QException>
#include <QWaitCondition>
#include <QSet>
#include <QAtomicInt>
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

class JZCORE_EXPORT RunnerEnv
{
public:
    RunnerEnv();
    ~RunnerEnv();
    
    void initVariable(QString name, const QVariant &value);
    void initVariable(int id, const QVariant &value);

    QVariant *getRef(int id);
    QVariant *getRef(QString name);    

    void clearIrCache();
    void applyIrCache();

    const JZFunction *function;
    QVariant object;      //this    
    JZNodeScript *script;    
    int pc;
    int inCount;          //传入参数数量
    int printNode;
    QMap<int, QVariant> watchMap;
    
    QMap<QString,QVariantPtr> locals;
    QMap<int,QVariantPtr> stacks;    
    QMap<JZNodeIRParam*,QVariant*> irParamCache; //为了加速JZNodeIRParam访问的缓存
};

class JZCORE_EXPORT Stack
{
public:
    Stack();
    ~Stack();

    void clear();
    int size() const;
    bool isEmpty() const;

    void pop();
    void push();

    RunnerEnv *currentEnv();
    RunnerEnv *env(int index);    

protected:   
    QList<RunnerEnv> m_env;
};

class JZCORE_EXPORT JZNodeRuntimeInfo
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

class JZCORE_EXPORT JZNodeRuntimeError
{
public:
    bool isError();
    QString errorReport();

    QString error;
    JZNodeRuntimeInfo info;
};
QDataStream &operator<<(QDataStream &s, const JZNodeRuntimeError &param);
QDataStream &operator>>(QDataStream &s, JZNodeRuntimeError &param);

//UnitTestResult
class JZCORE_EXPORT UnitTestResult
{
public:
    enum{
        None,
        Finish,
        Error,
        Cancel,
    };

    UnitTestResult();
    
    int result;
    QString function;
    QVariantList out;
    JZNodeRuntimeError runtimeError;
};
typedef QSharedPointer<UnitTestResult> UnitTestResultPtr;

//BreakStep
class JZCORE_EXPORT BreakStep
{
public: 
    enum Type{
        none,
        stepOver,     //不为nodeId中断
        stackEqual,  
    };

    BreakStep();
    void clear();

    Type type;
    QString file;
    int nodeId;            
    int stack;
};

//JZNodeEngine
class JZNodeDebugServer;
class JZCORE_EXPORT JZNodeEngine : public QObject
{
    Q_OBJECT

public:
    static void regist();
    
    JZNodeEngine();
    virtual ~JZNodeEngine();

    void init();
    void deinit();

    void statClear();
    void statReport();

    void setProgram(JZNodeProgram *program);
    JZNodeProgram *program();        

    int status();
    JZNodeRuntimeInfo runtimeInfo();    
    JZNodeRuntimeError runtimeError();

    void setDebug(bool flag);    

    void addBreakPoint(QString filepath,int nodeId);
    void addBreakPoint(const BreakPoint &pt);
    void removeBreakPoint(QString filepath,int nodeId);
    void clearBreakPoint();    

    void pause();
    void resume();    
    void stop();
    void stepIn();
    void stepOver();
    void stepOut();        
       
    Stack *stack();    
    
    QVariant createVariable(int type,const QString &value = QString());

    QVariant getParam(int stack_level,const JZNodeIRParam &param);
    void setParam(int stack_level, const JZNodeIRParam &param, const QVariant &value);

    QVariant getParam(const JZNodeIRParam &param);
    void setParam(const JZNodeIRParam &param, const QVariant &value);
            
    QVariant getVariable(const QString &name);
    void setVariable(const QString &name, const QVariant &value);
    
    const QVariant &getReg(int reg);
    void setReg(int reg, const QVariant &value);

    QVariant getSender();    

    void watchNotify();         //node display
    void printNode();
    QVariant dealExpr(const QVariant &a, const QVariant &b, int op);

    bool call(const QString &function,const QVariantList &in,QVariantList &out);    
    bool call(const JZFunction *func,const QVariantList &in,QVariantList &out);
    bool callUnitTest(ScriptDepend *depend,QVariantList &out);
    void invoke(const QString &function,const QVariantList &in,QVariantList &out);
    void onSlot(const QString &function,const QVariantList &in,QVariantList &out);
    void print(const QString &log);
    void printMemory();
    int regInCount();

signals:    
    void sigRuntimeError(JZNodeRuntimeError error);
    void sigLog(const QString &log);
    void sigStatusChanged(int status);
    void sigWatchNotify();

protected slots:
    void onWatchTimer();

protected:
    enum{
        Command_none,
        Command_pause,
        Command_resume,
        Command_stop,
    };

    struct Stat
    {
        Stat();

        void clear();
        void report();

        int statmentTime;
        int callTime;
        int exprTime;
        int getTime;
        int setTime;
    };

    virtual void customEvent(QEvent *event) override;
    void clear();
    bool checkIdlePause(const JZFunction *func);  //return is stop
    bool checkPause(int node_id);
    bool run();         
    void updateStatus(int status);

    const JZFunction *function(QString name,const QVariantList *list);
    const JZFunction *function(JZNodeIRCall *ir_call);

    void checkFunctionIn(const JZFunction *func);
    void checkFunctionOut(const JZFunction *func);
    void callCFunction(const JZFunction *func);    
    QVariant dealExprInt(const QVariant &a, const QVariant &b, int op);
    QVariant dealExprInt64(const QVariant &va, const QVariant &vb, int op);
    QVariant dealExprDouble(const QVariant &a, const QVariant &b, int op);        
    QVariant dealSingleExpr(const QVariant &a, int op);
    void dealSet(QVariant *ref, const QVariant &value);

    void initGlobal(QString name, const QVariant &v);
    void initLocal(QString name, const QVariant &v);
    void initLocal(int id, const QVariant &v);
    void clearReg();

    void pushStack(const JZFunction *define);
    void popStack();
    int indexOfBreakPoint(QString filepath,int nodeId);
    void waitCommand();
    bool breakPointTrigger(int node_id);    
    
    QVariant *getParamRef(int stack_level,const JZNodeIRParam &param);
    JZNodeObject *getVariableObject(QVariant *ref, const QStringList &name);        
        
    int nodeIdByPc(int pc);        
    int nodeIdByPc(JZNodeScript *script,QString func, int pc);    
    NodeRange nodeDebugRange(int node_id, int pc);
    int breakNodeId();
    JZFunctionDebugInfo *currentFunctionDebugInfo();

    JZNodeScript *getScript(QString path); 
        
    void splitMember(const QString &fullName,QStringList &objName,QString &memberName);
    void unSupportSingleOp(int a,int op);
    void unSupportOp(int a,int b,int op);

    bool isWidgetFunction(const JZFunction *function);
    void updateHook();
    
    int m_pc;    
    JZNodeProgram *m_program;    
    JZNodeScript *m_script;
    QWidget *m_window;    
        
    QList<BreakPoint> m_breakPoints;
    BreakStep m_breakStep; 
    int m_breakNodeId;

    Stack m_stack;
    int m_regInCount;
    QMap<QString,QVariantPtr> m_global;
    QVector<QVariant> m_regs;
    JZNodeObject *m_sender;
    qint64 m_watchTime;
           
    JZFunction m_idleFunc;
    QAtomicInt m_statusCommand;
    int m_status;     
    QMutex m_mutex;    
    QWaitCondition m_waitCond;
    bool m_debug;
    JZNodeRuntimeError m_error;

    ScriptDepend *m_depend;
    QMap<int,QVariantList> m_dependHook;
    bool m_hookEnable;
    QTimer *m_watchTimer;
    
    Stat m_stat;
};
extern JZNodeEngine *g_engine;

void JZScriptLog(const QString &name);
void JZScriptInvoke(const QString &function, const QVariantList &in, QVariantList &out);
void JZScriptOnSlot(const QString &function, const QVariantList &in, QVariantList &out);

#endif
