#ifndef TEST_SCRIPT_H_
#define TEST_SCRIPT_H_

#include <QObject>
#include "test_base.h"

class ScriptTest : public BaseTest
{
    Q_OBJECT

public:
    ScriptTest();

private slots:    
    void testMatchType();
    void testClone();
    void testContainer();

    void testClass();
    void testCClass();

    void testRegExp();
    void testObjectParse();
    
    void testBind();
    
    void testAuto();
    void testFor();
    void testWhileLoop();
    void testForEach();
    void testBreakContinue();    
    
    void testIf();
    void testSwitch();
    void testSequeue();
    
    void testExpr();
    void testCustomExpr();
    void testFunction();        
    void testArgs();
    void testTryCatch();

protected:    
};

void test_script(int argc, char *argv[]);

#endif
