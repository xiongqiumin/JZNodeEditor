#include <QEventLoop>
#include <QDebug>
#include <math.h>
#include <QApplication>
#include <QTest>
#include "test_script_help.h"
#include "JZScriptItemHelp.h"

ScriptHelpTest::ScriptHelpTest()
{
}

void ScriptHelpTest::testNop()
{
    
}

void ScriptHelpTest::testOperator()
{

}


void test_script_help(int argc, char *argv[])
{    
    ScriptHelpTest s; 
    QTest::qExec(&s,argc,argv);
}
