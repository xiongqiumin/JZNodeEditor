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
    JZNode *start = main_flow->getNode(0);

    if (temp == "ui")
    {  
        auto func_inst = JZNodeFunctionManager::instance();
        auto window_file = new JZScriptFile();
        window_file->setName("MainWindow.jz");

        auto ui_file = new JZUiFile();
        ui_file->setName("mainwindow.ui");        

        project->addItem("./",window_file);
        project->addItem("./",ui_file);

        auto class_define = window_file->addClass("MainWindow","Widget");        
        class_define->setUiFile("./mainwindow.ui");
        
        JZFunctionDefine define;
        define.name = "init";
        define.isFlowFunction = true;
        define.paramIn.push_back(JZParamDefine("this", "MainWindow"));
        class_define->addMemberFunction(define);

        global_def->addVariable("MainWindow", "MainWindow");

        JZNodeParam *get_param = new JZNodeParam();
        JZNodeSetParam *set_param = new JZNodeSetParam();
        JZNodeCreate *create = new JZNodeCreate();
        JZNodeFunction *func_init = new JZNodeFunction();
        JZNodeFunction *func_show = new JZNodeFunction();
        int node_start = main_flow->nodeList()[0];
        int node_create = main_flow->addNode(create);
        int node_set = main_flow->addNode(set_param);
        int node_get = main_flow->addNode(get_param);
        int node_init = main_flow->addNode(func_init);
        int node_show = main_flow->addNode(func_show);

        get_param->setVariable("MainWindow");
        set_param->setVariable("MainWindow");
        create->setClassName("MainWindow");
        func_init->setFunction(func_inst->function("MainWindow.init"));
        func_show->setFunction(func_inst->function("Widget.show"));

        JZNode *start = main_flow->getNode(0);
        main_flow->addConnect(start->flowOutGemo(), set_param->flowInGemo());
        main_flow->addConnect(create->paramOutGemo(0), set_param->paramInGemo(1));

        main_flow->addConnect(set_param->flowOutGemo(0), func_init->flowInGemo());
        main_flow->addConnect(get_param->paramOutGemo(0), func_init->paramInGemo(0));

        main_flow->addConnect(func_init->flowOutGemo(0), func_show->flowInGemo());
        main_flow->addConnect(get_param->paramOutGemo(0), func_show->paramInGemo(0));
        
    }
    projectUpdateLayout(project);
    project->save();
    project->saveAllItem();
    return true;
}