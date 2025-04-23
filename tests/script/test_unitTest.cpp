#include <QTest>
#include <QFile>
#include <QTextStream>
#include "test_unitTest.h"
#include "JZNodeBuilder.h"
#include "JZNodeUtils.h"
#include "JZScriptUnitTest.h"

TestUnitTest::TestUnitTest()
{
    
}

bool TestUnitTest::buidUnitTest(JZScriptItem *unit_script_item)
{
    JZProjectTempGuard guard(&m_project, unit_script_item, true);

    m_engine.deinit();
    if (!build())
        return false;

    JZNodeCompiler compiler;
    JZNodeScriptPtr unit_script = JZNodeScriptPtr(new JZNodeScript());
    if (!compiler.build(unit_script_item, unit_script.data()))
    {
        qDebug() << "build unit test failed";
        return false;
    }

    m_program.addScript(unit_script_item->itemPath(), unit_script);
    m_engine.deinit();
    m_engine.init();
    return true;
}

void TestUnitTest::testHello()
{
    QString code = R"(int add(int a,int b)
        {
            return a + pow(2,4) + b;
        }
    )";    

    if(!buildAs(code))
        return;

    JZScriptItem *add_script = m_file->getFunction("add");

    JZScriptUnitTest unit;
    unit.setProject(&m_project);

    JZScriptItemDependPtr ptr = unit.genDepend(add_script);
    ptr->setFunction("pow", 800.0);

    JZScriptItem *unit_script_item = unit.createUnitScript(ptr);
    if (!buidUnitTest(unit_script_item))
        return;

    QString unit_test = unit_script_item->function().fullName();
    unit.registEnv(m_engine.environment());
    
    QVariantList in, out;
    in << 1 << 2;
    bool ret = call(unit_test,in,out);
    QVERIFY(ret);    
    QCOMPARE(out[0].toInt(), 803);
}



void test_unitTest(int argc, char *argv[])
{
    TestUnitTest test;
    QTest::qExec(&test,argc,argv);
}