#include <QTest>
#include "test_ui.h"
#include "JZNodeEditor.h"

UiTest::UiTest()
{
}

void UiTest::testNodeEditor()
{
}

void test_ui(int argc, char *argv[])
{    
    UiTest s; 
    QTest::qExec(&s,argc,argv);
}
