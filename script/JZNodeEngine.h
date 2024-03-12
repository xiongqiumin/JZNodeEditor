﻿#ifndef JZNODE_ENGINE_H_
#define JZNODE_ENGINE_H_

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QThread>
#include <QException>
#include <QWaitCondition>
#include <QSet>
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
    
    void initVariable(QString name, const QVariant &value);
    void initVariable(int id, const QVariant &value);

    QVariant *getRef(int id);
    QVariant *getRef(QString name);

    const JZFunction *func;
    QVariant object;      //this    
    JZNodeScript *script;    
    int pc;
    
    JZVariantMap locals;
    JZVariantIntMap stacks;
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
    NodeRange pcRange;
};

//JZNodeEngine
class JZNodeDebugServer;
class JZNodeEngine : public QObject
{
    Q_OBJECT

public:
    static void regist();
    
    JZNodeEngine();
    virtual ~JZNodeEngine();

    void init();
    void deinit();

    void setProgram(JZNodeProgram *program);
    JZNodeProgram *program();        

    JZNodeRuntimeInfo runtimeInfo();    
    JZNodeRuntimeError runtimeError();

    void setDebug(bool flag);    

    void addBreakPoint(QString filepath,int nodeId);
    void removeBreakPoint(QString filepath,int nodeId);
    void clearBreakPoint();    

    void pause();
    void resume();    
    void stop();
    void stepIn();
    void stepOver();
    void stepOut();        

    void initGlobal(QString name, const QVariant &v);
    void initLocal(QString name, const QVariant &v);
    void initLocal(int id, const QVariant &v);
       
    Stack *stack();    

    QVariant *getVariableRef(int id);
    QVariant *getVariableRef(int id, int stack_level);
    QVariant getVariable(int id);
    void setVariable(int id, const QVariant &value);

    QVariant *getVariableRef(const QString &name);        
    QVariant *getVariableRef(const QString &name,int stack_level);
    QVariant getVariable(const QString &name);
    void setVariable(const QString &name, const QVariant &value);

    QVariant getReg(int id);
    QVariant *getRegRef(int id);
    void setReg(int id, const QVariant &value);

    QVariant getThis();
    QVariant getSender();    
     
    void connectEvent(JZNodeObject *sender, int event, JZNodeObject *recv, QString handle);
    void disconnectEvent(JZNodeObject *sender, int event, JZNodeObject *recv, QString handle);
    int connectCount(JZNodeObject *sender, int event);
    void connectSelf(JZNodeObject *object);
    void connectSingleLater(QVariant *v);

    void widgetBind(QWidget *w,QVariant *ref,int dir);
    void widgetUnBind(QWidget *w);
    void widgetUnBindNotify(QWidget *w);
    void watchNotify(int param_id);         //node display    

    void dealEvent(JZEvent *event);    
    void dealSlot(JZEvent *event);
    QVariant dealExpr(const QVariant &a, const QVariant &b, int op);

    bool call(const QString &function,const QVariantList &in,QVariantList &out);    
    bool call(const JZFunction *func,const QVariantList &in,QVariantList &out);
    
    void objectCreate(JZNodeObject *sender);
    void objectDelete(JZNodeObject *sender);
    void varaiantDeleteNotify(QVariant *v);

    void widgetValueChanged(QWidget *w);
    void valueChanged(QVariant *v);

    void print(const QString &log);

signals:
    void sigNodePropChanged(QString file,int id,QString value);
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
    
    struct ConnectCache
    {
        int eventType;
        QVariant *sender;
        JZNodeObject *recv;        
        QString handle;
    };

    class ParamChangeInfo
    {
    public:
        ParamChangeInfo();

        JZNodeObject *recv;
        QString handle;
    };

    class VariantInfo
    {
    public:
        VariantInfo();

        QList<ConnectCache> connectQueue;
        QList<ParamChangeInfo> paramChanges;
        QWidget *bindWidget;        
    };

    class VariantBindCache
    {
    public:
        QVariant *bindValue;
        int dir;
    };

    class ConnectInfo
    {
    public:        
        ConnectInfo();

        int eventType;
        JZNodeObject *sender;
        JZNodeObject *receiver;
        QString handle;
    };

    class JZObjectInfo
    {
    public:
        JZObjectInfo();

        QList<ConnectInfo> connects;
        int connectQueue;
    };    

    virtual void customEvent(QEvent *event) override;
    void clear();
    bool checkIdlePause(const JZFunction *func);  //return is stop
    bool checkPauseStop();  //return is stop
    bool run();         
    void updateStatus(int status);

    const JZFunction *function(QString name);
    void checkFunction(const JZFunction *func);
    void callCFunction(const JZFunction *func);    
    QVariant dealExprInt(const QVariant &a, const QVariant &b, int op);
    QVariant dealExprDouble(const QVariant &a, const QVariant &b, int op);        
    QVariant dealSingleExpr(const QVariant &a, int op);
    void dealSet(QVariant *ref, const QVariant &value);

    void pushStack(const JZFunction *define);
    void popStack();
    int indexOfBreakPoint(QString filepath,int nodeId);
    void waitCommand();
        
    QVariant getParam(const JZNodeIRParam &param);    
    void setParam(const JZNodeIRParam &param,const QVariant &value);
    QVariant *getVariableRefSingle(RunnerEnv *env,const QString &name);
        
    int nodeIdByPc(int pc);        
    int nodeIdByPc(JZNodeScript *script, int pc);
    NodeRange nodeDebugRange(int node_id, int pc);
    int breakNodeId();

    JZNodeScript *getScript(QString path); 
        
    void splitMember(const QString &fullName,QStringList &objName,QString &memberName);
    void unSupportSingleOp(int a,int op);
    void unSupportOp(int a,int b,int op);

    int m_pc;    
    JZNodeProgram *m_program;    
    JZNodeScript *m_script;
    QWidget *m_window;    
        
    QList<BreakPoint> m_breakPoints;
    BreakPoint m_breakStep;
    BreakPoint m_breakResume; //避免断点在node，按F5后重复停止到当前node。
    int m_breakNodeId;    

    Stack m_stack;
    JZVariantMap m_global;
    JZVariantIntMap m_regs;
    JZNodeObject *m_sender;
    qint64 m_watchTime;
           
    JZFunction m_idleFunc;
    int m_statusCommand;
    int m_status;     
    QMutex m_mutex;    
    QWaitCondition m_waitCond;
    bool m_debug;
    JZNodeRuntimeError m_error;
    
    QMap<JZNodeObject*,JZObjectInfo> m_objectInfo;    
    QMap<QVariant*, VariantInfo> m_variantInfo;
    QMap<QVariant*, VariantBindCache> m_widgetBindCache;
    QSet<QVariant*> m_anyVariants;    
};
extern JZNodeEngine *g_engine;

void JZScriptLog(const QString &name);
void JZScriptInvoke(const QString &function, const QVariantList &in, QVariantList &out);

#endif
