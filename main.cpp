#include <QApplication>
#include <QEventLoop>
#include <QMessageBox>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QDir>
#include "JZNodeBind.h"
#include "JZRegExpHelp.h"
#include "JZNodeInit.h"
#include "JZNodeVM.h"
#include "mainwindow.h"
#include "JZNodeProgramDumper.h"
#include "sample/VisionDemo/VisionDemo.h"

using namespace std;

int runProgram(QString name,bool debug)
{
    QString project_dir = qApp->applicationDirPath() + "/project/" + name;

    QString project_path = project_dir + "/" + name + ".jzproj";

    JZProject project;
    if (!project.open(project_path))
    {
        qDebug() << "load project failed";
        return 1;
    }

    QString program_path = project.path() + "/build/" + name + ".program";
    QString dump_path = project.path() + "/build/dump";
    if (!QFile::exists(project.path() + "/build"))
        QDir().mkdir(project.path() + "/build");
    if (!QFile::exists(dump_path))
        QDir().mkdir(dump_path);

    JZNodeBuilder builder;
    builder.setProject(&project);

    JZNodeProgram program;
    if (!builder.build(&program))
    {
        qDebug().noquote() << builder.error();
        return false;
    }

    //save asm    
    JZNodeProgramDumper dumper;
    dumper.init(&project, &program);
    dumper.dump(dump_path);

    //save bin
    if (!program.save(program_path))
    {
        qDebug() << "save failed";
        return false;
    }

    JZNodeVM vm;
    QString error;
    if (!vm.init(program_path, false, error))
    {
        QMessageBox::information(nullptr, "", "init program \"" + program_path + "\" failed\n" + error);
        return 1;
    }
    return qApp->exec();
}

void createSample()
{
    SampleVisionDemo demo;
    demo.initCameraFile();
    //demo.initCameraHik();
    demo.saveProject();
}

int runSample()
{
    SampleVisionDemo demo;
    demo.initCameraFile();
    return demo.run();    
}

int main(int argc,char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    JZNodeInit();               

    //createSample();
    return runSample();
    //return runProgram("Project20", false);    

    QCommandLineParser parser;

    //run
    QCommandLineOption runOption("run", "", "file");
    parser.addOption(runOption);

    //debug
    QCommandLineOption debugOption("debug");
    parser.addOption(debugOption);
    parser.process(a);

    bool debug = parser.isSet(debugOption);
    if (parser.isSet(runOption))
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