#include <QApplication>
#include <QEventLoop>
#include <QMessageBox>
#include <QCommandLineParser>
#include <QFileInfo>
#include "JZNodeBind.h"
#include "JZRegExpHelp.h"
#include "JZNodeInit.h"
#include "JZNodeVM.h"
#include "mainwindow.h"

using namespace std;

int runProgram(QString name, bool debug)
{
    QString path = qApp->applicationDirPath() + "/project/" + name + "/build/" + name + ".program";

    QString error;
    JZNodeVM vm;
    if (!vm.init(path, debug, error))
    {
        QMessageBox::information(nullptr, "", "init program failed.\n" + error);
        return 1;
    }
    return qApp->exec();
}

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    JZNodeInit();

    return runProgram("Project3", true);

    return qApp->exec();
}