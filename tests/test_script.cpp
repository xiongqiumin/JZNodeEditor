#include <QEventLoop>
#include <QDebug>
#include <math.h>
#include <QApplication>
#include <QTest>
#include "test_script.h"
#include "JZRegExpHelp.h"
#include "JZNodeObjectParser.h"

ScriptTest::ScriptTest()
{
    m_testPath = qApp->applicationDirPath() + "/testcase";
    m_dump = false;
    m_file = nullptr;

    connect(&m_engine, &JZNodeEngine::sigRuntimeError, this, &ScriptTest::onRuntimeError);
}

void ScriptTest::call()
{
    QVariantList in, out;
    m_engine.call("__main__", in, out);
}

bool ScriptTest::build()
{    
    JZNodeBuilder builder;
    if(!builder.build(&m_project,&m_program))
    {
        m_error = "build failed: " + builder.error();        
        QTest::qVerify(false, "build", m_error.toLocal8Bit().data(), __FILE__, __LINE__);
        return false;
    }        
    if (m_dump)
        qDebug().noquote() << m_program.dump();

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

void ScriptTest::onRuntimeError(JZNodeRuntimeError error)
{        
    qDebug().noquote() << m_program.dump();

    qDebug() << error.error;    
    Q_ASSERT(0);
}

void ScriptTest::initTestCase()
{
    QDir dir;
    if(!dir.exists(m_testPath))
        dir.mkdir(m_testPath);
}

void ScriptTest::init()
{
    m_project.clear();
    JZScriptFile *file = new JZScriptFile();
    file->setName("main.jz");
    m_project.addItem("./", file);

    JZScriptItem *main_flow = file->addFlow("main");
    JZNodeEvent *start = new JZNodeStartEvent();
    main_flow->addNode(JZNodePtr(start));

    file->addParamDefine("global");

    m_file = file;
    m_scriptFlow = m_project.mainScript();
    m_paramDef = m_project.globalDefine();
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

void ScriptTest::printCode()
{
    qDebug().noquote() << m_program.dump();
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
    JZNode *node_start = script->getNode(0);
    JZNodeWhile *node_while = new JZNodeWhile();
    JZNodeLiteral *node_true = new JZNodeLiteral();

    m_paramDef->addVariable("i",Type_int);
    
    script->addNode(JZNodePtr(node_while));
    script->addNode(JZNodePtr(node_true));

    node_true->setDataType(Type_bool);
    node_true->setLiteral(true);
    
    script->addConnect(JZNodeGemo(node_start->id(),node_start->flowOut()),JZNodeGemo(node_while->id(),node_while->flowIn()));
    script->addConnect(JZNodeGemo(node_true->id(),node_true->paramOut(0)),JZNodeGemo(node_while->id(),node_while->paramIn(0)));

    QList<JZNodeSetParam*> node_list;
    QMap<int,int> node_value;
    for(int i = 0; i < 100; i++)
    {
        JZNodeSetParam *node_set = new JZNodeSetParam();
        node_set->setVariable("i");
        node_set->setPinValue(node_set->paramIn(0),QString::number(i));
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

void ScriptTest::testMatchType()
{
    int ret = JZNodeType::matchType({ Type_bool,Type_double,Type_int }, QList<int>{Type_any});
    QVERIFY(ret == Type_double);
}

void ScriptTest::testRegExp()
{
    JZRegExpHelp help;
    QCOMPARE(help.isInt("235346"), true);
    QCOMPARE(help.isInt("-235346"), true);
    QCOMPARE(help.isInt("-2353xc46"), false);
    QCOMPARE(help.isInt("-235346xc"),false);

    QCOMPARE(help.isHex("235346"), false);
    QCOMPARE(help.isHex("0x235346"), true);
    QCOMPARE(help.isHex("0x2353ff46"), true);
    QCOMPARE(help.isHex("0x235346xc"), false);

    QCOMPARE(help.isFloat("23.5346"), true);
    QCOMPARE(help.isFloat("-235.346"), true);
    QCOMPARE(help.isFloat("235346"), false);
    QCOMPARE(help.isFloat("23534.6xc"), false);
}

void ScriptTest::testObjectParse()
{
    QList<JZNodeObjectPtr> cache;

    JZNodeObjectParser parser;
    auto obj_list = parser.parse("[1,2,3,4,5,6,7,8]");
    QVERIFY(obj_list && obj_list->type() == Type_list);
    cache << JZNodeObjectPtr(obj_list);

    auto obj_map = parser.parse(R"({"a":1,"b":998})");
    QVERIFY(obj_map && obj_map->type() == Type_map);
    cache << JZNodeObjectPtr(obj_map);

    obj_list = parser.parse("[ Point{1,2},Point{3,4},Point{5,6},Point{7,8}]");
    QVERIFY(obj_list && obj_list->type() == Type_list);
    cache << JZNodeObjectPtr(obj_list);
}

void ScriptTest::testParamBinding()
{
    /*
        c = a + b
    */
#if 0
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
#endif
}

void ScriptTest::testBranch()
{    
    JZNodeEngine *engine = &m_engine;

    FunctionDefine define;
    define.name = "testFunc";
    define.paramIn.push_back(JZParamDefine("a", Type_int));
    define.paramIn.push_back(JZParamDefine("b", Type_int));
    define.paramOut.push_back(JZParamDefine("c", Type_int));    

    auto script = m_file->addFunction("./", define);
    auto node_start = script->getNode(0);

    JZNodeBranch *branch = new JZNodeBranch();
    JZNodeEQ *eq = new JZNodeEQ();
    JZNodeParam *a = new JZNodeParam();
    a->setVariable("a");

    JZNodeParam *b = new JZNodeParam();
    b->setVariable("b");

    JZNodeReturn *r1 = new JZNodeReturn();
    r1->setFunction(&define);
    r1->setParamInValue(0, "1");
    
    JZNodeReturn *r2 = new JZNodeReturn();
    r2->setFunction(&define);
    r2->setParamInValue(0, "0");
    
    script->addNode(JZNodePtr(branch));
    script->addNode(JZNodePtr(eq));
    script->addNode(JZNodePtr(a));
    script->addNode(JZNodePtr(b));
    script->addNode(JZNodePtr(r1));
    script->addNode(JZNodePtr(r2));

    script->addConnect(a->paramOutGemo(0), eq->paramInGemo(0));
    script->addConnect(b->paramOutGemo(0), eq->paramInGemo(1));
    script->addConnect(eq->paramOutGemo(0), branch->paramInGemo(0));

    script->addConnect(node_start->flowOutGemo(0), branch->flowInGemo());
    script->addConnect(branch->flowOutGemo(0), r1->flowInGemo());
    script->addConnect(branch->flowOutGemo(1), r2->flowInGemo());

    if (!build())
        return;

    QVariantList in, out;
    in = { 0 ,1 };
    engine->call("testFunc", in, out);
    QCOMPARE(out[0].toInt(), 0);

    in = { 100 ,100 };
    engine->call("testFunc", in, out);
    QCOMPARE(out[0].toInt(), 1);
}

void ScriptTest::testIf()
{
    FunctionDefine define;
    define.name = "testFunc";
    define.paramIn.push_back(JZParamDefine("a", Type_int));    
    define.paramOut.push_back(JZParamDefine("ret", Type_int));

    auto script = m_file->addFunction("./", define);
    auto node_start = script->getNode(0);

    JZNodeIf *node_if = new JZNodeIf();
    for (int i = 0; i < 3; i++)
        node_if->addCondPin();    
    node_if->addElsePin();

    QCOMPARE(node_if->paramInCount(), 4);
    QCOMPARE(node_if->subFlowCount(), 5);
    
    JZNodeParam *a = new JZNodeParam();
    a->setVariable("a");
    script->addNode(JZNodePtr(node_if));
    script->addNode(JZNodePtr(a));
    script->addConnect(node_start->flowOutGemo(0), node_if->flowInGemo());

    for (int i = 0; i < 4; i++)
    {
        JZNodeEQ *eq = new JZNodeEQ();                
        script->addNode(JZNodePtr(eq));

        JZNodeReturn *ret = new JZNodeReturn();
        script->addNode(JZNodePtr(ret));

        ret->setFunction(&define);
        ret->setParamInValue(0, QString::number(i));

        script->addConnect(a->paramOutGemo(0), eq->paramInGemo(0));
        eq->setParamInValue(1, QString::number(i));

        script->addConnect(eq->paramOutGemo(0), node_if->paramInGemo(i));
        script->addConnect(node_if->subFlowOutGemo(i), ret->flowInGemo());
    }

    JZNodeReturn *ret_else = new JZNodeReturn();
    script->addNode(JZNodePtr(ret_else));
    ret_else->setFunction(&define);
    ret_else->setParamInValue(0, "-1");
    script->addConnect(node_if->subFlowOutGemo(4), ret_else->flowInGemo());

    if (!build())
        return;

    JZNodeEngine *engine = &m_engine;
    for (int i = 0; i < 5; i++)
    {
        QVariantList in, out;
        in = { i};
        engine->call("testFunc", in, out);

        if(i < 4)
            QCOMPARE(out[0].toInt(), i);
        else
            QCOMPARE(out[0].toInt(), -1);
    }
}

void ScriptTest::testSwitch()
{
    FunctionDefine define;
    define.name = "testFunc";
    define.paramIn.push_back(JZParamDefine("a", Type_int));
    define.paramOut.push_back(JZParamDefine("ret", Type_int));

    auto script = m_file->addFunction("./", define);
    auto node_start = script->getNode(0);

    JZNodeSwitch *node_switch = new JZNodeSwitch();
    while (node_switch->caseCount() < 4)
        node_switch->addCase();
    for (int i = 0; i < 4; i++)            
        node_switch->setCaseValue(i, QString::number(i));
    node_switch->addDefault();

    QCOMPARE(node_switch->paramInCount(), 1);
    QCOMPARE(node_switch->subFlowCount(), 5);

    JZNodeParam *a = new JZNodeParam();
    a->setVariable("a");
    script->addNode(JZNodePtr(node_switch));
    script->addNode(JZNodePtr(a));
    script->addConnect(node_start->flowOutGemo(0), node_switch->flowInGemo());

    script->addConnect(a->paramOutGemo(0), node_switch->paramInGemo(0));
    for (int i = 0; i < 4; i++)
    {
        JZNodeReturn *ret = new JZNodeReturn();
        script->addNode(JZNodePtr(ret));

        ret->setFunction(&define);
        ret->setParamInValue(0, QString::number(i));
        
        script->addConnect(node_switch->subFlowOutGemo(i), ret->flowInGemo());
    }

    JZNodeReturn *ret_else = new JZNodeReturn();
    script->addNode(JZNodePtr(ret_else));
    ret_else->setFunction(&define);
    ret_else->setParamInValue(0, "-1");
    script->addConnect(node_switch->subFlowOutGemo(4), ret_else->flowInGemo());

    if (!build())
        return;            

    JZNodeEngine *engine = &m_engine;
    for (int i = 0; i < 5; i++)
    {
        QVariantList in, out;
        in = { i };
        engine->call("testFunc", in, out);

        if (i < 4)
            QCOMPARE(out[0].toInt(), i);
        else
            QCOMPARE(out[0].toInt(), -1);
    }
}

void ScriptTest::testSequeue()
{
    /*
        a = 1 + 2
        b = 2 + 3
        c = 3 + 4
        d = 4 + 5
    */    
    JZScriptItem *script = m_scriptFlow;
    JZNodeEngine *engine = &m_engine;
    auto *paramDef = m_paramDef;

    JZNode *node_start = script->getNode(0);
    JZNodeSequence *node_seq = new JZNodeSequence();

    int start_id = node_start->id();
    int for_id = script->addNode(JZNodePtr(node_seq));

    paramDef->addVariable("a",Type_int);
    paramDef->addVariable("b",Type_int);
    paramDef->addVariable("c",Type_int);
    paramDef->addVariable("d",Type_int);

    node_seq->addSequeue();
    node_seq->addSequeue();
    node_seq->addSequeue();
    node_seq->addSequeue();
    
    script->addConnect(JZNodeGemo(start_id,node_start->flowOut()),JZNodeGemo(for_id,node_seq->flowIn()));
    for(int i = 0; i < 4; i++)
    {
        JZNodeSetParam *node_set = new JZNodeSetParam();
        node_set->setVariable(QString('a' + i));

        JZNodeAdd *node_add = new JZNodeAdd();
        node_add->setPinValue(node_add->paramIn(0), QString::number(i+1));
        node_add->setPinValue(node_add->paramIn(1), QString::number(i+2));

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
    for (int i = 0; i < 4; i++)
    {
        FunctionDefine define;
        define.name = "ForTest" + QString::number(i);        
        define.paramOut.push_back(JZParamDefine("result", Type_int));

        JZScriptItem *script = m_file->addFunction("./", define);
        script->addLocalVariable(JZParamDefine("sum", Type_int));

        JZNode *node_start = script->getNode(0);

        JZNodeFor *node_for = new JZNodeFor();
        JZNodeAdd *node_add = new JZNodeAdd();
        JZNodeParam *node_sum = new JZNodeParam();
        JZNodeSetParam *node_set = new JZNodeSetParam();
        JZNodeReturn *node_ret = new JZNodeReturn();

        int start_id = node_start->id();
        int for_id = script->addNode(JZNodePtr(node_for));
        int set_id = script->addNode(JZNodePtr(node_set));
        script->addNode(JZNodePtr(node_add));
        script->addNode(JZNodePtr(node_sum));
        script->addNode(JZNodePtr(node_ret));

        node_sum->setVariable("sum");
        node_set->setVariable("sum");
        node_ret->setFunction(&define);

        //start
        script->addConnect(JZNodeGemo(start_id, node_start->flowOut()), JZNodeGemo(for_id, node_for->flowIn()));
        if(i == 0)
            node_for->setRange(0, 1, 10);
        else if (i == 0)
            node_for->setRange(9, 1, -1);
        else if (i == 0)
            node_for->setRange(0, -1, 10);
        else
            node_for->setRange(9, -1, -1);        

        script->addConnect(node_for->subFlowOutGemo(0), node_set->flowInGemo());
        script->addConnect(node_sum->paramOutGemo(0), node_add->paramInGemo(0));
        script->addConnect(node_for->paramOutGemo(0), node_add->paramInGemo(1));
        script->addConnect(node_add->paramOutGemo(0), node_set->paramInGemo(0));

        script->addConnect(node_for->flowOutGemo(0), node_ret->flowInGemo());
        script->addConnect(node_sum->paramOutGemo(0), node_ret->paramInGemo(0));
    }
    
    if(!run())
        return;

    QVariantList in,out;
    m_engine.call("ForTest0", in, out);
    QCOMPARE(out[0].toInt(),45);

    m_engine.call("ForTest1", in, out);
    QCOMPARE(out[0].toInt(), 45);

    m_engine.call("ForTest2", in, out);
    QCOMPARE(out[0].toInt(), 45);

    m_engine.call("ForTest3", in, out);
    QCOMPARE(out[0].toInt(), 45);
}

void ScriptTest::testForEach()
{
    /*
        for(int i = 0; i < list.size(); i++)
        {
            sum = sum + i;
        }
    */    
    JZScriptItem *script = m_scriptFlow;
    JZNodeEngine *engine = &m_engine;
    JZParamItem *paramDef = m_paramDef;
    paramDef->addVariable("sum",Type_int,0);
    paramDef->addVariable("a",JZNodeObjectManager::instance()->getClassId("List"));

    JZNode *node_start = script->getNode(0);
    JZNodeForEach *node_for = new JZNodeForEach();        
    JZNodeAdd *node_add = new JZNodeAdd();

    JZNodeParam *node_sum = new JZNodeParam();
    JZNodeSetParam *node_set = new JZNodeSetParam();

    JZNodeSetParam *node_list = new JZNodeSetParam();
    node_list->setVariable("a");
    node_list->setParamInValue(0, 0);

    node_sum->setVariable("sum");
    node_set->setVariable("sum");

    JZNodeFunction *node_create = new JZNodeFunction();
    node_create->setFunction(JZNodeFunctionManager::instance()->function("List.createFromString"));
    node_create->setParamInValue(0, "1,2,3,4,5,6,7,8,9,10");

    int start_id = node_start->id();
    script->addNode(JZNodePtr(node_for));    
    script->addNode(JZNodePtr(node_sum));
    script->addNode(JZNodePtr(node_add));
    script->addNode(JZNodePtr(node_set));
    script->addNode(JZNodePtr(node_create));
    script->addNode(JZNodePtr(node_list));

    //start    
    script->addConnect(JZNodeGemo(start_id,node_start->flowOut()), node_list->flowInGemo());   
    script->addConnect(node_create->paramOutGemo(0), node_list->paramInGemo(0));

    script->addConnect(node_list->flowOutGemo(),node_for->flowInGemo());
    script->addConnect(node_list->paramOutGemo(0),node_for->paramInGemo(0));

    // sum = sum + i
    script->addConnect(node_sum->paramOutGemo(0),node_add->paramInGemo(0));
    script->addConnect(node_for->paramOutGemo(1),node_add->paramInGemo(1));

    script->addConnect(node_add->paramOutGemo(0),node_set->paramInGemo(0));
    script->addConnect(node_for->subFlowOutGemo(0),node_set->flowInGemo());
    
    if (!build())
        return;
        
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
    JZScriptItem *script = m_scriptFlow;
    script->addLocalVariable(JZParamDefine("i", Type_int));

    JZNode *node_start = script->getNode(0);
    JZNodeSetParam *node_set = new JZNodeSetParam();
    JZNodeParam *node_param = new JZNodeParam();
    JZNodeWhile *node_while = new JZNodeWhile();    
    JZNodeLiteral *node_value = new JZNodeLiteral();
    JZNodeLiteral *node_value10 = new JZNodeLiteral();
    JZNodeAdd *node_add = new JZNodeAdd();    
    JZNodeNE *node_eq = new JZNodeNE();        
    
    int start_id = node_start->id();
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

    node_value->setDataType(Type_int);
    node_value->setLiteral(1);

    node_value10->setDataType(Type_int);
    node_value10->setLiteral(10);

    //start    
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
    JZScriptItem *script = m_scriptFlow;
    JZNodeEngine *engine = &m_engine;
    QMap<int,int> node_value = initWhileSetCase();
    if(!run(true))
        return;

    QThread::msleep(50);
    stop();
}

void ScriptTest::testDebugServer()
{           
    JZScriptItem *script = m_scriptFlow;
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
/*
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
*/
    server.stopServer();
    client.stop();    
}

void ScriptTest::testExpr()
{    
    JZScriptItem *script = m_scriptFlow;
    JZNodeEngine *engine = &m_engine;
    JZParamItem *paramDef = m_paramDef;

    QVector<int> op_type = {Node_add,Node_sub,Node_mul,Node_div,Node_mod,Node_eq,Node_ne,Node_le,
        Node_ge,Node_lt,Node_gt,Node_and,Node_or,Node_bitand,Node_bitor,Node_bitxor};

    JZNode *node_start = script->getNode(0);      
    JZNodeParam *node_a = new JZNodeParam();
    JZNodeParam *node_b = new JZNodeParam();

    int a = 100,b = 50;    
    paramDef->addVariable("a",Type_int, QString::number(a));
    paramDef->addVariable("b",Type_int, QString::number(b));

    node_a->setVariable("a");
    node_b->setVariable("b");

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
    JZScriptItem *script = m_scriptFlow;
    JZNodeEngine *engine = &m_engine;
    JZParamItem *paramDef = m_paramDef;

    JZNode *node_start = script->getNode(0);
    JZNodeParam *node_a = new JZNodeParam();
    JZNodeParam *node_b = new JZNodeParam();
    JZNodeExpression *node_expr = new JZNodeExpression();
    
    paramDef->addVariable("a",Type_int,"2");
    paramDef->addVariable("b",Type_int,"3");
    paramDef->addVariable("c",Type_int,"0");

    node_a->setVariable("a");
    node_b->setVariable("b");

    node_start->id();
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
    QVERIFY(c.toInt() == pow(a,b) - pow(b,a));
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

    FunctionDefine fab;
    fab.name = "fab";
    fab.paramIn.push_back(JZParamDefine("n", Type_int));
    fab.paramOut.push_back(JZParamDefine("result", Type_int));
    JZScriptItem *script = m_file->addFunction("./",fab);

    JZNode *node_start = script->getNode(0);
    JZNodeBranch *node_branch = new JZNodeBranch();    
    JZNodeGE *node_ge = new JZNodeGE();
    node_ge->setPinValue(node_ge->paramIn(1), "2");
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
    ret1->setFunction(&fab);
    ret2->setFunction(&fab);

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
    sub1->setPinValue(sub1->paramIn(1),"1");
    sub2->setPinValue(sub2->paramIn(1),"2");
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
    JZParamItem *paramDef = m_paramDef;
/*
    auto classBase = project->addClass("./", "ClassBase");
    project->addClass("ClassA","ClassBase");
    project->addClass("ClassB","ClassBase");

    classBase->addMemberVariable("a",Type_int,50);
    paramDef->addVariable("a",JZNodeObjectManager::instance()->getClassId("ClassA"));
    paramDef->addVariable("b",JZNodeObjectManager::instance()->getClassId("ClassB"));

    if(!run())
        return;

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
}

void ScriptTest::testCClass()
{        
    JZNodeEngine *engine = &m_engine;
    JZParamItem *paramDef = m_paramDef;

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
    engine->call("String.left",{va,6},out);
    QCOMPARE(out[0],a.left(6));

    engine->call("String.size",{out[0]},out);
    QCOMPARE(out[0],a.left(6).size());

    engine->call("String.replace",{va,vb,vc},out);
    QCOMPARE(out[0],a.replace(b,c));
}

void test_script()
{    
    ScriptTest s; 
    QTest::qExec(&s);
}
