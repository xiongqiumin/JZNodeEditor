#ifndef TEST_SCRIPT_H_
#define TEST_SCRIPT_H_

#include <QObject>
#include "test_base.h"

class ScriptHelpTest : public BaseTest
{
    Q_OBJECT

public:
    ScriptHelpTest();

private slots:
    void testNop();
    void testOperator();

protected:
    
};

void test_script_help(int argc, char *argv[]);

#endif
