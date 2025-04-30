#ifndef JZ_SCRIPT_UNIT_TEST_H_
#define JZ_SCRIPT_UNIT_TEST_H_

#include <functional>
#include "JZScriptItem.h"
#include "JZScriptItemVisitor.h"
#include "JZNodeEngine.h"

class JZScriptItemDepend
{
public:
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
    void clear();
    
    bool isError();
    void setParam(int id,const QVariant &value);
    
    QString error;
    JZFunctionDefine function;
    
    QList<ParamDepend> paramList;
    QList<FunctionDepend> functionList;
    
    QVariantList input;
    QVariantList output;

    JZScriptItem *initExtScript;
    bool isTrigger;
    JZScriptItem *triggerScript; //触发
    
    JZScriptItem *unitScript;
    JZScriptItem *script;        //原始
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
class JZScriptUnitTestVistor: public JZScriptItemVistor
{
public:
    JZScriptUnitTestVistor();

    virtual void updateDepend(JZScriptItemDepend *depend);

protected:    
    JZScriptItemDepend *m_depend;
};

//JZScriptNomarlVistor
class JZScriptNomarlVistor: public JZScriptUnitTestVistor
{
public:
    JZScriptNomarlVistor();

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

    JZScriptItemDepend *genDepend(JZScriptItem *script);
    JZNodeEngine *engine();
    void dump(QString dir);

    bool init();
    void deinit();

    void start();
    void stop();
    bool isFinish();
    bool waitFinish(int timeout = 5000);
    void setFinish(bool flag);
    
    bool run(int timeout = 5000);

    bool hasHook(int id);
    QVariant hookValue(int id);

protected slots:
    void onRuntimeError();

protected:
    void initRuntime();
    virtual void timerEvent(QTimerEvent* event) override;

    JZProject* m_project;
    JZScriptItem *m_script;     
    JZScriptItem *m_initExtScript;
    JZScriptItem* m_triggerScript;
    QString m_error;
    int m_timeId;
    
    JZScriptItemDepend m_depend;
    QMap<int,QVariant> m_hookValues;

    bool m_isFinish;
    JZNodeProgram m_program;
    JZNodeEngine m_engine;
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

