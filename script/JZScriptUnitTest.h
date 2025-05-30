#ifndef JZ_SCRIPT_UNIT_TEST_H_
#define JZ_SCRIPT_UNIT_TEST_H_

#include <functional>
#include "JZScriptItem.h"
#include "JZScriptItemVisitor.h"
#include "JZNodeEngine.h"

class JZScriptItemDepend
{
public:
    enum RunStatus
    {
        None,
        Running,
        Successed,
        Failed,
        Cancel,
    };

    //这里存的node节点是原脚本的
    struct ParamDepend
    {
        int node_id;
        QVariant value;
    };

    struct FunctionDepend
    {
        int node_id;
        QVariant value;
    };
    
    JZScriptItemDepend();
    void init(JZScriptItem *script);
    
    bool isError();
    void setParam(int id,const QVariant &value);
    JZFunctionDefine function;

    QString error;    
    
    QList<ParamDepend> paramList;
    QList<FunctionDepend> functionList;       

    JZScriptItem *initExtScript;
    
    bool isRunOnce;
    bool isTriggeScriptr;         //是否有触发函数,对于事件，有触发函数
    JZScriptItem *triggerScript;  //触发    
    JZScriptItem *unitScript;
    JZScriptItem *originScript;   //原始

    RunStatus status;
    QVariantList input;
    QVariantList output;
};
typedef QSharedPointer<JZScriptItemDepend> JZScriptItemDependPtr;

class JZScriptUnitTest;
class JZNodeUnitTest : public JZNode
{
public:
    JZNodeUnitTest();
    ~JZNodeUnitTest();

    void copyFrom(const JZNode *node);
    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;

    JZScriptUnitTest *hook;
};

//JZScriptUnitTestVistor
class JZScriptUnitTestVistor: public JZScriptItemVisitor
{
public:
    JZScriptUnitTestVistor();

    virtual void updateDepend(JZScriptItemDepend *depend);

protected:    
    JZScriptItemDepend *m_depend;
};

//JZScriptHookVistor
class JZScriptHookVistor : public JZScriptUnitTestVistor
{
public:
    JZScriptHookVistor();

    virtual void visitorSelf(JZNode *node) override;
protected:    

};

class JZScriptUnitTest : public QObject
{
    Q_OBJECT

public:    
    JZScriptUnitTest();
    ~JZScriptUnitTest();

    void setProject(JZProject* project);
    void setScript(JZScriptItem *script);

    JZScriptItemDepend *depend();
    JZNodeEngine *engine();
    void dump(QString dir);

    bool isInit();
    bool init();
    void deinit();

    void start(bool once);
    void stop();
    bool isFinish();
    bool waitFinish(int timeout = 5000);
    void setFinish(bool flag);
    
    bool run(int timeout = 5000);

    bool hasHook(int id);
    QVariant hookValue(int id);

signals:

protected slots:
    void onRuntimeError();

protected:
    void hookEngine();
    void initRuntime();    
    void updateStatus(JZScriptItemDepend::RunStatus status);

    JZProject* m_project;
    JZScriptItem *m_script;     
    JZScriptItem *m_initExtScript;
    JZScriptItem* m_triggerScript;
    QString m_error;    
    
    JZScriptItemDepend m_depend;
    QMap<int,QVariant> m_hookValues;
    
    JZNodeProgram m_program;
    JZNodeEngine *m_engine;
    JZNodeObjectPointer m_object;
};

//JZScriptUnitTestManager
class JZScriptUnitTestManager
{
public:
    static JZScriptUnitTestManager* instance();
    void initEnv(JZScriptEnvironment* env);

    void regist(JZScriptUnitTestVistor* replace);
    void updateDepend(JZScriptItemDepend* depend);

public:
    JZScriptUnitTestManager();
    ~JZScriptUnitTestManager();

    QList<JZScriptUnitTestVistor*> m_replaceList;
};

#endif

