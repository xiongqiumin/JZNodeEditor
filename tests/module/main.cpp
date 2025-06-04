#include <QCoreApplication>
#include <QDebug>
#include "JZNodeInit.h"
#include "test_comm.h"
#include "test_camera.h"
#include "test_model.h"

int main(int argc,char *argv[])
{
    QCoreApplication a(argc,argv);
    JZNodeInit();
    
    test_model(argc, argv);

    return 0;
}