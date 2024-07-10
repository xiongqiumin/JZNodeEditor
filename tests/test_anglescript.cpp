#include <QTest>
#include <QFile>
#include <QTextStream>
#include "test_anglescript.h"
#include "JZNodeBuilder.h"
#include "JZNodeUtils.h"

AngleScriptTest::AngleScriptTest()
{
    
}

bool AngleScriptTest::buildAs(QString code)
{
    auto script_file = m_project.getScriptFile(m_project.mainFunction());

    ASConvert convert;
    if(!convert.convert(code,script_file))
    {
        QTest::qVerify(false, "convert", convert.error().toLocal8Bit().data(), __FILE__, __LINE__);
        return false;
    }

    return build();
}

void AngleScriptTest::testHello()
{
    QString code = R"(int add(int a,int b)
        {
            return 100 + a * b - (b - a) * (b + a) + 298/5 + 1;
        }
        )";

    if(!buildAs(code))
        return;
    
    int a = 8,b = 2;
    QVariantList in,out;
    in << a << b;
    
    bool ret = call("add",in,out);
    QVERIFY(ret);
    QCOMPARE(out[0].toInt(),100 + a * b - (b - a) * (b + a) + 298/5 + 1);
}

void AngleScriptTest::testFab()
{
    QString code = R"(int testIf(int n)
        {
            if(n == 10)
            {
                return 10;
            }
            else if(n == 9)
                return 9;
            else if(n == 8)
                return 8;
            else if(n == 7)
                return 7;
            else if(n == 6)
                return 6;
            else if(n == 5)
                return 5;
            else if(n == 4)
                return 4;
            else if(n == 3)
                return 3;
            else if(n == 2)
                return 2;
            else if(n == 1)
                return 1;
            else
                return 0;
        }
        )";

    if(!buildAs(code))
        return;
    
    for(int i = 0; i < 10; i++)
    {
        QVariantList in,out;
        in << i;
        
        bool ret = call("testIf",in,out);
        QVERIFY(ret);
        QCOMPARE(out[0].toInt(),i);
    }
}

void test_anglescript(int argc, char *argv[])
{
    AngleScriptTest test;
    QTest::qExec(&test,argc,argv);
}