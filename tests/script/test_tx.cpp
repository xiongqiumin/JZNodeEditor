#include <QEventLoop>
#include <QDebug>
#include <math.h>
#include <QApplication>
#include <QTest>
#include "JZNodeFactory.h"
#include "test_tx.h"

TxTest::TxTest()
{
}

void TxTest::testLoop()
{
    QString code = R"(int testWhile(int n) {
        int i = 0;
        while(i < n) {
            i = i + 1;
        }
        return 0;
    })";

    if(!buildAs(code))
        return;
    dump("debug_testWhile");

    QVariantList in,out;
    in << 1000000;
    call("testWhile", in,out);
}


void test_tx(int argc, char *argv[])
{    
    TxTest s; 
    QTest::qExec(&s,argc,argv);
}
