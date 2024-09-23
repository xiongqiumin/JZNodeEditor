#include <QCoreApplication>
#include "JZNodeInit.h"
#include "test_anglescript.h"
#include "test_benchmark.h"
#include "test_script.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    JZNodeInit();   
    
    test_script(argc, argv);
    test_anglescript(argc, argv);
    test_benchmark(argc, argv);    
    
    return 0;
}