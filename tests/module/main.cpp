#include <QCoreApplication>
#include <QDebug>
#include "JZNodeInit.h"
#include "test_comm.h"
#include "test_camera.h"

int main(int argc,char *argv[])
{
    QCoreApplication a(argc,argv);
    JZNodeInit();
    
    test_camera(argc, argv);

    return 0;
}