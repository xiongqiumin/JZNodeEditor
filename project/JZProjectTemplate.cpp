#include <QDir>
#include "JZProjectTemplate.h"
#include "JZProject.h"
#include "JZScriptFile.h"
#include "JZNodeEvent.h"
#include "JZNodeValue.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeFunction.h"
#include "JZUiFile.h"
#include "JZNodeUtils.h"

JZProjectTemplate *JZProjectTemplate::instance()
{
    static JZProjectTemplate inst;
    return &inst;
}        

bool JZProjectTemplate::initProject(JZProject *project, QString temp)
{
    JZScriptFile *main_file = new JZScriptFile();
    main_file->setName("main.jz");
    project->addItem("./", main_file);
    
    JZFunctionDefine main_def;
    main_def.name = "main";
    auto *main_flow = main_file->addFunction(main_def);
    auto *global_def = main_file->addParamDefine("global");

    if (temp == "ui")
    {  
        auto func_inst = project->environment()->functionManager();
        auto window_file = new JZScriptFile();
        window_file->setName("MainWindow.jz");

        auto ui_file = new JZUiFile();
        ui_file->setName("mainwindow.ui");        

        project->addItem("./",window_file);
        project->addItem("./",ui_file);

        auto class_item = window_file->addClass("MainWindow","QWidget");        
        class_item->setUiFile("./mainwindow.ui");
        
        JZFunctionDefine define;
        define.className = "MainWindow";
        define.name = "init";        
        define.isFlowFunction = true;
        define.paramIn.push_back(JZParamDefine("this", "MainWindow"));
        class_item->addMemberFunction(define);
        project->onItemChanged(class_item);

        main_flow->addLocalVariable("mainwindow", class_item->classType());

        JZNodeParam *get_param = new JZNodeParam();
        JZNodeSetParam *set_param = new JZNodeSetParam();
        JZNodeCreate *create = new JZNodeCreate();
        JZNodeFunction *func_init = new JZNodeFunction();
        JZNodeFunction *func_show = new JZNodeFunction();
        JZNodeMainLoop *main_loop = new JZNodeMainLoop();

        main_flow->addNode(create);
        main_flow->addNode(set_param);
        main_flow->addNode(get_param);
        main_flow->addNode(func_init);
        main_flow->addNode(func_show);
        main_flow->addNode(main_loop);

        get_param->setVariable("mainwindow");
        set_param->setVariable("mainwindow");
        create->setClassName("MainWindow");
        func_init->setFunction(&define);
        func_show->setFunction(func_inst->function("QWidget.show"));        

        JZNode *start = main_flow->getNode(0);
        main_flow->addConnect(start->flowOutGemo(), set_param->flowInGemo());
        main_flow->addConnect(create->paramOutGemo(0), set_param->paramInGemo(1));

        main_flow->addConnect(set_param->flowOutGemo(0), func_init->flowInGemo());
        main_flow->addConnect(get_param->paramOutGemo(0), func_init->paramInGemo(0));

        main_flow->addConnect(func_init->flowOutGemo(0), func_show->flowInGemo());
        main_flow->addConnect(get_param->paramOutGemo(0), func_show->paramInGemo(0));

        main_flow->addConnect(func_show->flowOutGemo(0), main_loop->flowInGemo());
        main_flow->addConnect(get_param->paramOutGemo(0), main_loop->paramInGemo(0));
        
    }
    JZNodeUtils::projectUpdateLayout(project);
    project->save();
    project->saveAllItem();
    return true;
}