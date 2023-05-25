#include <QApplication>
#include <QEventLoop>
#include "mainwindow.h"
#include "JZNodeInit.h"
#include "JZNodeVM.h"
#include <QMessageBox>

extern void testBuild();
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    JZNodeInit();    

    argc = 2;
    if(argc == 1)
    {
        MainWindow w;
        w.show();
        return a.exec();
    }
    else if(argc == 2)
    {
        testBuild();
        return 1;        
    }
    else
    {
        JZNodeVM vm;
        if(!vm.init(argv[1]))
            return 1;                    
        return a.exec();            
    }            
}
