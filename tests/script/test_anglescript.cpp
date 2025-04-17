#include <QTest>
#include <QFile>
#include <QTextStream>
#include "test_anglescript.h"
#include "JZNodeBuilder.h"
#include "JZNodeUtils.h"
#include "JZScriptConvert.h"

AngleScriptTest::AngleScriptTest()
{
    
}

void AngleScriptTest::testHello()
{
    return;
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

void AngleScriptTest::testIf()
{    
    return;
    QString code = R"(int testIf(int n) {
    if (n >= 8) {
        if (n >= 12) {
            if (n >= 14) {
                if (n >= 15) {
                    return 15;
                } else {
                    return 14;
                }
            } else {
                if (n >= 13) {
                    return 13;
                } else {
                    return 12;
                }
            }
        } else {
            if (n >= 10) {
                if (n >= 11) {
                    return 11;
                } else {
                    return 10;
                }
            } else {
                if (n >= 9) {
                    return 9;
                } else {
                    return 8;
                }
            }
        }
    } else {
        if (n >= 4) {
            if (n >= 6) {
                if (n >= 7) {
                    return 7;
                } else {
                    return 6;
                }
            } else {
                if (n >= 5) {
                    return 5;
                } else {
                    return 4;
                }
            }
        } else {
            if (n >= 2) {
                if (n >= 3) {
                    return 3;
                } else {
                    return 2;
                }
            } else {
                if (n >= 1) {
                    return 1;
                } else {
                    return 0;
                }
            }
        }
    }
}
        )";

    if(!buildAs(code))
        return;
    dump("as_testIf");
    
    for(int i = 0; i < 16; i++)
    {
        QVariantList in,out;
        in << i;
        
        bool ret = call("testIf",in,out);
        QVERIFY(ret);
        QCOMPARE(out[0].toInt(),i);
    }
}

void AngleScriptTest::testFor()
{
    QString code = R"(void testFor() {       
        int result = 0;
        for (int i = 0; i < n; i++) {
            result += i;
        }
        return result;        
    })";

    if(!buildAs(code))
        return;
    dump("as_testFor");
    
    auto testFor = [](int n)->int {       
        int result = 0;
        for (int i = 0; i < n; i++) {
            result += i;
        }
        return result;        
    };

    for(int i = 0; i < 10; i++)
    {
        QVariantList in,out;
        in << i;
        
        bool ret = call("testFor",in,out);
        QVERIFY(ret);
        QCOMPARE(out[0].toInt(), testFor(i));
    }
}

void AngleScriptTest::testWhile()
{    
    QString code = R"(int testWhile(int n) {
        int result = 0;
        int i = 0;
        while(i < n) {
            result += i;
            i = i + 1;
        }
        return result;
    })";

    if (!buildAs(code))
        return;
    dump("as_testWhile");

    auto testWhile = [](int n)->int {
        int result = 0;
        int i = 0;
        while(i < n) {
            result += i;
            i++;
        }
        return result;
    };

    for (int i = 0; i < 10; i++)
    {
        QVariantList in, out;
        in << i;

        bool ret = call("testWhile", in, out);
        QVERIFY(ret);
        QCOMPARE(out[0].toInt(), testWhile(i));
    }
}

void AngleScriptTest::testSwitch()
{

}

void test_anglescript(int argc, char *argv[])
{
    AngleScriptTest test;
    QTest::qExec(&test,argc,argv);
}