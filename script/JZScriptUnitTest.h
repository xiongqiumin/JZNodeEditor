#ifndef JZ_SCRIPT_UNIT_TEST_H_
#define JZ_SCRIPT_UNIT_TEST_H_

#include "JZScriptItem.h"
#include "JZScriptItemVisitor.h"

class JZScriptItemDepend
{
public:
    //这里存的node节点是原脚本的
    struct ParamDepend
    {
        const JZNodeParam *param;
        QVariant value;
    };

    struct FunctionDepend
    {
        const JZNodeFunction *func;
        QVariant value;
    };
    
    JZScriptItemDepend();
    void setParam(QString name,const QVariant &value);
    void setFunction(QString func, const QVariant &value);
        
    QList<ParamDepend> paramList;
    QList<FunctionDepend> functionList;
    const JZScriptItem *script;
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

class JZScriptUnitTestVistor: public JZScriptItemVistor
{
public:
    JZScriptUnitTestVistor();

    virtual void visitorSelf(const JZNode *node) override;

    JZScriptItemDependPtr depend;
};

class JZScriptUnitTest
{
public:    
    JZScriptUnitTest();
    ~JZScriptUnitTest();

    void setProject(JZProject* project);
    void registEnv(JZScriptEnvironment *env);

    JZScriptItemDependPtr genDepend(const JZScriptItem *script);
    JZScriptItem *createUnitScript(JZScriptItemDependPtr depend);
    JZScriptItem *script();    

    bool hasHook(int id);
    QVariant hookValue(int id);

protected:
    JZProject* m_project;
    JZScriptItem *m_script;        
    QMap<int,QVariant> m_hookValues;
};

void JZScriptUnitTestNodeInit();

#endif

