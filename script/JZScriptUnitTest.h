#ifndef JZ_SCRIPT_UNIT_TEST_H_
#define JZ_SCRIPT_UNIT_TEST_H_

#include "JZScriptItem.h"
#include "JZScriptItemVisitor.h"

class JZScriptItemDepend
{
public:
    struct ParamDepend
    {
        int id;
        QVariant value;
    };

    struct FunctionDepend
    {
        int id;
        QVariant value;
    };
    
    QVariantList inputList;
    QList<ParamDepend> paramList;
    QList<FunctionDepend> functionList;
};
typedef QSharedPointer<JZScriptItemDepend> JZScriptItemDependPtr;

class JZNodeUnitTest : public JZNode
{
public:
    JZNodeUnitTest();
    ~JZNodeUnitTest();

    void fromNode(JZNode *node);
    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
};

class JZScriptUnitTestVistor: public JZScriptItemVistor
{
public:
    JZScriptUnitTestVistor();

    virtual void visitorSelf(JZNode *node) override;

    JZScriptItemDependPtr depend;
};

class JZScriptUnitTest
{
public:
    static JZScriptUnitTest *instance();

    JZScriptUnitTest();
    ~JZScriptUnitTest();

    void setProject(JZProject* project);
    JZScriptItemDependPtr init(JZScriptItem *script);
    void build();
    void applyDepends(JZScriptItemDependPtr depend);

    bool hasHook(int id);
    QVariant hookValue(int id);

protected:
    JZProject* m_project;
    JZScriptItem *m_script;        
    QMap<int,QVariant> m_hookValues;
};

void JZScriptUnitTestInit();

#endif

