#include "test_server.h"
#include "JZNodeBuilder.h"
#include "JZNodeValue.h"
#include "JZNodeFunction.h"
#include "JZNodeUtils.h"
#include <QApplication>
#include <QDir>

TestServer::TestServer()
{    
    m_project = nullptr;
    m_engine = nullptr;
}

void TestServer::init(JZProject *project)
{
    QString filepath = qApp->applicationDirPath() + "/test";
    if (!QDir().exists(filepath))
        QDir().mkpath(filepath);

    m_project = project;    

    auto func_inst = JZNodeFunctionManager::instance();

    m_project->initConsole();
    JZScriptFile *script = (JZScriptFile*)m_project->getItem(m_project->mainScript());
    JZParamFile *paramDef = (JZParamFile*)m_project->getItem("param.def");
    paramDef->addVariable("timer", Type_timer);

    JZScriptClassFile *class_file = m_project->addClass("TestClass", "TestClass");
    class_file->addMemberVariable("i32", Type_int);
    class_file->addMemberVariable("list", Type_list);
    class_file->addMemberVariable("map", Type_map);

    FunctionDefine func_def;
    func_def.name = "testFunc";
    func_def.isFlowFunction = true;
    func_def.paramIn.push_back(JZParamDefine("this", class_file->classType()));
    class_file->addMemberFunction(func_def);

    auto time_meta = JZNodeObjectManager::instance()->meta(Type_timer);

    JZNodeCreate *create_timer = new  JZNodeCreate();
    create_timer->setClassName(time_meta->className);

    JZNodeSetParam *set_timer = new JZNodeSetParam();
    set_timer->setVariable("timer");

    JZNodeFunction *start_timer = new JZNodeFunction();
    start_timer->setFunction(time_meta->function("start"));
    start_timer->setParamInValue(1, 500);

    auto start_node = script->getNode(0);
    script->addNode(JZNodePtr(create_timer));
    script->addNode(JZNodePtr(set_timer));
    script->addNode(JZNodePtr(start_timer));

    script->addConnect(start_node->flowOutGemo(0), create_timer->flowInGemo());
    script->addConnect(create_timer->paramOutGemo(0), set_timer->paramInGemo(0));
    script->addConnect(create_timer->flowOutGemo(0), set_timer->flowInGemo());
    script->addConnect(set_timer->paramOutGemo(0), start_timer->paramInGemo(0));
    script->addConnect(set_timer->flowOutGemo(0), start_timer->flowInGemo());

    auto param_def = (JZParamFile *)m_project->getItem("./param.def");
    param_def->addVariable("test", JZClassId("TestClass"));

    JZNodeSetParam *set_param = new JZNodeSetParam();
    JZNodeCreate *create = new JZNodeCreate();

    set_param->setVariable("test");
    create->setClassName("TestClass");
    
    script->addNode(JZNodePtr(set_param));
    script->addNode(JZNodePtr(create));
    script->addConnect(start_timer->flowOutGemo(), create->flowInGemo());
    script->addConnect(create->flowOutGemo(), set_param->flowInGemo());
    script->addConnect(create->paramOutGemo(0), set_param->paramInGemo(0));
    

    //timeout
    JZNodeSingleEvent *timeout = new  JZNodeSingleEvent();
    timeout->setSingle(time_meta->className, time_meta->single("timeout"));
    timeout->setVariable("timer");

    JZNodeFor *node_for = new JZNodeFor();
    node_for->setRange(0, 10000, 1);

    JZNodePrint *node_print = new JZNodePrint();
    node_print->setParamInValue(0, "hello");

    JZNodeDisplay *node_display = new JZNodeDisplay();

    JZNodeNop *nop = new JZNodeNop();
    script->addNode(JZNodePtr(nop));
    script->addNode(JZNodePtr(timeout));
    script->addNode(JZNodePtr(node_for));
    script->addNode(JZNodePtr(node_print));
    script->addNode(JZNodePtr(node_display));

    JZNodeFunction *node_func = new JZNodeFunction();
    node_func->setFunction(&class_file->getMemberFunction(func_def.name)->function());
    script->addNode(JZNodePtr(node_func));

    script->addConnect(timeout->flowOutGemo(0), node_print->flowInGemo());
    script->addConnect(node_print->flowOutGemo(0), node_for->flowInGemo());
    script->addConnect(node_for->subFlowOutGemo(0), nop->flowInGemo());
    script->addConnect(node_for->paramOutGemo(0), node_display->paramInGemo(0));
    script->addConnect(nop->flowOutGemo(0), node_func->flowInGemo());

    JZNodeParam *get_param = new JZNodeParam();
    get_param->setVariable("test");
    script->addNode(JZNodePtr(get_param));

    script->addConnect(get_param->paramOutGemo(0), node_func->paramInGemo(0));

    m_project->saveAllItem();
    projectUpdateLayout(m_project);
    m_project->saveAs(filepath + "/localTest.jzprogram");
}

void TestServer::stop()
{
    if(m_engine)
        m_engine->stop();
    quit();
    wait();
}

void TestServer::onRuntimeError()
{
    quit();
}

void TestServer::run()
{    

    JZNodeBuilder builder;
    if (!builder.build(m_project, &m_program))
    {
        Q_ASSERT(0);
    }    

    JZNodeEngine engine;
    connect(&engine, &JZNodeEngine::sigRuntimeError, this, &TestServer::onRuntimeError);

    m_engine = &engine;
    engine.setProgram(&m_program);
    engine.setDebug(true);
    engine.init();
     
    JZNodeDebugServer server;
    server.setEngine(&engine);
    server.startServer(19888);
    server.waitForAttach();       
        
    QVariantList in, out;
    engine.call("__main__", in,out);
    exec();
    m_engine = nullptr;
}