#include <QCoreApplication>
#include <QDebug>
#include "JZNodeInit.h"
#include "JZProject.h"
#include "JZNodeEngine.h"
#include "JZNodeBuilder.h"
#include "JZProjectTemplate.h"
#include "JZNodeFunction.h"
#include "JZNodeProgramDumper.h"
#include "JZNodeValue.h"

int main(int argc,char *argv[])
{
    QCoreApplication a(argc,argv);
    JZNodeInit();

    JZProject project;    
    JZProjectTemplate::instance()->initProject(&project, "console");
    
    project.addGlobalVariable("camera","JZCameraFile");

    auto env = project.environment();
    auto meta = env->meta("JZCameraFile");

    JZFunctionDefine func = JZSlotFunctionDefine("camera", meta->signal("sigFrameReady"));
    JZScriptFile *main_file = project.mainFile();
    JZScriptItem *slot = main_file->addFunction(func);
    auto slot_start = slot->startNode();

    JZNodePrint* node_print = new JZNodePrint();
    node_print->setParamInValue(0, "hello");
    slot->addNode(node_print);
    slot->addConnect(slot_start->flowOutGemo(), node_print->flowInGemo());

    //main
    JZScriptItem *main = project.mainFunction();
    auto start = main->startNode();

    JZNodeFunction* function_open = new JZNodeFunction();
    JZNodeFunction *function_start = new JZNodeFunction();
    main->addNode(function_open);
    main->addNode(function_start);

    function_open->setFunction(meta->function("open"));
    function_open->setVariable("camera");
    function_open->setParamInValue(1, "D:/work/qt/JZNodeEditor/Resources/icons");

    function_start->setFunction(meta->function("start"));
    function_start->setVariable("camera");

    main->addConnect(start->flowOutGemo(), function_open->flowInGemo());
    main->addConnect(function_open->flowOutGemo(), function_start->flowInGemo());

    JZNodeProgram program;
    JZNodeBuilder builder;
    builder.setProject(&project);
    if (!builder.build(&program))
    {
        qDebug() << "build failed:" << builder.error();
        return false;
    }
    JZNodeProgramDumper dumper;
    qDebug().noquote() << dumper.dump(&program);

    JZNodeEngine engine;
    engine.setProgram(&program);
    engine.init();

    QVariantList in, out;
    engine.call("main", in, out);
    return a.exec();
}