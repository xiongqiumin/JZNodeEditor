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
#include "JZNodeObject.h"
#include "JZScriptEnvironment.h"
#include "JZNodeTrace.h"
#include "JZEngineCoroutine.h"

enum JZEngineStatus{
    Status_none,
    Status_idle,
    Status_running,
    Status_pause,
    Status_error,
};

class RunnerEnv
{
public:
    RunnerEnv();
    ~RunnerEnv();
    
    void initVariable(QString name, QVariantPtr ptr);
    void initVariable(int id, QVariantPtr ptr);
    void deinitVariable(QString name);
    void deinitVariable(int id);

    QVariantPtr *getRef(int id);
    QVariantPtr *getRef(const QString &name);

    const JZFunction *function;
    QVariantPtr  self;  //this    
    const JZNodeScript *script;
    int pc;
    int inCount;          //传入参数数量    
    
    QMap<QString,QVariantPtr> locals;
    QMap<int,QVariantPtr> stacks;
};

class Stack
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

struct TryCatchInfo
{
    int stack;
    int catchPc;
    JZNodeIRParam irExcep;
};

struct NodeTraceInfo
{
    NodeTraceInfo();
    void clear();

    QString toString() const;

    int nodeId;
    QString name;
    QMap<QString, QString> input;
    QMap<QString, QString> output;
};

struct JZNodeCoroutine
{
    JZNodeCoroutine();

    int id;
    QString name;
    JZEngineStatus status;
    JZEngineCoroutine* coTask;

    Stack stack;
    QVector<QVariant> regs;
    QList<TryCatchInfo> tryCatchList;
    JZNodeObject* sender;
    JZNodeTraceContext traceContext;    

    int pc;
    const JZNodeScript* script;
    NodeTraceInfo nodeTrace;
};
typedef QSharedPointer<JZNodeCoroutine> JZNodeCoPtr;

class JZNodeRuntimeInfo
{
public:
    struct Stack 
    {
        Stack();

        QString function;
        QString scriptItemPath;
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
    bool isError() const;
    QString errorReport() const;

    QString error;
    JZNodeRuntimeInfo info;
};
QDataStream &operator<<(QDataStream &s, const JZNodeRuntimeError &param);
QDataStream &operator>>(QDataStream &s, JZNodeRuntimeError &param);

//BreakStep
class BreakStep
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
    QString scriptItemPath;
    int nodeId;            
    int stack;
};

//JZEngineTraceConfig
class JZEngineTraceConfig
{
public:
    JZEngineTraceConfig();

    bool enable;
};

//JZNodeEngine
class JZNodeDebugServer;
class JZNodeEngine : public QObject
{
    Q_OBJECT

public:        
    static void regist();

    JZNodeEngine(QObject *parent = nullptr);
    virtual ~JZNodeEngine();

    bool isInit() const;
    bool init();
    void deinit();

    void statClear();
    void statReport();

    void setProgram(const JZNodeProgram *program);
    const JZNodeProgram *program();
    
    JZEngineStatus status();
    JZNodeRuntimeInfo runtimeInfo();    
    JZNodeRuntimeError runtimeError();
    QString currentFunction();

    void setNodeTrace(JZEngineTraceConfig config);
    void setDebug(bool flag);        

    void addBreakPoint(QString filepath,int nodeId);
    void addBreakPoint(const BreakPoint &pt);
    void removeBreakPoint(QString filepath,int nodeId);
    void clearBreakPoint();    

    bool isPauseOrError();
    
    void pause();
    void resume();    
    void stop();    
    void stepIn();
    void stepOver();
    void stepOut();      

    QList<int> coList();
    JZNodeCoroutine *co(int id);
    JZNodeCoroutine* currentCo();

    int createCo(JZEngineCoroutine *task);
    void destoryCo(int id);
    void switchCo(int id); 
    bool isInterruptCo();
    void stopAllCo();

    Stack *currentStack();    
    JZScriptEnvironment *environment();
    
    QVariant createVariable(int type,const QString &value = QString());

    bool hasParam(int stack_level, const JZNodeIRParam &param);
    QVariant getParam(int stack_level,const JZNodeIRParam &param);
    void setParam(int stack_level, const JZNodeIRParam &param, const QVariant &value);

    QVariant getParam(const JZNodeIRParam &param);
    void setParam(const JZNodeIRParam &param, const QVariant &value);
            
    QVariant getVariable(const QString &name);
    void setVariable(const QString &name, const QVariant &value);
    
    const QVariant &getReg(int reg);
    void setReg(int reg, const QVariant &value);
    int regInCount();

    QVariantPtr *getParamRef(int stack_level, const JZNodeIRParam &param);
    void dealSet(QVariantPtr *ref, const QVariant &value);

    QVariant getSender();    

    void startWatch();
    void stopWatch();
    void watchNotify();         //node display

    JZNodeTraceContext *currentTraceContext();

    void collectNodeParam(int node_id,bool is_input);
    void printNode();
    QVariant dealExpr(const QVariant &a, const QVariant &b, int op);
    QVariant dealSingleExpr(const QVariant& a, int op);

    //外部用
    bool call(const QString &function,const QVariantList &in,QVariantList &out);
    bool call(const JZFunction* func, const QVariantList& in, QVariantList& out);
    bool callVirtual(const QString& function, const QVariantList& in, QVariantList& out);
    
    //内部用
    void invoke(const QString &function,const QVariantList &in,QVariantList &out);
    void invokeVirtual(const QString& function, const QVariantList& in, QVariantList& out);
    void onSlot(const QString &function,const QVariantList &in,QVariantList &out);
    void print(const QString &log);
    void printMemory();

signals:    
    void sigRuntimeError(JZNodeRuntimeError error);
    void sigLog(const QString &log);
    void sigStatusChanged(JZEngineStatus status);
    void sigNodeTrace(const NodeTraceInfo& node_trace);
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
    bool checkPause(int node_id);
    bool run();             
    void updateStatus(JZEngineStatus status);

    const JZFunction *function(QString name);
    const JZFunction *function(const JZNodeIRCall *ir_call);
    const JZFunction *virtualFunction(JZNodeObject *obj,QString name);

    void checkFunctionIn(const JZFunction *func);
    void checkFunctionOut(const JZFunction *func);
    void callCFunction(const JZFunction *func); 

    QVariantPtr initVariantPtr(int data_type);
    void initGlobal(QString name, int data_type);
    void initLocal(QString name, int data_type);
    void initLocal(int id, int data_type);
    void deinitGlobal(QString name);
    void deinitLocal(QString name);
    void deinitLocal(int id);
    void clearReg();

    void pushStack(const JZFunction *define);
    void popStack();
    int indexOfBreakPoint(QString filepath,int nodeId);
    void waitCommand();
    bool breakPointTrigger(int node_id);    
        
    JZNodeObject *getVariableObject(QVariant *ref, const QStringList &name);
        
    int nodeIdByPc(int pc);        
    int nodeIdByPc(const JZNodeScript *script,QString func, int pc);
    NodeRange nodeDebugRange(int node_id, int pc);
    int breakNodeId();
    const JZFunctionDebugInfo *currentFunctionDebugInfo();

    const JZNodeScript *getScript(QString path);
    
    void pushTryCatch(JZNodeIRTry *ir);
    void popTryCatch();
    bool catchException(QString tips); 
    void waitAllCoExcept(JZNodeCoroutine *cur_co);    
    JZEngineStatus statusUnLock();

    const JZNodeProgram *m_program;
        
    QList<BreakPoint> m_breakPoints;
    BreakStep m_breakStep; 
    QSet<const JZNodeIRNodeEnter*> m_breakIr;
    JZEngineTraceConfig m_traceConfig;

    JZScriptEnvironment m_env;
    QMap<QString,QVariantPtr> m_global;
           
    JZFunction m_idleFunc;
    QAtomicInt m_statusCommand;
    JZEngineStatus m_status;
    QMutex m_mutex;    
    QWaitCondition m_waitCond;
    bool m_debug;
        
    JZNodeRuntimeError m_error;
    QList<TryCatchInfo> m_tryCatchList;

    JZNodeCoroutine *m_co;
    JZNodeCoPtr m_mainCo;
    QMap<int, JZNodeCoPtr> m_coMap;
    int m_coId;

    bool m_watch;
    Stat m_stat;
};
extern thread_local JZNodeEngine *g_engine;

void JZScriptLog(const QString &name);
QVariant JZConvertVariant(const QVariant &in, int type);
QString JZObjectToString(JZNodeObject* obj);
JZNodeObject* JZObjectFromString(int type, const QString& text);
void JZScriptInvoke(const QString &function, const QVariantList &in, QVariantList &out);
void JZScriptOnSlot(const QString &function, const QVariantList &in, QVariantList &out);

#endif
