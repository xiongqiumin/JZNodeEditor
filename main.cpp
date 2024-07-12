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
#include "sample/SmartHome/SmartHome.h"

void run_testcase(int argc, char *argv[])
{
    extern void test_script(int argc, char *argv[]);
    extern void test_anglescript(int argc, char *argv[]);
    extern void test_benchmark(int argc, char *argv[]);

    test_script(argc,argv);
    test_anglescript(argc,argv);
    test_benchmark(argc,argv);
}   

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    JZNodeInit();           

    if(0)
    {
        run_testcase(argc,argv);
        return 0;
    }    
    if(0 && argc == 1)
    {
        SampleRussian sample;        
        //SampleImageBatch sample;
        //SampleSmartHome sample;
        sample.saveProject();
        //sample.loadProject();
        sample.run();
        return 1;
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
