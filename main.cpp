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
#include "JZNodeCppGenerater.h"
#include "3rd/jzupdate/JZUpdateClient.h"

void run_testcase(int argc, char *argv[])
{
    extern void test_script(int argc, char *argv[]);
    extern void test_anglescript(int argc, char *argv[]);
    extern void test_benchmark(int argc, char *argv[]);

    test_script(argc, argv);
    test_anglescript(argc, argv);
    test_benchmark(argc, argv);
}

QtMessageHandler g_defaultMessageHandle = nullptr;
void outputLogMessage(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    g_defaultMessageHandle(type, context, msg);
}

int main(int argc, char *argv[])
{
    g_defaultMessageHandle = qInstallMessageHandler(outputLogMessage);

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

        //JZNodeCppGenerater cpp;
        //cpp.generate(sample.project(),"cpp");
        return 1;
    }    

    QCommandLineParser parser;

    //run
    QCommandLineOption runOption("run", "", "file");
    parser.addOption(runOption);    

    //debug
    QCommandLineOption debugOption("debug");
    parser.addOption(debugOption);

    parser.process(a);

    JZUpdateClient client(qApp->applicationDirPath());
    if (client.isDownloadFinish())
    {        
        if (!client.dealUpdate())
            return false;

        QString exe_path = qApp->applicationFilePath();
        QProcess::startDetached(exe_path);
        return 0;
    }
    client.clearCache();     

    if(parser.isSet(runOption))
    {   
        QString error;
        QString program_path = parser.value(runOption);
        bool debug = parser.isSet(debugOption);
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
