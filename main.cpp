#include <QApplication>
#include <QEventLoop>
#include <QMessageBox>
#include <QCommandLineParser>
#include <QFileInfo>
#include "JZNodeBind.h"
#include "JZNodeCppGenerater.h"
#include "JZRegExpHelp.h"
#include "mainwindow.h"
#include "JZNodeInit.h"
#include "JZNodeVM.h"

using namespace std;

int runProgram(QString program_path,bool debug)
{
    QString error;
    JZNodeVM vm;
    if (!vm.init(program_path, debug, error))
    {
        QMessageBox::information(nullptr, "", "init program failed.\n" + error);
        return 1;
    }
    return qApp->exec();
}

int main(int argc,char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    JZNodeInit();               

    return runProgram(R"(C:\Users\xiong\Desktop\JZNodeEditor\build\Debug\project\Project17\build\Project17.program)", false);    

    QCommandLineParser parser;

    //run
    QCommandLineOption runOption("run", "", "file");
    parser.addOption(runOption);

    //debug
    QCommandLineOption debugOption("debug");
    parser.addOption(debugOption);
    parser.process(a);

    bool debug = parser.isSet(debugOption);
    QFileInfo app_info(QString::fromLocal8Bit(argv[0]));
    if (app_info.fileName() != "JZNodeEditor.exe")
    {
        QString program = app_info.baseName() + ".prog";
        return runProgram(program, debug);        
    }       

    if(parser.isSet(runOption))
    {   
        QString error;
        QString program_path = parser.value(runOption);        
        JZNodeVM vm;
        if (!vm.init(program_path, debug, error))
        {
            QMessageBox::information(nullptr, "", "init program \"" + program_path + "\" failed.\n" + error);
            return 1;
        }
        return a.exec();        
    }
    else
    {                  
        MainWindow w;
        w.showMaximized();
        return a.exec();                  
    }
}