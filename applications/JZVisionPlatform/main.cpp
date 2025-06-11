#include <QApplication>
#include <QTreeWidget>
#include "JZNodeInit.h"
#include "JZModuleVisionApp.h"
#include "mainwindow.h"

int main(int argc,char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);

    Q_INIT_RESOURCE(JZNodeEditor);
    Q_INIT_RESOURCE(vision);       

    JZNodeInit();
    JZModuleManager::instance()->addModule(new JZModuleVisionApp());

    MainWindow w;
    w.showMaximized();
    return a.exec();
}