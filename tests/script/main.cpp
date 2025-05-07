#include <QCoreApplication>
#include <QDebug>
#include "JZNodeInit.h"
#include "test_benchmark.h"
#include "test_script.h"
#include "test_opencv.h"
#include "test_anglescript.h"
#include "test_debug.h"
#include "test_tx.h"
#include "test_unitTest.h"

int main(int argc,char *argv[])
{
    QCoreApplication a(argc,argv);
    JZNodeInit();

    test_script(argc, argv);
    //test_benchmark(argc, argv);
    //test_anglescript(argc, argv);
    //test_debug(argc, argv);
    //test_tx(argc, argv);
    //test_unitTest(argc, argv);

    return 0;
}