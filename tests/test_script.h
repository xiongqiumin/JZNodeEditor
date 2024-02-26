#ifndef TEST_SCRIPT_H_
#define TEST_SCRIPT_H_

#include <QObject>
#include "JZProject.h"
#include "JZNodeCompiler.h"
#include "JZNodeEngine.h"
#include "JZNodeValue.h"
#include "JZNodeProgram.h"
#include "JZNodeBuilder.h"
#include "JZNodeDebugServer.h"
#include "JZNodeDebugClient.h"
#include "JZNodeFunction.h"
#include "JZNodeBind.h"
#include "JZNodeFactory.h"
#include "JZNodeObject.h"
#include "JZParamItem.h"
#include "JZScriptFile.h"

class ScriptTest : public QObject
{
    Q_OBJECT

public:
    ScriptTest();

protected slots:
    void onRuntimeError(JZNodeRuntimeError error);

private slots:
    void initTestCase();
    void init();
    void cleanup();

    void testMatchType();

    void testClass();
    void testCClass();

    void testRegExp();
    void testObjectParse();

    void testProjectSave();
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
    bool build();
    void call();
    bool run(bool async);
    void stop();
    void printCode();
    QMap<int,int> initWhileSetCase(); //返回第几行应该是什么值

    JZProject m_project;
    JZScriptFile *m_file;
    JZScriptItem *m_scriptFlow;
    JZParamItem *m_paramDef;

    JZNodeProgram m_program;
    JZNodeEngine m_engine;
    std::thread m_asyncThread;
    QString m_testPath;
    QString m_error;
    bool m_dump;
};






#endif
