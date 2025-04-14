#include <QCoreApplication>
#include <QDebug>
#include "JZNodeInit.h"
#include "test_benchmark.h"
#include "test_script.h"
#include "test_opencv.h"
#include "test_anglescript.h"

int main(int argc,char *argv[])
{
    QCoreApplication a(argc,argv);
    JZNodeInit();

    //test_script(argc, argv);
    //test_benchmark(argc, argv);
    test_anglescript(argc, argv);

    return 0;
}