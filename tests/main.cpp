#include <QApplication>
#include "JZNodeInit.h"
#include "test_benchmark.h"
#include "test_script.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    JZNodeInit();   
    
    test_script(argc, argv);    
    test_benchmark(argc, argv);        

    return 0;
}