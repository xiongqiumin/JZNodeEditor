#ifndef JZ_SCRIPT_UNIT_TEST_H_
#define JZ_SCRIPT_UNIT_TEST_H_

#include "JZScriptItem.h"
#include "JZScriptItemVisitor.h"


class JZScriptItemDepend
{
public:
    QList<int> m_dependNode;
};

class JZNodeUnitTest : public JZNode
{
public:
    JZNodeUnitTest();
    ~JZNodeUnitTest();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
};

class JZScriptItemUnitTest
{
public:
    JZScriptItemUnitTest();
    ~JZScriptItemUnitTest();

    void init(JZScriptItem *script);
    void applyDepends(const JZScriptItemDepend &depend);

protected:

    JZScriptItem *m_script;        
};

#endif

