#include <QApplication>
#include <QEventLoop>
#include "mainwindow.h"
#include "JZNodeInit.h"
#include "JZNodeVM.h"
#include <QMessageBox>
#include <QCommandLineParser>
#include "JZNodeBind.h"
#include "sample/Russian.h"

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
    if(1)
    {
        SampleRussian russian;
        russian.saveProject();
        russian.run();
        return 1;
    }
    if(0)
    {         
        QString project_name = "project8898";

        QString project_path = "C:/Users/xiong/Desktop/JZNodeEditor/x64/Debug/project/" + project_name
            + "/" + project_name + ".jzproject";
        QString program_path = "C:/Users/xiong/Desktop/JZNodeEditor/x64/Debug/project/" + project_name 
            +  "/build/" +  project_name + ".program";
        
        JZProject project;
        if (!project.open(project_path))
        {
            qDebug() << "load project failed";
            return false;
        }

        JZNodeBuilder builder;
        JZNodeProgram program;
        if (!builder.build(&project, &program))
        {
            qDebug().noquote() << builder.error();
            return false;
        }
        if(!program.save(program_path))
        {
            qDebug() << "save failed";
            return false;
        }       

        JZNodeVM vm;
        QString error;
        if(!vm.init(program_path,false, error))
        {
            QMessageBox::information(nullptr,"","init program \"" + program_path + "\" failed\n" + error);
            return 1;
        }
        return a.exec();
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
