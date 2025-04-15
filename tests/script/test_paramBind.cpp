#include <QTest>
#include <math.h>
#include <functional>
#include "test_paramBind.h"

//ParamBindTest
ParamBindTest::ParamBindTest()
{

}

void ParamBindTest::testSample()
{
    
}

void test_paramBind(int argc, char* argv[])
{
    ParamBindTest test;
    QTest::qExec(&test, argc, argv);
}