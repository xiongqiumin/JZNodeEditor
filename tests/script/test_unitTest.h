#ifndef TEST_UINT_TEST_H_
#define TEST_UINT_TEST_H_

#include <QObject>
#include "JZProject.h"
#include "JZNodeEngine.h"
#include "test_base.h"

class TestUnitTest : public BaseTest
{
    Q_OBJECT

public:
    TestUnitTest();

private slots:
    void testHello();

protected:
    bool buidUnitTest(JZScriptItem *unit_script_item);
};

void test_unitTest(int argc, char *argv[]);

#endif
