#include <QEventLoop>
#include <QDebug>
#include <math.h>
#include <QApplication>
#include <QTest>
#include "test_script.h"

ScriptTest::ScriptTest()
{
    m_testPath = qApp->applicationDirPath() + "/testcase";
}

void ScriptTest::call()
{
    JZEvent event;
    event.eventType = Event_programStart;
    m_engine.dealEvent(&event);
}

bool ScriptTest::build()
{
    JZNodeBuilder builder;
    if(!builder.build(&m_project,&m_program))
    {
        QString error = "build failed: " + builder.error();
        QTest::qVerify(false, qUtf8Printable(error), "", __FILE__, __LINE__);
        return false;
    }

    m_engine.setProgram(&m_program);
    m_engine.init();
    return true;
}

bool ScriptTest::run(bool async = false)
{
    if(!build())
        return false;

    if(async)
    {
        m_asyncThread = std::thread([this]{
                this->call();
            });
    }
    else
    {
        call();
    }

    return true;
}

void ScriptTest::initTestCase()
{
    QDir dir;
    if(!dir.exists(m_testPath))
        dir.mkdir(m_testPath);
}

void ScriptTest::init()
{
    m_project.initConsole();
    m_scriptFlow = (JZScriptFile*)m_project.getItem(m_project.mainScript());
    m_paramDef = (JZParamFile*)m_project.getItem("param.def");
}

void ScriptTest::cleanup()
{
    stop();
}

void ScriptTest::stop()
{
    m_engine.stop();
    if(m_asyncThread.joinable())
        m_asyncThread.join();
}

/*
    while(true)
    {
        i = 0;
        i = 1;
        ...
        i = 100;
    }
*/
QMap<int,int> ScriptTest::initWhileSetCase()
{
    auto script = m_scriptFlow;
    JZNodeEvent *node_start = new JZNodeEvent();
    JZNodeWhile *node_while = new JZNodeWhile();
    JZNodeLiteral *node_true = new JZNodeLiteral();

    m_paramDef->addVariable("i",Type_int);

    script->addNode(JZNodePtr(node_start));
    script->addNode(JZNodePtr(node_while));
    script->addNode(JZNodePtr(node_true));

    node_true->setLiteral(true);

    node_start->setEventType(Event_programStart);
    script->addConnect(JZNodeGemo(node_start->id(),node_start->flowOut()),JZNodeGemo(node_while->id(),node_while->flowIn()));
    script->addConnect(JZNodeGemo(node_true->id(),node_true->paramOut(0)),JZNodeGemo(node_while->id(),node_while->paramIn(0)));

    QList<JZNodeSetParam*> node_list;
    QMap<int,int> node_value;
    for(int i = 0; i < 100; i++)
    {
        JZNodeSetParam *node_set = new JZNodeSetParam();
        node_set->setVariable("i");
        node_set->setPropValue(node_set->paramIn(0),i);
        script->addNode(JZNodePtr(node_set));

        node_value[node_set->id()] = i;
        if(i == 0)
            script->addConnect(JZNodeGemo(node_while->id(),node_while->subFlowOut(0)),JZNodeGemo(node_set->id(),node_set->flowIn()));
        else
        {
            JZNodeSetParam *pre_node = node_list.back();
            script->addConnect(JZNodeGemo(pre_node->id(),pre_node->flowOut()),JZNodeGemo(node_set->id(),node_set->flowIn()));
        }
        node_list.push_back(node_set);
    }
    return node_value;
}


void ScriptTest::testProjectSave()
{
    QString projectPath = m_testPath + "/test.prj";
    bool ret;

    JZProject project;
    project.initConsole();

    //check flow
    JZScriptFile *scriptFlow = (JZScriptFile*)project.getItem(project.mainScript());
    JZNodeAdd *node_add = new JZNodeAdd();
    JZNodeParam *node_a = new JZNodeParam();
    JZNodeParam *node_b = new JZNodeParam();
    scriptFlow->addNode(JZNodePtr(node_a));
    scriptFlow->addNode(JZNodePtr(node_b));
    scriptFlow->addNode(JZNodePtr(node_add));
    scriptFlow->addConnect(node_a->paramOutGemo(0),node_add->paramInGemo(0));
    scriptFlow->addConnect(node_b->paramOutGemo(0),node_add->paramInGemo(1));
    scriptFlow->save();


    //check define
    JZParamFile *paramDef = (JZParamFile*)project.getItem("param.def");

    paramDef->addVariable("heello",Type_double,3.0);
    paramDef->save();
    ret = project.saveAs(projectPath);
    QVERIFY2(ret,"project.saveAs");

    ret = project.open(projectPath);
    QVERIFY2(ret,"project.open");

    paramDef = (JZParamFile*)project.getItem("param.def");
    JZParamDefine *def = paramDef->getVariable("heello");
    QVERIFY2(def,"getVariable");
    QCOMPARE(def->name,"heello");
    QCOMPARE(def->dataType,Type_double);
    QCOMPARE(def->value,3.0);
}

void ScriptTest::testParamBinding()
{
    /*
        c = a + b
    */
    auto script = m_scriptFlow;
    m_paramDef->addVariable("a",Type_int,10);
    m_paramDef->addVariable("b",Type_int,20);
    m_paramDef->addVariable("c",Type_int);

    JZNodeSetParamDataFlow *node_c = new JZNodeSetParamDataFlow();
    JZNodeAdd *node_add = new JZNodeAdd();
    JZNodeParam *node_a = new JZNodeParam();
    JZNodeParam *node_b = new JZNodeParam();

    script->addNode(JZNodePtr(node_a));
    script->addNode(JZNodePtr(node_b));
    script->addNode(JZNodePtr(node_c));
    script->addNode(JZNodePtr(node_add));

    node_a->setVariable("a");
    node_b->setVariable("b");
    node_c->setVariable("c");
    script->addConnect(node_a->paramOutGemo(0),node_add->paramInGemo(0));
    script->addConnect(node_b->paramOutGemo(0),node_add->paramInGemo(1));

    script->addConnect(node_add->paramOutGemo(0),node_c->paramInGemo(0));

    if(!run())
        return;

    QCOMPARE(30,m_engine.getVariable("c"));

    m_engine.setVariable("a",200);
    QCOMPARE(220,m_engine.getVariable("c"));

    m_engine.setVariable("b",200);
    QCOMPARE(400,m_engine.getVariable("c"));
}

void ScriptTest::testSequeue()
{
    /*
        a = 1 + 2
        b = 2 + 3
        c = 3 + 4
        d = 4 + 5
    */    
    JZScriptFile *script = m_scriptFlow;
    JZNodeEngine *engine = &m_engine;
    auto *paramDef = m_paramDef;

    JZNodeEvent *node_start = new JZNodeEvent();
    JZNodeSequence *node_seq = new JZNodeSequence();

    int start_id = script->addNode(JZNodePtr(node_start));
    int for_id = script->addNode(JZNodePtr(node_seq));

    paramDef->addVariable("a",Type_int);
    paramDef->addVariable("b",Type_int);
    paramDef->addVariable("c",Type_int);
    paramDef->addVariable("d",Type_int);

    node_seq->addSequeue();
    node_seq->addSequeue();
    node_seq->addSequeue();
    node_seq->addSequeue();

    node_start->setEventType(Event_programStart);
    script->addConnect(JZNodeGemo(start_id,node_start->flowOut()),JZNodeGemo(for_id,node_seq->flowIn()));
    for(int i = 0; i < 4; i++)
    {
        JZNodeSetParam *node_set = new JZNodeSetParam();
        node_set->setVariable(QString('a' + i));

        JZNodeAdd *node_add = new JZNodeAdd();
        node_add->setPropValue(node_add->paramIn(0),i+1);
        node_add->setPropValue(node_add->paramIn(1),i+2);

        script->addNode(JZNodePtr(node_set));
        script->addNode(JZNodePtr(node_add));

        script->addConnect(node_seq->subFlowOutGemo(i),node_set->flowInGemo());
        script->addConnect(node_add->paramOutGemo(0),node_set->paramInGemo(0));
    }
    if(!run())
        return;

    int a = engine->getVariable("a").toInt();
    int b = engine->getVariable("b").toInt();
    int c = engine->getVariable("c").toInt();
    int d = engine->getVariable("d").toInt();
    QCOMPARE(a,3);
    QCOMPARE(b,5);
    QCOMPARE(c,7);
    QCOMPARE(d,9);
}

void ScriptTest::testFor()
{
    /*
        for(int i = 5; i < 100; i++)
        {
            if(i > 20)
                break;            
        }    
    */

    JZScriptFile *script = m_scriptFlow;

    JZNodeEvent *node_start = new JZNodeEvent();
    JZNodeFor *node_for = new JZNodeFor();
    JZNodeBranch *node_branch = new JZNodeBranch();
    JZNodeBreak *node_break = new JZNodeBreak();
    JZNodeGT *node_gt = new JZNodeGT();
    JZNodeSetParam *node_set = new JZNodeSetParam();
    
    int start_id = script->addNode(JZNodePtr(node_start));
    int for_id = script->addNode(JZNodePtr(node_for));    
    int branch_id = script->addNode(JZNodePtr(node_branch));
    int break_id = script->addNode(JZNodePtr(node_break));
    int gt_id = script->addNode(JZNodePtr(node_gt));
    int set_id = script->addNode(JZNodePtr(node_set));

    m_paramDef->addVariable("i",Type_int);
    node_set->setVariable("i");

    //start
    node_start->setEventType(Event_programStart);
    script->addConnect(JZNodeGemo(start_id,node_start->flowOut()),JZNodeGemo(for_id,node_for->flowIn()));
    node_for->setPropValue(node_for->paramIn(0),5);
    node_for->setPropValue(node_for->paramIn(1),100);

    // if(i < 10)    
    script->addConnect(JZNodeGemo(for_id,node_for->subFlowOut(0)),JZNodeGemo(branch_id,node_branch->flowIn()));    
    script->addConnect(JZNodeGemo(for_id,node_for->paramOut(0)),JZNodeGemo(gt_id,node_gt->paramIn(0)));
    node_gt->setPropValue(node_gt->paramIn(1),20);

    script->addConnect(JZNodeGemo(gt_id,node_gt->paramOut(0)),JZNodeGemo(branch_id,node_branch->paramIn(0)));

    script->addConnect(JZNodeGemo(branch_id,node_branch->flowOut(0)),JZNodeGemo(set_id,node_set->flowIn()));
    script->addConnect(JZNodeGemo(for_id,node_for->paramOut(0)),JZNodeGemo(set_id,node_set->paramIn(0)));

    script->addConnect(JZNodeGemo(set_id,node_set->flowOut()),JZNodeGemo(break_id,node_break->flowIn()));        
    
    if(!run())
        return;

    int i = m_engine.getVariable("i").toInt();
    QCOMPARE(i,21);
}

void ScriptTest::testForEach()
{
    /*
        for(int i = 0; i < list.size(); i++)
        {
            sum = sum + i;
        }
    */    
    JZScriptFile *script = m_scriptFlow;
    JZNodeEngine *engine = &m_engine;
    JZParamFile *paramDef = m_paramDef;
    paramDef->addVariable("sum",Type_int,0);
    paramDef->addVariable("a",JZNodeObjectManager::instance()->getClassId("list"));

    JZNodeEvent *node_start = new JZNodeEvent();
    JZNodeForEach *node_for = new JZNodeForEach();
    JZNodeParam *node_param = new JZNodeParam();
    JZNodeParam *node_sum = new JZNodeParam();
    JZNodeAdd *node_add = new JZNodeAdd();
    JZNodeSetParam *node_set = new JZNodeSetParam();
    node_param->setVariable("a");
    node_sum->setVariable("sum");
    node_set->setVariable("sum");

    JZNodeSetParam *node_int_a = new JZNodeSetParam();
    node_int_a->setVariable("a");

    JZNodeCreate *node_create = new JZNodeCreate();
    node_create->setClassName("list");

    int start_id = script->addNode(JZNodePtr(node_start));
    script->addNode(JZNodePtr(node_for));
    script->addNode(JZNodePtr(node_param));
    script->addNode(JZNodePtr(node_sum));
    script->addNode(JZNodePtr(node_add));
    script->addNode(JZNodePtr(node_set));
    script->addNode(JZNodePtr(node_create));
    script->addNode(JZNodePtr(node_int_a));

    //start
    node_start->setEventType(Event_programStart);
    script->addConnect(JZNodeGemo(start_id,node_start->flowOut()),node_create->flowInGemo());
    script->addConnect(node_create->flowOutGemo(),node_set->flowInGemo());
    script->addConnect(node_create->paramOutGemo(0),node_set->paramInGemo(0));

    script->addConnect(node_set->flowOutGemo(),node_for->flowInGemo());
    script->addConnect(node_param->paramOutGemo(0),node_for->paramInGemo(0));

    // sum = sum + i
    script->addConnect(node_sum->paramOutGemo(0),node_add->paramInGemo(0));
    script->addConnect(node_for->paramOutGemo(0),node_add->paramInGemo(1));

    script->addConnect(node_add->paramOutGemo(0),node_set->paramInGemo(0));
    script->addConnect(node_for->subFlowOutGemo(0),node_set->flowInGemo());

    if(!build())
        return;

    QVariantList out;
    QVariant list = engine->getVariable("a");
    qDebug() << list;
    for(int i = 0; i < 10; i++)
        engine->call("list.push_back",{list,i+1},out);

    call();

    QVariant sum = engine->getVariable("sum");
    QCOMPARE(sum,55);
}

void ScriptTest::testWhileLoop()
{
    /*
        while(i < 10)   
            i = i + 1;
    */
    JZScriptFile *script = m_scriptFlow;

    JZNodeEvent *node_start = new JZNodeEvent();
    JZNodeSetParam *node_set = new JZNodeSetParam();
    JZNodeParam *node_param = new JZNodeParam();
    JZNodeWhile *node_while = new JZNodeWhile();    
    JZNodeLiteral *node_value = new JZNodeLiteral();
    JZNodeLiteral *node_value10 = new JZNodeLiteral();
    JZNodeAdd *node_add = new JZNodeAdd();    
    JZNodeNE *node_eq = new JZNodeNE();        
    
    int start_id = script->addNode(JZNodePtr(node_start));
    int param_id = script->addNode(JZNodePtr(node_param));    
    int set_id = script->addNode(JZNodePtr(node_set));
    int while_id = script->addNode(JZNodePtr(node_while));
    int value_id = script->addNode(JZNodePtr(node_value));
    int add_id = script->addNode(JZNodePtr(node_add));    
    int eq_id = script->addNode(JZNodePtr(node_eq));
    int value10_id = script->addNode(JZNodePtr(node_value10));

    //project.addVariable("i",1);
    node_param->setVariable("i");
    node_set->setVariable("i");

    node_value->setLiteral(1);
    node_value10->setLiteral(10);

    //start
    node_start->setEventType(Event_programStart);
    script->addConnect(JZNodeGemo(start_id,node_start->flowOut()),JZNodeGemo(while_id,node_while->flowIn()));

    // param < 10    
    script->addConnect(JZNodeGemo(param_id,node_param->paramOut(0)),JZNodeGemo(eq_id,node_eq->paramIn(0)));
    script->addConnect(JZNodeGemo(value10_id,node_param->paramOut(0)),JZNodeGemo(eq_id,node_eq->paramIn(1)));

    // while
    script->addConnect(JZNodeGemo(eq_id,node_eq->paramOut(0)),JZNodeGemo(while_id,node_while->paramIn(0)));    
    script->addConnect(JZNodeGemo(while_id,node_while->subFlowOut(0)),JZNodeGemo(set_id,node_set->flowIn()));

    // i = i + 1
    script->addConnect(JZNodeGemo(param_id,node_param->paramOut(0)),JZNodeGemo(add_id,node_add->paramIn(0)));
    script->addConnect(JZNodeGemo(value_id,node_value->paramOut(0)),JZNodeGemo(add_id,node_add->paramIn(1)));
        
    script->addConnect(JZNodeGemo(add_id,node_add->paramOut(0)),JZNodeGemo(set_id,node_set->paramIn(0)));
    
    if(run())
        return;

    int sum = m_engine.getVariable("i").toInt();
    QCOMPARE(sum,55);
}

void ScriptTest::testBreakPoint()
{    
    JZScriptFile *script = m_scriptFlow;
    JZNodeEngine *engine = &m_engine;
    QMap<int,int> node_value = initWhileSetCase();
    if(!run(true))
        return;

    for(int i = 0; i < 20; i++)
    {
        QThread::msleep(100);        
        engine->pause();

        auto info = engine->runtimeInfo();
        QVERIFY(info.status == Status_pause);

        if(node_value.contains(info.nodeId))
        {
            int value = engine->getVariable("i").toInt();
            if(!(abs(value - node_value[info.nodeId]) <= 1))
            {
                QVERIFY2(false,"paruse/resume error");
            }
        }
        engine->resume();
    }

    int value = 0;
    for(int i = 0; i < 20; i++)
    {
        int node_id = node_value.keys()[rand()%100];
        engine->addBreakPoint(script->itemPath(),node_id);
        QThread::msleep(100);

        auto info = engine->runtimeInfo();
        QVERIFY(info.status == Status_pause);

        value = engine->getVariable("i").toInt();
        if(node_value[info.nodeId] - value != 1)
            QVERIFY2(false,"before stepover error");

        engine->stepOver();
        QThread::msleep(10);
        info = engine->runtimeInfo();
        QVERIFY(info.status == Status_pause);

        value = engine->getVariable("i").toInt();
        if((node_value[info.nodeId]+100)%100 - value != 1)
            QVERIFY2(false,"after stepover error");

        engine->removeBreakPoint(script->itemPath(),node_id);
        engine->resume();
    }
    stop();
}

void ScriptTest::testDebugServer()
{           
    JZScriptFile *script = m_scriptFlow;
    JZNodeEngine *engine = &m_engine;
    QMap<int,int> node_value = initWhileSetCase();
    if(!run(true))
        return;

    JZNodeDebugServer server;
    JZNodeDebugClient client;
    server.setEngine(engine);
    if(!server.startServer(18888))
    {
        QVERIFY2(false,"start server failded");
    }

    if(!client.connectToServer("127.0.0.1",18888))
    {
        QVERIFY2(false,"connect to server failded");
    }

    for(int i = 0; i < 20; i++)
    {
        QThread::msleep(100);
        client.pause();

        auto info = client.runtimeInfo();
        QVERIFY(info.status == Status_pause);

        if(node_value.contains(info.nodeId))
        {
            int value = client.getVariable("i").toInt();
            if(!(abs(value - node_value[info.nodeId]) <= 1))
            {
                QVERIFY2(false,"paruse/resume error");
            }
        }        
        client.resume();
    }

    int value = 0;
    for(int i = 0; i < 20; i++)
    {
        int node_id = node_value.keys()[rand()%100];
        client.addBreakPoint(script->itemPath(),node_id);
        QThread::msleep(100);

        auto info = client.runtimeInfo();
        QVERIFY(info.status == Status_pause);

        value = client.getVariable("i").toInt();
        if(node_value[info.nodeId] - value != 1)
            QVERIFY2(false,"before stepover error");

        client.stepOver();
        QThread::msleep(50);
        info = client.runtimeInfo();
        QVERIFY(info.status == Status_pause);

        value = client.getVariable("i").toInt();
        if((node_value[info.nodeId]+100)%100 - value != 1)
            QVERIFY2(false,"after stepover error");

        client.removeBreakPoint(script->itemPath(),node_id);
        client.resume();
    }
    client.stop();
    server.stopServer();
}

void ScriptTest::testExpr()
{    
    JZScriptFile *script = m_scriptFlow;
    JZNodeEngine *engine = &m_engine;
    JZParamFile *paramDef = m_paramDef;

    QVector<int> op_type = {Node_add,Node_sub,Node_mul,Node_div,Node_mod,Node_eq,Node_ne,Node_le,
        Node_ge,Node_lt,Node_gt,Node_and,Node_or,Node_bitand,Node_bitor,Node_bitxor};

    JZNodeEvent *node_start = new JZNodeEvent();      
    JZNodeParam *node_a = new JZNodeParam();
    JZNodeParam *node_b = new JZNodeParam();

    int a = 100,b = 50;
    node_start->setEventType(Event_programStart);
    paramDef->addVariable("a",Type_int,a);
    paramDef->addVariable("b",Type_int,b);

    node_a->setVariable("a");
    node_b->setVariable("b");

    script->addNode(JZNodePtr(node_start));    
    script->addNode(JZNodePtr(node_a));
    script->addNode(JZNodePtr(node_b));    

    QMap<int,int> node_value;
    JZNodeSetParam *pre_node = nullptr;
    for(int i = 0; i < op_type.size(); i++)
    {
        paramDef->addVariable("i" + QString::number(i),Type_int);

        JZNodeSetParam *node_set = new JZNodeSetParam();
        node_set->setVariable("i" + QString::number(i));
        script->addNode(JZNodePtr(node_set));

        auto node_op = JZNodeFactory::instance()->createNode(op_type[i]);
        script->addNode(JZNodePtr(node_op));

        script->addConnect(node_a->paramOutGemo(0),node_op->paramInGemo(0));
        script->addConnect(node_b->paramOutGemo(0),node_op->paramInGemo(1));
        script->addConnect(node_op->paramOutGemo(0),node_set->paramInGemo(0));

        node_value[node_set->id()] = i;
        if(i == 0)
            script->addConnect(node_start->flowOutGemo(),node_set->flowInGemo());
        else       
            script->addConnect(pre_node->flowOutGemo(),node_set->flowInGemo());

        pre_node = node_set;
    }

    if(!run())
        return;


    QVariant i0 = engine->getVariable("i0");
    QVariant i1 = engine->getVariable("i1");
    QVariant i2 = engine->getVariable("i2");
    QVariant i3 = engine->getVariable("i3");
    QCOMPARE(i0,a + b);
    QCOMPARE(i1,a - b);
    QCOMPARE(i2,a * b);
    QCOMPARE(i3,a / b);
}

void ScriptTest::testCustomExpr()
{    
    JZScriptFile *script = m_scriptFlow;
    JZNodeEngine *engine = &m_engine;
    JZParamFile *paramDef = m_paramDef;

    JZNodeEvent *node_start = new JZNodeEvent();
    JZNodeParam *node_a = new JZNodeParam();
    JZNodeParam *node_b = new JZNodeParam();
    JZNodeExpression *node_expr = new JZNodeExpression();

    node_start->setEventType(Event_programStart);
    paramDef->addVariable("a",Type_int,2);
    paramDef->addVariable("b",Type_int,3);
    paramDef->addVariable("c",Type_int,0);

    node_a->setVariable("a");
    node_b->setVariable("b");

    script->addNode(JZNodePtr(node_start));
    script->addNode(JZNodePtr(node_a));
    script->addNode(JZNodePtr(node_b));

    QString error;
    if(!node_expr->setExpr("c = pow(a,b) - pow(b,a)",error))
    {
        QVERIFY2(false,qUtf8Printable(error));
        return;
    }
    script->addNode(JZNodePtr(node_expr));

    JZNodeSetParam *node_set = new JZNodeSetParam();
    node_set->setVariable("c");
    script->addNode(JZNodePtr(node_set));

    script->addConnect(node_a->paramOutGemo(0),node_expr->paramInGemo(0));
    script->addConnect(node_b->paramOutGemo(0),node_expr->paramInGemo(1));
    script->addConnect(node_expr->paramOutGemo(0),node_set->paramInGemo(0));

    script->addConnect(node_start->flowOutGemo(),node_set->flowInGemo());

    if(!run())
        return;

    int a = engine->getVariable("a").toInt();
    int b = engine->getVariable("b").toInt();
    QVariant c = engine->getVariable("c");
    qDebug() << c << pow(a,b) - pow(b,a);
}

void ScriptTest::testFunction()
{
    /*
        int fab(int n)
        {
            if(n >= 2)
                return fab(n-1) + fab(n-2);
            else
                return n;
        }
    */    

    FunctionParam inParam,outParam;
    inParam.name = "n";
    inParam.dataType = Type_int;
    outParam.name = "result";
    outParam.dataType = Type_int;

    FunctionDefine fab;
    fab.name = "fab";
    fab.paramIn.push_back(inParam);
    fab.paramOut.push_back(outParam);
    JZScriptFile *script = m_project.addFunction(fab);

    JZNode *node_start = script->getNode(0);
    JZNodeBranch *node_branch = new JZNodeBranch();    
    JZNodeGE *node_ge = new JZNodeGE();
    node_ge->setPropValue(node_ge->paramIn(1),2);
    JZNodeAdd *node_add = new JZNodeAdd();    

    JZNodeFunction *func1 = new JZNodeFunction();
    func1->setFunction(&fab);
    JZNodeFunction *func2 = new JZNodeFunction();
    func2->setFunction(&fab);

    JZNodeSub *sub1 = new JZNodeSub();
    JZNodeSub *sub2 = new JZNodeSub();

    JZNodeParam *n = new JZNodeParam();
    n->setVariable("n");

    JZNodeReturn *ret1 = new JZNodeReturn();
    JZNodeReturn *ret2 = new JZNodeReturn();    
    ret1->addParamIn("n",Prop_editValue);
    ret2->addParamIn("n",Prop_editValue);

    script->addNode(JZNodePtr(node_branch));
    script->addNode(JZNodePtr(node_ge));
    script->addNode(JZNodePtr(func1));
    script->addNode(JZNodePtr(func2));
    script->addNode(JZNodePtr(sub1));
    script->addNode(JZNodePtr(sub2));
    script->addNode(JZNodePtr(n));
    script->addNode(JZNodePtr(ret1));
    script->addNode(JZNodePtr(ret2));
    script->addNode(JZNodePtr(node_add));        

    script->addConnect(node_start->flowOutGemo(),node_branch->flowInGemo());

    /* if(n >= 2) */
    script->addConnect(n->paramOutGemo(0),node_ge->paramInGemo(0));
    script->addConnect(node_ge->paramOutGemo(0),node_branch->paramInGemo(0));
    script->addConnect(node_branch->flowOutGemo(0),ret1->flowInGemo());
    script->addConnect(node_branch->flowOutGemo(1),ret2->flowInGemo());

    /* return fab(n-1) + fab(n-2) */
    sub1->setPropValue(sub1->paramIn(1),1);
    sub2->setPropValue(sub2->paramIn(1),2);
    script->addConnect(n->paramOutGemo(0),sub1->paramInGemo(0));
    script->addConnect(n->paramOutGemo(0),sub2->paramInGemo(0));
    script->addConnect(sub1->paramOutGemo(0),func1->paramInGemo(0));
    script->addConnect(sub2->paramOutGemo(0),func2->paramInGemo(0));
    script->addConnect(func1->paramOutGemo(0),node_add->paramInGemo(0));
    script->addConnect(func2->paramOutGemo(0),node_add->paramInGemo(1));
    script->addConnect(node_add->paramOutGemo(0),ret1->paramInGemo(0));

    /* return n */
    script->addConnect(n->paramOutGemo(0),ret2->paramInGemo(0));

    if(!run())
        return;    

    QVariantList out;    
    bool ret = m_engine.call("fab",{15},out);
    QVERIFY(ret);
    QCOMPARE(out[0].toInt(),610);

    ret = m_engine.call("fab",{16},out);
    QVERIFY(ret);
    QCOMPARE(out[0].toInt(),987);
}

void ScriptTest::testClass()
{       
    JZProject *project = &m_project;
    JZParamFile *paramDef = m_paramDef;

    auto classBase = project->addClass("ClassBase");
    project->addClass("ClassA","ClassBase");
    project->addClass("ClassB","ClassBase");

    classBase->addMemberVariable("a",Type_int,50);
    paramDef->addVariable("a",JZNodeObjectManager::instance()->getClassId("ClassA"));
    paramDef->addVariable("b",JZNodeObjectManager::instance()->getClassId("ClassB"));

    if(!run())
        return;
/*
    QVariant value;
    value = m_engine.getVariable("a.a");
    QCOMPARE(value,50);

    m_engine.setVariable("a.a",100);
    value = m_engine.getVariable("a.a");
    QCOMPARE(value,100);

    m_engine.setVariable("b.a",200);
    value = m_engine.getVariable("b.a");
    QCOMPARE(value,200);
*/
}

void ScriptTest::testBind()
{
    auto add = [](int a,int b,int c)->int{ return a + b + c; };
    auto str_left = [](const QString &inst,int size)->QString{
        return inst.left(size);
    };

    QString str = "test QString>()lalala";
    QVariantList out;

    auto impl_add = jzbind::createFuncion(add);
    impl_add->call({100,200,35},out);
    QCOMPARE(out[0],100 + 200 + 35);

    auto impl_left = jzbind::createFuncion(str_left);
    impl_left->call({str,100},out);    

    CFunction *impl_left_in;
    {
        auto str_left_in = [](const QString &inst,int size)->QString{
            return inst.left(size);
        };
        impl_left_in = jzbind::createFuncion(str_left_in);
    }
    impl_left_in->call({str,100},out);
}

void ScriptTest::testCClass()
{        
    JZNodeEngine *engine = &m_engine;
    JZParamFile *paramDef = m_paramDef;

    paramDef->addVariable("a",Type_string);
    paramDef->addVariable("b",Type_string);
    paramDef->addVariable("c",Type_string);
    if(!run())
        return;

    QString a = "i'love jznode";
    QString b = "jznode";
    QString c = "money";
    engine->setVariable("a",a);
    engine->setVariable("b",b);
    engine->setVariable("c",c);

    QVariant va = engine->getVariable("a");
    QVariant vb = engine->getVariable("b");
    QVariant vc = engine->getVariable("c");
    QVariantList out;
    engine->call("string.left",{va,6},out);
    QCOMPARE(out[0],a.left(6));

    engine->call("string.size",{out[0]},out);
    QCOMPARE(out[0],a.left(6).size());

    engine->call("string.replace",{va,vb,vc},out);
    QCOMPARE(out[0],a.replace(b,c));
}

void test_script()
{    
    ScriptTest s; 
    QTest::qExec(&s);
}
