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

    //timeout
    JZNodeSingleEvent *timeout = new  JZNodeSingleEvent();
    timeout->setSingle(time_meta->className, time_meta->single("timeout"));
    timeout->setVariable("timer");

    JZNodeFor *node_for = new JZNodeFor();
    node_for->setRange(0, 10000, 1);

    JZNodePrint *node_print = new JZNodePrint();
    node_print->setParamInValue(0, "hello");

    JZNodeNop *nop = new JZNodeNop();
    script->addNode(JZNodePtr(nop));
    script->addNode(JZNodePtr(timeout));
    script->addNode(JZNodePtr(node_for));
    script->addNode(JZNodePtr(node_print));

    script->addConnect(timeout->flowOutGemo(0), node_print->flowInGemo());
    script->addConnect(node_print->flowOutGemo(0), node_for->flowInGemo());
    script->addConnect(node_for->subFlowOutGemo(0), nop->flowInGemo());

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

void TestServer::run()
{    

    JZNodeBuilder builder;
    if (!builder.build(m_project, &m_program))
    {
        Q_ASSERT(0);
    }    

    JZNodeEngine engine;
    m_engine = &engine;
    engine.setProgram(&m_program);
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