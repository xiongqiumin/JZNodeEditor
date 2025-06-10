#include <QApplication>
#include <QEventLoop>
#include <QMessageBox>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QTreeWidget>
#include "JZNodeInit.h"

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);

    JZNodeInit();
    
    QWidget w;
    w.show();
    return a.exec();
}