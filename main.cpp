#include <QApplication>
#include <QEventLoop>
#include "mainwindow.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeCompiler.h"
#include "JZNodeEngine.h"
#include "JZNodeFactory.h"

extern void testBuild();
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    JZNodeFactory::instance()->init();
    JZNodeFunctionManager::instance()->init();    

    testBuild();
    return 1;

    MainWindow w;
    w.show();
    return a.exec();
}
