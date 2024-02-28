#include <QDir>
#include "JZProjectTemplate.h"
#include "JZProject.h"
#include "JZScriptFile.h"
#include "JZNodeEvent.h"
#include "JZNodeValue.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeFunction.h"
#include "JZUiFile.h"

JZProjectTemplate *JZProjectTemplate::instance()
{
    static JZProjectTemplate inst;
    return &inst;
}        

bool JZProjectTemplate::initProject(QString path,QString name, QString temp)
{
    QString project_dir = path + "/" + name;
    if(!QFile::exists(project_dir))
        QDir().mkdir(project_dir);

    JZProject project;
    project.setFilePath(project_dir + "/" + name + ".jzproj");

    JZScriptFile *main_file = new JZScriptFile();
    main_file->setName("main.jz");
    project.addItem("./", main_file);
    
    auto *main_flow = main_file->addFlow("main");
    auto *global_def = main_file->addParamDefine("global");

    JZNodeEvent *start = new JZNodeStartEvent();
    main_flow->addNode(JZNodePtr(start));

    if (temp == "ui")
    {  
        auto func_inst = JZNodeFunctionManager::instance();
        auto window_file = new JZScriptFile();
        window_file->setName("mainwindow.jz");

        auto ui_file = new JZUiFile();
        ui_file->setName("mainwindow.ui");        

        project.addItem("./",window_file);
        project.addItem("./",ui_file);

        auto class_define = window_file->addClass("Mainwindow","Widget");        
        class_define->setUiFile("mainwindow.ui");
        class_define->addFlow("ÊÂ¼þ");
                
        global_def->addVariable("mainwindow", "Mainwindow");

        JZNodeSetParam *set_param = new JZNodeSetParam();
        JZNodeCreate *create = new JZNodeCreate();
        JZNodeFunction *func_show = new JZNodeFunction();
        int node_start = main_flow->nodeList()[0];
        int node_create = main_flow->addNode(JZNodePtr(create));
        int node_set = main_flow->addNode(JZNodePtr(set_param));
        int node_show = main_flow->addNode(JZNodePtr(func_show));

        set_param->setVariable("mainwindow");
        create->setClassName("Mainwindow");
        func_show->setFunction(func_inst->function("Widget.show"));

        JZNode *start = main_flow->getNode(0);
        main_flow->addConnect(start->flowOutGemo(), create->flowInGemo());

        main_flow->addConnect(create->flowOutGemo(), set_param->flowInGemo());
        main_flow->addConnect(create->paramOutGemo(0), set_param->paramInGemo(0));

        main_flow->addConnect(set_param->flowOutGemo(0), func_show->flowInGemo());
        main_flow->addConnect(set_param->paramOutGemo(0), func_show->paramInGemo(0));

        main_flow->setNodePos(node_start, QPointF(0, 0));
        main_flow->setNodePos(node_create, QPointF(150, 0));
        main_flow->setNodePos(node_set, QPointF(480, 0));
        main_flow->setNodePos(node_show, QPointF(730, 0));
    }
    project.save();
    project.saveAllItem();

    return true;
}