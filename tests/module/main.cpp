#include <QCoreApplication>
#include <QDebug>
#include "JZNodeInit.h"
#include "test_modbus.h"

int main(int argc,char *argv[])
{
    QCoreApplication a(argc,argv);
    JZNodeInit();
    
    test_modbus(argc, argv);

    return 0;
}