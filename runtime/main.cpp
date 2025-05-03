#include <QApplication>
#include <QEventLoop>
#include <QMessageBox>
#include <QCommandLineParser>
#include <QFileInfo>
#include "JZNodeBind.h"
#include "JZRegExpHelp.h"
#include "JZNodeInit.h"
#include "JZNodeVM.h"

using namespace std;

int runProgram(QString program, bool debug)
{    
    QString error;
    JZNodeVM vm;
    if (!vm.init(program, debug, error))
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

    QString program = R"(C:\Users\xiong\Desktop\JZNodeEditor\build\Debug\sample\VisionDemo\build\VisionDemo.program)";
    return runProgram(program,false);
}