#include <QDir>
#include "JZProjectTemplate.h"
#include "JZProject.h"
#include "JZScriptFile.h"
#include "JZNodeEvent.h"
#include "JZNodeValue.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeFunction.h"
#include "JZUiItem.h"
#include "JZNodeUtils.h"
#include "JZNodeFlow.h"
#include "modules/communication/JZModuleComm.h"
#include "modules/camera/JZModuleCamera.h"
#include "modules/opencv/JZModuleOpencv.h"

JZProjectTemplate *JZProjectTemplate::instance()
{
    static JZProjectTemplate inst;
    return &inst;
}        

JZProjectTemplate::JZProjectTemplate()
{
}

QStringList JZProjectTemplate::templateList()
{
    QStringList list;
    list << "console" << "ui" << "vision";
    return list;
}

void JZProjectTemplate::createMainWindow()
{
    auto* main_flow = m_project->mainFunction();
    auto project = m_project;
    project->addGlobalVariable("__mainwindow__", "MainWindow");

    auto func_inst = project->environment()->functionManager();
    auto window_file = new JZScriptFile();
    window_file->setName("MainWindow.jz");

    auto ui_file = new JZUiItem();
    ui_file->setName("ui");

    project->addItem("./", window_file);

    auto class_item = window_file->addClass("MainWindow", "QWidget");
    class_item->addUi(ui_file);

    JZFunctionDefine define;
    define.className = "MainWindow";
    define.name = "init";
    define.isFlowFunction = true;
    define.paramIn.push_back(JZParamDefine("this", "MainWindow*"));
    class_item->addMemberFunction(define);
    project->onItemChanged(class_item);

    JZNodeParam* get_param = new JZNodeParam();
    JZNodeFunction* func_init = new JZNodeFunction();
    JZNodeFunction* func_show = new JZNodeFunction();
    JZNodeMainLoop* main_loop = new JZNodeMainLoop();

    main_flow->addNode(get_param);
    main_flow->addNode(func_init);
    main_flow->addNode(func_show);
    main_flow->addNode(main_loop);

    get_param->setVariable("__mainwindow__");
    func_init->setFunction(&define);
    func_show->setFunction(func_inst->function("QWidget::show"));

    JZNode* start = main_flow->getNode(0);
    main_flow->addConnect(start->flowOutGemo(), func_init->flowInGemo());
    main_flow->addConnect(get_param->paramOutGemo(0), func_init->paramInGemo(0));

    main_flow->addConnect(func_init->flowOutGemo(0), func_show->flowInGemo());
    main_flow->addConnect(get_param->paramOutGemo(0), func_show->paramInGemo(0));

    main_flow->addConnect(func_show->flowOutGemo(0), main_loop->flowInGemo());
    main_flow->addConnect(get_param->paramOutGemo(0), main_loop->paramInGemo(0));
}

bool JZProjectTemplate::initProject(JZProject *project, QString temp)
{
    project->clear();
    m_project = project;

    JZScriptFile *main_file = new JZScriptFile();
    main_file->setName("main.jz");
    project->addItem("./", main_file);
    
    JZFunctionDefine main_def;
    main_def.name = "main";
    auto *main_flow = main_file->addFunction(main_def);
    auto *global_def = main_file->addParamDefine("global");

    if (temp == "console")
    {

    }
    else if (temp == "ui")
    {  
        createMainWindow();
    }
    else if (temp == "vision")
    {
        createMainWindow();

        auto class_item = m_project->getClass("MainWindow");
        class_item->addMemberVariable("cameraManager", "JZCameraManager");
        class_item->addMemberVariable("commManager", "JZCommManager");
        class_item->addMemberVariable("modelManager", "JZModelManager");
        
        auto func_init = class_item->memberFunction("init");
        auto start = func_init->startNode();

        JZNodeCameraInit* camera_init = new JZNodeCameraInit();
        JZNodeModelInit* model_init = new JZNodeModelInit();
        JZNodeCommInit* comm_init = new JZNodeCommInit();
        func_init->addNode(camera_init);
        func_init->addNode(model_init);
        func_init->addNode(comm_init);
        func_init->addConnect(start->flowOutGemo(), camera_init->flowInGemo());
        func_init->addConnect(camera_init->flowOutGemo(), model_init->flowInGemo());
        func_init->addConnect(model_init->flowOutGemo(), comm_init->flowInGemo());

        class_item->addFlow("flow");
    }
    else
    {
        Q_ASSERT(0);
    }

    return true;
}