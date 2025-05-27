#include <QApplication>
#include "mainwindow.h"
#include "JZNodeInit.h"

int main(int argc,char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    JZNodeInit();
        
    MainWindow w;
    w.showMaximized();
    return a.exec();
}