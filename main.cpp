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
    if(0)
    {
        QString program_path = "C:/work/xiong/build-JZNodeEditor-Desktop_Qt_5_15_2_MinGW_64_bit-Debug/debug/build/untitled.program";

        bool build_project = true;
        if(build_project)
        {
            JZProject project;
            project.initUi();

            auto script = (JZScriptFile*)project.getItem(project.mainScript());

            auto meta = JZNodeObjectManager::instance()->meta("AbstractButton");
            JZNodeSingleEvent *event = new JZNodeSingleEvent();
            event->setSingle(meta->className,meta->single("clicked"));
            event->setVariable("mainwindow.btn");

            script->addNode(JZNodePtr(event));

            JZNodeBuilder builder;
            JZNodeProgram program;
            if(!builder.build(&project,&program))
            {
                qDebug() << builder.error();
                return 1;
            }
            qDebug().noquote() << program.dump();
            if(!program.save(program_path))
            {
                qDebug() << "save failed";
                return false;
            }
        }

        JZNodeVM vm;
        if(!vm.init(program_path,false))
        {
            QMessageBox::information(nullptr,"","init program \"" + program_path + "\" failed");
            return 1;
        }
        return a.exec();
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
