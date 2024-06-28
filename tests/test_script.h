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
    void testContainer();

    void testClass();
    void testCClass();

    void testRegExp();
    void testObjectParse();
    
    void testBind();
    void testParamBinding();
    
    void testWhileLoop();
    void testFor();
    void testForEach();

    void testBranch();
    void testIf();
    void testSwitch();
    void testSequeue();
    
    void testExpr();
    void testCustomExpr();
    void testFunction();    
    void testBreakPoint();
    void testDebugServer();

protected:
    bool initWhileCase(QList<int> &nodeId,QList<int> &value);
};






#endif
