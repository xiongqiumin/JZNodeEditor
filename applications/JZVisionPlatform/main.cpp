#include <QApplication>
#include "mainwindow.h"
#include "JZNodeInit.h"
#include "JZModuleVisionApp.h"
#include <fstream>
#include "modules/model/BackEnd/Onnx/JZModelEngineOnnx.h"

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