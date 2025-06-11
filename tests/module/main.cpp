#include <QCoreApplication>
#include <QDebug>
#include "JZNodeInit.h"
#include "test_comm.h"
#include "test_camera.h"
#include "test_model.h"
#include "test_vision.h"

int main(int argc,char *argv[])
{
    QCoreApplication a(argc,argv);
    JZNodeInit();
    
    test_vision(argc, argv);

    return 0;
}