#include <QApplication>
#include <QEventLoop>
#include "mainwindow.h"
#include "JZNodeInit.h"
#include "JZNodeVM.h"
#include <QMessageBox>
#include <QCommandLineParser>
#include "JZNodeBind.h"
#include "sample/Russian/Russian.h"
#include "sample/ImageBatch/ImageBatch.h"

extern void test_script();

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    JZNodeInit();           

    if(0)
    {
        test_script();
        return 0;
    }
    if(0)
    {
        SampleRussian sample;        
        //SampleImageBatch sample;        
        sample.saveProject();
        sample.run();
        return 0;
    }    

    if(argc == 1)
    {
        MainWindow w;
        w.showMaximized();
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

        QString error;
        QString program_path = parser.value(runOption);
        bool debug = parser.isSet(debugOption);
        JZNodeVM vm;        
        if(!vm.init(program_path,debug, error))
        {
            QMessageBox::information(nullptr,"","init program \"" + program_path + "\" failed.\n" + error);
            return 1;
        }

        return a.exec();            
    }            
}
