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

void AngleScriptTest::testExpr()
{
    QString code = R"(double add(double a,double b)
        {
            return 100 + pow(a,b) - pow(b,a);
        }
    )";

    if (!buildAs(code))
        return;
    dump("as_testExpr");

    double a = 8.0, b = 2.0;
    QVariantList in, out;
    in << a << b;

    bool ret = call("add", in, out);
    QVERIFY(ret);
    QCOMPARE(out[0].toDouble(), 100 + pow(a, b) - pow(b, a));
}

void AngleScriptTest::testIf()
{    
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
    QString code = R"(int testFor(int n) {       
        int result = 0;
        int i = 0;
        for (i = 0; i < n; i++) {
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

static int test_complex_c(int num)
{
    int i = 0;
    int j = 0;
    int k = 0;
    int sum = 0;
    for (i = 0; i < num; i++)
    {
        while (i < 5)
        {
            j++;
            if (j > 10)
                break;
        }

        switch (j)
        {
        case 7:
            k = 0;
            break;
        case 8:
            k = 1;
            break;
        case 9:
            k = 2;
            break;
        default:
            break;
        }
        sum += i + j + k;
    }
    return sum;
}

void AngleScriptTest::testComplex()
{
    QString code = R"(
    int testComplex(int num)
    {
        int i = 0;
        int j = 0;
        int k = 0;
        int sum = 0;
        for (i = 0; i < num; i++)
        {
            while (i < 5)
            {
                j++;
                if (j > 10)
                    break;
            }

            switch (j)
            {
            case 7:
                k = 0;
                break;
            case 8:
                k = 1;
                break;
            case 9:
                k = 2;
                break;
            default:
                break;
            }
            sum += i + j + k;
        }
        return sum;
    })";


    if (!buildAs(code))
        return;
    dump("as_testComplex");

    for (int i = 5; i < 15; i++)
    {
        QVariantList in, out;
        in << i;

        bool ret = call("testComplex", in, out);
        QVERIFY(ret);
        QCOMPARE(out[0].toInt(), test_complex_c(i));
    }
}

void AngleScriptTest::testFab()
{
    QString code = R"(
    int fab(int n) {
        if (n <= 1) {
            return n;
        }
        return fab(n - 1) + fab(n - 2);
    })";

    auto fab = [](int n)->int {
        const double phi = (1 + sqrt(5)) / 2;
        return static_cast<int>((pow(phi, n) - pow(-phi, -n)) / sqrt(5));
    };


    if (!buildAs(code))
        return;
    dump("as_testFab");

    for (int i = 5; i < 15; i++)
    {
        QVariantList in, out;
        in << i;

        bool ret = call("fab", in, out);
        QVERIFY(ret);
        QCOMPARE(out[0].toInt(), fab(i));
    }
}

void AngleScriptTest::testSwitch()
{
    QString code = R"(
    int testSwitch(int input) {
        switch (input) {
            case 0:
                return 0;
            case 1:
                return 1;
            case 2:
                return 2;
            case 3:
                return 3;
            case 4:
                return 4;
            case 5:
                return 5;
            default:
                return -1;
        }
    })";

    if (!buildAs(code))
        return;
    dump("as_testSwitch");

    auto testSwitch = [](int input)->int {
        switch (input) {
        case 0:
            return 0;
        case 1:
            return 1;
        case 2:
            return 2;
        case 3:
            return 3;
        case 4:
            return 4;
        case 5:
            return 5;
        default:
            return -1;
        }
    };


    for (int i = 0; i < 9; i++)
    {
        QVariantList in, out;
        in << i;

        bool ret = call("testSwitch", in, out);
        QVERIFY(ret);
        QCOMPARE(out[0].toInt(), testSwitch(i));
    }
}

void AngleScriptTest::testList()
{
    QString code = R"(
    int getList(QList<int> &input,int n ) {
        return input[n];
    }

    int getListSize(QList<int> &input) {
        return input.size();
    }
    
    void setList(QList<int> &input,int index,int n) {
        input[index] = n;
    }
    )";


    m_project.addGlobalVariable("list_int", "QList<int>", "{1,2,3,4,5,6,7,8}");

    if (!buildAs(code))
        return;
    dump("as_testList");

    auto env = m_project.environment();
    auto list_holder = m_engine.getVariable("list_int");
    auto list_ptr = env->convertTo(list_holder, env->nameToType("QList<int>*"));

    QList<int>* clist_ptr = JZObjectCast<QList<int>>(toJZObject(list_holder));

    QVariantList in, out;
    for (int i = 0; i < 8; i++)
    {
        in.clear();
        in << list_ptr << i;
        m_engine.call("getList", in, out);
        QCOMPARE(out[0].toInt(), i+1);
    }

    in.clear();
    in << list_ptr;
    m_engine.call("getListSize", in, out);
    QCOMPARE(out[0].toInt(), 8);

    in.clear();
    in << list_ptr << 0 << 100;
    m_engine.call("setList", in, out);
    QCOMPARE(clist_ptr->at(0), 100);
}

void AngleScriptTest::testSort()
{
    return;
    QString code = R"(
    QList<int> testSort(QList<int> input) {
        for(int i = 0; i < input.size(); i++)
        {
            for(int j = 0; j < input.size(); j++)
            {
                if(input[i] < input[j])
                {
                    int tmp = input[j];
                    input[j] = input[i];
                    input[i] = tmp;
                }
            }
        }
    })";

    if (!buildAs(code))
        return;
    dump("as_testSort");

    auto testSort = [](QList<int> input)->QList<int> {
        for (int i = 0; i < input.size(); i++)
        {
            for (int j = 0; j < input.size(); j++)
            {
                if (input[i] < input[j])
                {
                    int tmp = input[j];
                    input[j] = input[i];
                    input[i] = tmp;
                }
            }
        }
    };

    for (int i = 0; i < 9; i++)
    {
        QVariantList in, out;
        in << i;

        bool ret = call("testSort", in, out);
        QVERIFY(ret);
    }
}


void test_anglescript(int argc, char *argv[])
{
    AngleScriptTest test;
    QTest::qExec(&test,argc,argv);
}