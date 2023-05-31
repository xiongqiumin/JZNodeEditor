#include <QApplication>
#include <QEventLoop>
#include "mainwindow.h"
#include "JZNodeInit.h"
#include "JZNodeVM.h"
#include <QMessageBox>
#include <QCommandLineParser>

extern void testBuild();
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    JZNodeInit();    

    if(0)
    {
        testBuild();
        return 0;
    }

    if(argc == 1)
    {
        MainWindow w;
        w.show();
        return a.exec();
    }
    else
    {  
        QCommandLineParser parser;

        //run
        QCommandLineOption runOption("run","","file");
        parser.addOption(runOption);

        //debug
        QCommandLineOption debugOption("debug");
        parser.addOption(debugOption);

        //start-pause
        QCommandLineOption pauseOption("start-pause");
        parser.addOption(pauseOption);
        parser.process(a);        

        QString program_path = parser.value(runOption);
        bool debug = parser.isSet(debugOption);
        JZNodeVM vm;
        if(!vm.init(program_path,debug))
        {
            QMessageBox::information(nullptr,"","init program \"" + program_path + "\" failed");
            return 1;
        }

        return a.exec();            
    }            
}
