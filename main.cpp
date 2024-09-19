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
#include "editor/tools/JZModbusSimulator.h"
#include "JZRegExpHelp.h"

using namespace std;

void run_testcase(int argc, char *argv[])
{
    /*
    extern void test_script(int argc, char *argv[]);
    extern void test_anglescript(int argc, char *argv[]);
    extern void test_benchmark(int argc, char *argv[]);

    test_script(argc, argv);
    test_anglescript(argc, argv);
    test_benchmark(argc, argv);
    */
}

void runProgram(QString program_path)
{
    QString error;
    JZNodeVM vm;
    if (!vm.init(program_path, false, error))
    {
        QMessageBox::information(nullptr, "", "init program \"" + program_path + "\" failed.\n" + error);
        return;
    }
    qApp->exec();
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
    
    runProgram(R"(C:\Users\xiong\Desktop\JZNodeEditor\build\Debug\project\project\build\project.program)");

    JZUpdateClient client(qApp->applicationDirPath());
    if (client.isDownloadFinish())
    {
        if (!client.dealUpdate())
        {
            QMessageBox::information(nullptr, "", "自动更新失败，请尝试重新下载");
            return false;
        }

        QString exe_path = qApp->applicationFilePath();
        QProcess::startDetached(exe_path);
        return 0;
    }
    client.clearCache();

    QCommandLineParser parser;

    //run
    QCommandLineOption runOption("run", "", "file");
    parser.addOption(runOption);    

    //debug
    QCommandLineOption debugOption("debug");
    parser.addOption(debugOption);

    parser.process(a);

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
