#include <QApplication>
#include "mainwindow.h"
#include "JZNodeInit.h"

int main(int argc,char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);

    Q_INIT_RESOURCE(JZNodeEditor);
    Q_INIT_RESOURCE(vision);

    JZNodeInit();
        
    MainWindow w;
    w.showMaximized();
    return a.exec();
}