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
    QString dirpath = qApp->applicationDirPath() + "/test";
    if (!QFile::exists(dirpath))
        QDir().mkdir(dirpath);

    m_project = project;    
    m_project->newProject(dirpath, "test", "console");

    JZScriptClassItem *class_file = m_project->mainFile()->addClass("TestClass", "Object");
    class_file->addMemberVariable("timer", Type_timer);

    m_project->addGlobalVariable("t", "TestClass");

    addTimeoutFunction();
    addInitFunction();

    projectUpdateLayout(m_project);
    m_project->saveAllItem();
    m_project->save();
}

void TestServer::addInitFunction()
{        
    auto time_meta = JZNodeObjectManager::instance()->meta(Type_timer);

    JZScriptItem *script = m_project->mainFunction();    

    JZNodeCreate *create_timer = new JZNodeCreate();
    create_timer->setClassName(time_meta->className);

    JZNodeSetParam *set_timer = new JZNodeSetParam();
    set_timer->setVariable("t.timer");

    JZNodeFunction *start_timer = new JZNodeFunction();
    start_timer->setFunction(time_meta->function("start"));
    start_timer->setParamInValue(1, "500");

    auto start_node = script->getNode(0);
    script->addNode(create_timer);
    script->addNode(set_timer);
    script->addNode(start_timer);

    JZNodeSetParam *set_param = new JZNodeSetParam();
    JZNodeCreate *create = new JZNodeCreate();

    set_param->setVariable("t");
    create->setClassName("TestClass");

    script->addNode(set_param);
    script->addNode(create);
    script->addConnect(start_node->flowOutGemo(), set_param->flowInGemo());
    script->addConnect(create->paramOutGemo(0), set_param->paramInGemo(1));

    script->addConnect(set_param->flowOutGemo(0), set_timer->flowInGemo());
    script->addConnect(create_timer->paramOutGemo(0), set_timer->paramInGemo(1));
    
    script->addConnect(set_timer->flowOutGemo(0), start_timer->flowInGemo());
    script->addConnect(set_timer->paramOutGemo(0), start_timer->paramInGemo(0));    

    JZNodeSignalConnect *node_connect = new JZNodeSignalConnect();    
    JZNodeFunctionPointer *node_timeout = new JZNodeFunctionPointer();
    JZNodeFunctionPointer *node_slot = new JZNodeFunctionPointer();
    script->addNode(node_connect);    
    script->addNode(node_timeout);
    script->addNode(node_slot);

    node_timeout->setFucntion("Timer.timeout");
    node_slot->setFucntion("TestClass.onTimer");

    script->addConnect(start_timer->flowOutGemo(0), node_connect->flowInGemo());
    script->addConnect(set_timer->paramOutGemo(0), node_connect->paramInGemo(0));
    script->addConnect(node_timeout->paramOutGemo(0), node_connect->paramInGemo(1));
    script->addConnect(set_param->paramOutGemo(0), node_connect->paramInGemo(2));
    script->addConnect(node_slot->paramOutGemo(0), node_connect->paramInGemo(3));
}

void TestServer::addTimeoutFunction()
{
    //timeout
    auto meta = JZNodeObjectManager::instance()->meta("TestClass");
    auto def = meta->initMemberFunction("onTimer");
    JZScriptClassItem *class_file = m_project->getClass("TestClass");
    auto script = class_file->addMemberFunction(def);
    auto node_start = script->getNode(0);

    JZNodeFor *node_for = new JZNodeFor();
    node_for->setRange(0, 1, 10);

    JZNodePrint *node_print = new JZNodePrint();
    node_print->setParamInValue(0, "hello");

    JZNodeDisplay *node_display = new JZNodeDisplay();

    JZNodeNop *nop = new JZNodeNop();
    script->addNode(nop);    
    script->addNode(node_for);
    script->addNode(node_print);
    script->addNode(node_display);

    script->addConnect(node_start->flowOutGemo(0), node_print->flowInGemo());
    script->addConnect(node_print->flowOutGemo(0), node_for->flowInGemo());
    script->addConnect(node_for->subFlowOutGemo(0), nop->flowInGemo());
    script->addConnect(node_for->paramOutGemo(0), node_display->paramInGemo(0));        
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
    QString dirpath = qApp->applicationDirPath() + "/test/build/test.program";
    bool ret = m_program.load(dirpath);
    Q_ASSERT(ret);

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
    engine.call("main", in,out);
    exec();
    m_engine = nullptr;
}