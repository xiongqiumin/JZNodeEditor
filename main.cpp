#include <QApplication>
#include <QEventLoop>
#include "mainwindow.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeCompiler.h"
#include "JZNodeEngine.h"
#include "JZNodeFactory.h"
#include <QMessageBox>

extern void testBuild();
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    JZNodeFactory::instance()->init();
    JZNodeFunctionManager::instance()->init();    

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
        if(vm.load(argv[1]))
        {
            QMessageBox::information(this,"","应用启动失败");
            return 1;
        }
        return vm.exec();            
    }            
}
