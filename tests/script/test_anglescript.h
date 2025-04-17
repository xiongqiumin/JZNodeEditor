#ifndef TEST_ANGLE_SCRIPT_H_
#define TEST_ANGLE_SCRIPT_H_

#include <QObject>
#include "JZProject.h"
#include "JZNodeEngine.h"
#include "test_base.h"

class AngleScriptTest : public BaseTest
{
    Q_OBJECT

public:
    AngleScriptTest();

private slots:
    void testHello();
    void testIf();
    void testFor();
    void testWhile();
    void testSwitch();
    
protected:

};

void test_anglescript(int argc, char *argv[]);

#endif
