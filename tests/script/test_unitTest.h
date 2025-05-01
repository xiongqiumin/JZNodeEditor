#ifndef TEST_UINT_TEST_H_
#define TEST_UINT_TEST_H_

#include <QObject>
#include "JZProject.h"
#include "JZNodeEngine.h"
#include "test_base.h"
#include "JZScriptUnitTest.h"

class TestUnitTest : public BaseTest
{
    Q_OBJECT

public:
    TestUnitTest();

private slots:
    void testHello();
    void testTimerEvent();
    void testCameraEvent();

protected:
    void dumpUnit(JZScriptUnitTest *unit,QString name);

};

void test_unitTest(int argc, char *argv[]);

#endif
