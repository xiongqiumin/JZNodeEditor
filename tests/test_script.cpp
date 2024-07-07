#include <QEventLoop>
#include <QDebug>
#include <math.h>
#include <QApplication>
#include <QTest>
#include "JZNodeFactory.h"
#include "test_script.h"
#include "JZRegExpHelp.h"
#include "JZNodeObjectParser.h"
#include "JZNodeBind.h"
#include "JZNodeValue.h"
#include "JZNodeFunction.h"
#include "JZNodeExpression.h"
#include "JZNodeDebugServer.h"
#include "JZNodeDebugClient.h"
#include "JZNodeBind.h"

ScriptTest::ScriptTest()
{
}

void ScriptTest::testMatchType()
{
    int ret = JZNodeType::matchType({ Type_bool,Type_double,Type_int }, QList<int>{Type_bool});
    QVERIFY(ret == Type_bool);

    QVariant vb = true;
    QVariant vi = 1999;
    QVariant vd = 8.34;
    QVariant vs = "true";
    QVariant vnull = QVariant::fromValue(JZObjectNull());
    QVariant vfunc = QVariant::fromValue(JZFunctionPointer());
    QVariant vany = QVariant::fromValue(JZNodeVariantAny());

    QCOMPARE(JZNodeType::variantType(vb),Type_bool);
    QCOMPARE(JZNodeType::variantType(vi),Type_int);
    QCOMPARE(JZNodeType::variantType(vd),Type_double);
    QCOMPARE(JZNodeType::variantType(vs),Type_string);
    QCOMPARE(JZNodeType::variantType(vnull),Type_nullptr);
    QCOMPARE(JZNodeType::variantType(vfunc),Type_function);
    QCOMPARE(JZNodeType::variantType(vany),Type_any);
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

void ScriptTest::testContainer()
{
    m_project.addGlobalVariable("list_int","List<int>","1,2,3,4,5,6,7,8");
    m_project.addGlobalVariable("list_double","List<double>","1,2,3,4,5,6,7,8");
    m_project.addGlobalVariable("map_int_int","Map<int,int>","1:19,2:4");
    m_project.addGlobalVariable("map_int_string","Map<int,string>",R"(1:"hello",2:"57575")");
    m_project.addGlobalVariable("map_string_int","Map<string,int>",R"("a":200,"b":400)");
    m_project.addGlobalVariable("map_string_string","Map<string,string>",R"("a":"aa","b":"bbb")");

    if(!build())
        return;

    QVariantList in,out;
    auto list_int = m_engine.getVariable("list_int");
    in << list_int << 0;
    m_engine.call("List<int>.get",in,out);
    QCOMPARE(out[0].toInt(),1);

    in.clear();
    in << list_int << 0 << 5;
    m_engine.call("List<int>.set",in,out);

    in.clear();
    in << list_int << 0;
    m_engine.call("List<int>.get",in,out);
    QCOMPARE(out[0].toInt(),5);

    in.clear();
    in << list_int << 500;
    m_engine.call("List<int>.push_back",in,out);  

    in.clear();
    in << list_int;
    m_engine.call("List<int>.size",in,out);
    QCOMPARE(out[0].toInt(),9);

    m_engine.call("List<int>.clear",in,out);
    
    m_engine.call("List<int>.size",in,out);
    QCOMPARE(out[0].toInt(),0);
}

void ScriptTest::testObjectParse()
{
    if (!build())
        return;

    QList<JZNodeObjectPtr> cache;

    JZNodeObjectParser parser;
    auto obj_list = parser.parse("[1,2,3,4,5,6,7,8]");
    QVERIFY2(obj_list, qUtf8Printable(parser.error()));
    cache << JZNodeObjectPtr(obj_list,true);

    auto obj_map = parser.parse(R"({"a":1,"b":998})");
    QVERIFY2(obj_map, qUtf8Printable(parser.error()));
    cache << JZNodeObjectPtr(obj_map,true);

    obj_list = parser.parse("[ Point{1,2},Point{3,4},Point{5,6},Point{7,8}]");
    QVERIFY2(obj_list, qUtf8Printable(parser.error()));
    cache << JZNodeObjectPtr(obj_list,true);
}

void ScriptTest::testParamBinding()
{
    /*
        c = a + b
    */
#if 0
    auto script = m_project.mainFunction();
    m_paramDef->addLocalVariable("a",Type_int,10);
    m_paramDef->addLocalVariable("b",Type_int,20);
    m_paramDef->addLocalVariable("c",Type_int);

    JZNodeSetParamDataFlow *node_c = new JZNodeSetParamDataFlow();
    JZNodeAdd *node_add = new JZNodeAdd();
    JZNodeParam *node_a = new JZNodeParam();
    JZNodeParam *node_b = new JZNodeParam();

    script->addNode(node_a));
    script->addNode(node_b));
    script->addNode(node_c));
    script->addNode(node_add));

    node_a->setVariable("a");
    node_b->setVariable("b");
    node_c->setVariable("c");
    script->addConnect(node_a->paramOutGemo(0),node_add->paramInGemo(0));
    script->addConnect(node_b->paramOutGemo(0),node_add->paramInGemo(1));

    script->addConnect(node_add->paramOutGemo(0),node_c->paramInGemo(0));

    if(!build())
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

    JZFunctionDefine define;
    define.name = "testFunc";
    define.paramIn.push_back(JZParamDefine("a", Type_int));
    define.paramIn.push_back(JZParamDefine("b", Type_int));
    define.paramOut.push_back(JZParamDefine("c", Type_int));    

    auto script = m_file->addFunction(define);
    auto node_start = script->getNode(0);

    JZNodeBranch *branch = new JZNodeBranch();
    JZNodeEQ *eq = new JZNodeEQ();
    JZNodeParam *a = new JZNodeParam();
    a->setVariable("a");

    JZNodeParam *b = new JZNodeParam();
    b->setVariable("b");

    JZNodeReturn *r1 = new JZNodeReturn();
    JZNodeReturn *r2 = new JZNodeReturn();
    
    script->addNode(branch);
    script->addNode(eq);
    script->addNode(a);
    script->addNode(b);
    script->addNode(r1);
    script->addNode(r2);

    r1->setParamInValue(0, "1");
    r2->setParamInValue(0, "0");

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
    JZFunctionDefine define;
    define.name = "testFunc";
    define.paramIn.push_back(JZParamDefine("a", Type_int));    
    define.paramOut.push_back(JZParamDefine("ret", Type_int));

    auto script = m_file->addFunction(define);
    auto node_start = script->getNode(0);

    JZNodeIf *node_if = new JZNodeIf();
    for (int i = 0; i < 3; i++)
        node_if->addCondPin();    
    node_if->addElsePin();

    QCOMPARE(node_if->paramInCount(), 4);
    QCOMPARE(node_if->subFlowCount(), 5);
    
    JZNodeParam *a = new JZNodeParam();
    a->setVariable("a");
    script->addNode(node_if);
    script->addNode(a);
    script->addConnect(node_start->flowOutGemo(0), node_if->flowInGemo());

    for (int i = 0; i < 4; i++)
    {
        JZNodeEQ *eq = new JZNodeEQ();                
        script->addNode(eq);

        JZNodeReturn *ret = new JZNodeReturn();
        script->addNode(ret);

        ret->setParamInValue(0, QString::number(i));

        script->addConnect(a->paramOutGemo(0), eq->paramInGemo(0));
        eq->setParamInValue(1, QString::number(i));

        script->addConnect(eq->paramOutGemo(0), node_if->paramInGemo(i));
        script->addConnect(node_if->subFlowOutGemo(i), ret->flowInGemo());
    }

    JZNodeReturn *ret_else = new JZNodeReturn();
    script->addNode(ret_else);
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
    JZFunctionDefine define;
    define.name = "testFunc";
    define.paramIn.push_back(JZParamDefine("a", Type_int));
    define.paramOut.push_back(JZParamDefine("ret", Type_int));

    auto script = m_file->addFunction(define);
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
    script->addNode(node_switch);
    script->addNode(a);
    script->addConnect(node_start->flowOutGemo(0), node_switch->flowInGemo());

    script->addConnect(a->paramOutGemo(0), node_switch->paramInGemo(0));
    for (int i = 0; i < 4; i++)
    {
        JZNodeReturn *ret = new JZNodeReturn();
        script->addNode(ret);

        ret->setParamInValue(0, QString::number(i));
        
        script->addConnect(node_switch->subFlowOutGemo(i), ret->flowInGemo());
    }

    JZNodeReturn *ret_else = new JZNodeReturn();
    script->addNode(ret_else);
    ret_else->setParamInValue(0, "-1");
    script->addConnect(node_switch->subFlowOutGemo(4), ret_else->flowInGemo());

    if (!build())
        return;  
    dumpAsm("testSwitch.jsm");          

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
    JZScriptItem *script = m_project.mainFunction();
    JZNodeEngine *engine = &m_engine;

    JZNode *node_start = script->getNode(0);
    JZNodeSequence *node_seq = new JZNodeSequence();

    int start_id = node_start->id();
    int for_id = script->addNode(node_seq);
    m_project.addGlobalVariable("a",Type_int);
    m_project.addGlobalVariable("b",Type_int);
    m_project.addGlobalVariable("c",Type_int);
    m_project.addGlobalVariable("d",Type_int);

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

        script->addNode(node_set);
        script->addNode(node_add);

        script->addConnect(node_seq->subFlowOutGemo(i),node_set->flowInGemo());
        script->addConnect(node_add->paramOutGemo(0),node_set->paramInGemo(1));
    }
    if(!build())
        return;
    QVariantList in,out;
    call("main",in,out);

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
        JZFunctionDefine define;
        define.name = "ForTest" + QString::number(i);        
        define.paramOut.push_back(JZParamDefine("result", Type_int));

        JZScriptItem *script = m_file->addFunction(define);
        script->addLocalVariable(JZParamDefine("sum", Type_int));

        JZNode *node_start = script->getNode(0);

        JZNodeFor *node_for = new JZNodeFor();
        JZNodeAdd *node_add = new JZNodeAdd();
        JZNodeParam *node_sum = new JZNodeParam();
        JZNodeSetParam *node_set = new JZNodeSetParam();
        JZNodeReturn *node_ret = new JZNodeReturn();

        int start_id = node_start->id();
        int for_id = script->addNode(node_for);
        int set_id = script->addNode(node_set);
        script->addNode(node_add);
        script->addNode(node_sum);
        script->addNode(node_ret);

        node_sum->setVariable("sum");
        node_set->setVariable("sum");

        //start
        script->addConnect(JZNodeGemo(start_id, node_start->flowOut()), JZNodeGemo(for_id, node_for->flowIn()));
        if (i == 0)
        {
            node_for->setRange(0, 1, 10);
            node_for->setOp(OP_lt);
        }
        else if (i == 1)
        {
            node_for->setRange(9, -1, -1);
            node_for->setOp(OP_gt);
        }
        else if (i == 2)
        {
            node_for->setRange(0, 1, 9);
            node_for->setOp(OP_le);
        }
        else
        {
            node_for->setRange(9, -1, 0);
            node_for->setOp(OP_ge);
        }

        script->addConnect(node_for->subFlowOutGemo(0), node_set->flowInGemo());
        script->addConnect(node_sum->paramOutGemo(0), node_add->paramInGemo(0));
        script->addConnect(node_for->paramOutGemo(0), node_add->paramInGemo(1));
        script->addConnect(node_add->paramOutGemo(0), node_set->paramInGemo(1));

        script->addConnect(node_for->flowOutGemo(0), node_ret->flowInGemo());
        script->addConnect(node_sum->paramOutGemo(0), node_ret->paramInGemo(0));
    }
    
    if(!build())
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
    return;
    /*
        for(int i = 0; i < list.size(); i++)
        {
            sum = sum + i;
        }
    */    
    JZScriptItem *script = m_project.mainFunction();
    JZNodeEngine *engine = &m_engine;

    m_project.addGlobalVariable("sum",Type_int,0);
    m_project.addGlobalVariable("a",JZNodeObjectManager::instance()->getClassId("List"));

    JZNode *node_start = script->getNode(0);
    JZNodeForEach *node_for = new JZNodeForEach();        
    JZNodeAdd *node_add = new JZNodeAdd();

    JZNodeParam *node_sum = new JZNodeParam();
    JZNodeSetParam *node_set = new JZNodeSetParam();

    JZNodeSetParam *node_list = new JZNodeSetParam();
    node_list->setVariable("a");

    node_sum->setVariable("sum");
    node_set->setVariable("sum");

    JZNodeFunction *node_create = new JZNodeFunction();
    node_create->setFunction(JZNodeFunctionManager::instance()->function("List.__fromString__"));
    node_create->setParamInValue(0, "\"1,2,3,4,5,6,7,8,9,10\"");

    int start_id = node_start->id();
    script->addNode(node_for);    
    script->addNode(node_sum);
    script->addNode(node_add);
    script->addNode(node_set);
    script->addNode(node_create);
    script->addNode(node_list);

    //start    
    script->addConnect(JZNodeGemo(start_id,node_start->flowOut()), node_list->flowInGemo());   
    script->addConnect(node_create->paramOutGemo(0), node_list->paramInGemo(1));

    script->addConnect(node_list->flowOutGemo(),node_for->flowInGemo());
    script->addConnect(node_list->paramOutGemo(0),node_for->paramInGemo(0));

    // sum = sum + i
    script->addConnect(node_sum->paramOutGemo(0),node_add->paramInGemo(0));
    script->addConnect(node_for->paramOutGemo(1),node_add->paramInGemo(1));

    script->addConnect(node_add->paramOutGemo(0),node_set->paramInGemo(1));
    script->addConnect(node_for->subFlowOutGemo(0),node_set->flowInGemo());
    
    if (!build())
        return;
        
    QVariantList in,out;
    call("main",in,out);

    QVariant sum = engine->getVariable("sum");
    QCOMPARE(sum,55);
}

void ScriptTest::testWhileLoop()
{
    /*
        while(i <= 10)   
            i = i + 1;
    */
    JZScriptItem *script = m_project.mainFunction();
    m_project.addGlobalVariable("i", Type_int);
    m_project.addGlobalVariable("time", Type_int);

    JZNode *node_start = script->getNode(0);
    JZNodeSetParam *node_set = new JZNodeSetParam();
    JZNodeParam *node_param = new JZNodeParam();
    JZNodeWhile *node_while = new JZNodeWhile();    
    JZNodeLiteral *node_value = new JZNodeLiteral();
    JZNodeLiteral *node_value10 = new JZNodeLiteral();
    JZNodeAdd *node_add = new JZNodeAdd();    
    JZNodeLE *node_le = new JZNodeLE();        
    
    int start_id = node_start->id();
    int param_id = script->addNode(node_param);    
    int set_id = script->addNode(node_set);
    int while_id = script->addNode(node_while);
    int value_id = script->addNode(node_value);
    int add_id = script->addNode(node_add);    
    script->addNode(node_le);
    int value10_id = script->addNode(node_value10);

    node_param->setVariable("i");
    node_set->setVariable("i");

    node_value->setDataType(Type_int);
    node_value->setLiteral("1");

    node_value10->setDataType(Type_int);
    node_value10->setLiteral("10");

    //start    
    script->addConnect(JZNodeGemo(start_id,node_start->flowOut()),JZNodeGemo(while_id,node_while->flowIn()));

    // param < 10    
    script->addConnect(node_param->paramOutGemo(0),node_le->paramInGemo(0));
    script->addConnect(JZNodeGemo(value10_id,node_param->paramOut(0)),node_le->paramInGemo(1));

    // while
    script->addConnect(node_le->paramOutGemo(0),JZNodeGemo(while_id,node_while->paramIn(0)));    
    script->addConnect(JZNodeGemo(while_id,node_while->subFlowOut(0)),JZNodeGemo(set_id,node_set->flowIn()));

    // i = i + 1
    script->addConnect(JZNodeGemo(param_id,node_param->paramOut(0)),JZNodeGemo(add_id,node_add->paramIn(0)));
    script->addConnect(JZNodeGemo(value_id,node_value->paramOut(0)),JZNodeGemo(add_id,node_add->paramIn(1)));
        
    script->addConnect(JZNodeGemo(add_id,node_add->paramOut(0)),JZNodeGemo(set_id,node_set->paramIn(1)));
        
    if(!build())
        return;
    dumpAsm("testWhileLoop.jsm");

    QVariantList in,out;
    call("main",in,out);

    int sum = m_engine.getVariable("i").toInt();
    QCOMPARE(sum,11);
}

bool ScriptTest::initWhileCase(QList<int> &id_list,QList<int> &value_list)
{
    auto script = m_project.mainFunction();
    JZNode *node_start = script->getNode(0);
    JZNodeWhile *node_while = new JZNodeWhile();
    JZNodeLiteral *node_true = new JZNodeLiteral();

    script->addLocalVariable("i",Type_int);
    
    script->addNode(node_while);
    script->addNode(node_true);

    node_true->setDataType(Type_bool);
    node_true->setLiteral("true");
    
    script->addConnect(JZNodeGemo(node_start->id(),node_start->flowOut()),JZNodeGemo(node_while->id(),node_while->flowIn()));
    script->addConnect(JZNodeGemo(node_true->id(),node_true->paramOut(0)),JZNodeGemo(node_while->id(),node_while->paramIn(0)));

    QList<JZNodeSetParam*> node_list;
    for(int i = 0; i < 100; i++)
    {
        JZNodeSetParam *node_set = new JZNodeSetParam();
        node_set->setVariable("i");
        node_set->setPinValue(node_set->paramIn(1),QString::number(i));
        script->addNode(node_set);

        id_list.push_back(node_set->id());
        value_list.push_back(i);
        if(i == 0)
            script->addConnect(JZNodeGemo(node_while->id(),node_while->subFlowOut(0)),JZNodeGemo(node_set->id(),node_set->flowIn()));
        else
        {
            JZNodeSetParam *pre_node = node_list.back();
            script->addConnect(JZNodeGemo(pre_node->id(),pre_node->flowOut()),JZNodeGemo(node_set->id(),node_set->flowIn()));
        }
        node_list.push_back(node_set);
    }

    return build();
}

void ScriptTest::testBreakPoint()
{    
    QList<int> id_list,value_list;
    if(!initWhileCase(id_list,value_list))
        return;
    
    QVariantList in;
    callAsync("main",in);
    msleep(200);

    QString main_function = m_project.mainFunctionPath();
    for(int i = 0; i < 5; i++)
    {
        int cur_id = rand()%id_list.size();

        m_engine.addBreakPoint(main_function, id_list[cur_id]);
        msleep(50);
        QVERIFY(m_engine.status() == Status_pause);

        m_engine.stepOver();
        msleep(20);

        int value = m_engine.getVariable("i").toInt();
        QCOMPARE(value, value_list[cur_id]);

        m_engine.removeBreakPoint(main_function, id_list[cur_id]);
        m_engine.resume();
    }

    m_engine.stop();
}

void ScriptTest::testDebugServer()
{           
    QList<int> id_list,value_list;
    if(!initWhileCase(id_list,value_list))
        return;

    bool cmd_ret;
    QVariantList in;
    callAsync("main",in);

    JZNodeDebugServer server;
    JZNodeDebugClient client;
    server.setEngine(&m_engine);
    if(!server.startServer(18888))
    {
        QVERIFY2(false,"start server failded");
    }
    if(!client.connectToServer("127.0.0.1",18888))
    {
        QVERIFY2(false,"connect to server failded");
    }
    QThread::msleep(100);

    JZNodeDebugInfo init_info;
    JZNodeProgramInfo ret;
    cmd_ret = client.init(init_info,ret);
    QVERIFY(cmd_ret);

    cmd_ret = server.waitForAttach(500); 
    QVERIFY(cmd_ret);

    QString main_function = m_project.mainFunctionPath();
    for(int i = 0; i < 5; i++)
    {
        int cur_id = rand()%id_list.size();

        client.addBreakPoint(main_function, id_list[cur_id]);
        msleep(50);

        JZNodeRuntimeInfo runtime;
        cmd_ret = client.runtimeInfo(runtime);
        QVERIFY(cmd_ret);
        QCOMPARE(runtime.status,Status_pause);

        client.stepOver();
        msleep(10);

        JZNodeDebugParamInfo info,ret;
        JZNodeParamCoor coor;
        coor.type = JZNodeParamCoor::Name;
        coor.name = "i";
        info.coors << coor;
        
        cmd_ret = client.getVariable(info,ret);
        QVERIFY(cmd_ret);

        int value = ret.values[0].value.toInt();
        QCOMPARE(value, value_list[cur_id]);

        client.removeBreakPoint(main_function, id_list[cur_id]);
        client.resume();
    }

    client.stop(); 
    server.stopServer();
}

void ScriptTest::testExpr()
{    
    JZScriptItem *script = m_project.mainFunction();
    JZNodeEngine *engine = &m_engine;

    QVector<int> op_type = {Node_add,Node_sub,Node_mul,Node_div,Node_mod,Node_eq,Node_ne,Node_le,
        Node_ge,Node_lt,Node_gt,Node_and,Node_or,Node_bitand,Node_bitor,Node_bitxor};

    JZNode *node_start = script->getNode(0);      
    JZNodeParam *node_a = new JZNodeParam();
    JZNodeParam *node_b = new JZNodeParam();

    int a = 100,b = 50;    
    m_project.addGlobalVariable("a",Type_int, QString::number(a));
    m_project.addGlobalVariable("b",Type_int, QString::number(b));

    node_a->setVariable("a");
    node_b->setVariable("b");

    script->addNode(node_a);
    script->addNode(node_b);    

    JZNodeSetParam *pre_node = nullptr;
    for(int i = 0; i < op_type.size(); i++)
    {
        m_project.addGlobalVariable("i" + QString::number(i),Type_int);

        JZNodeSetParam *node_set = new JZNodeSetParam();
        node_set->setVariable("i" + QString::number(i));
        script->addNode(node_set);

        auto node_op = JZNodeFactory::instance()->createNode(op_type[i]);
        script->addNode(node_op);

        script->addConnect(node_a->paramOutGemo(0),node_op->paramInGemo(0));
        script->addConnect(node_b->paramOutGemo(0),node_op->paramInGemo(1));
        script->addConnect(node_op->paramOutGemo(0),node_set->paramInGemo(1));

        if(i == 0)
            script->addConnect(node_start->flowOutGemo(),node_set->flowInGemo());
        else       
            script->addConnect(pre_node->flowOutGemo(),node_set->flowInGemo());

        pre_node = node_set;
    }

    if(!build())
        return;

    QVariantList in,out;
    call("main",in,out);

    QList<int> ret;
    for(int i = 0; i < op_type.size(); i++) 
        ret << engine->getVariable("i" + QString::number(i)).toInt();

    QCOMPARE(ret[0],a + b); //Node_add
    QCOMPARE(ret[1],a - b); //Node_sub
    QCOMPARE(ret[2],a * b); //Node_mul
    QCOMPARE(ret[3],a / b); //Node_div
    QCOMPARE(ret[4],a % b); //Node_mod
    QCOMPARE((bool)ret[5],a == b); //Node_eq
    QCOMPARE((bool)ret[6],a != b); //Node_ne
    QCOMPARE((bool)ret[7],a <= b); //Node_le
    QCOMPARE((bool)ret[8],a >= b); //Node_ge
    QCOMPARE((bool)ret[9],a < b);  //Node_lt
    QCOMPARE((bool)ret[10],a > b); //Node_gt
    QCOMPARE((bool)ret[11],a && b); //Node_and
    QCOMPARE((bool)ret[12],a || b); //Node_or
    QCOMPARE(ret[13],a & b); //Node_bitand
    QCOMPARE(ret[14],a | b); //Node_bitor
    QCOMPARE(ret[15],a ^ b); //Node_bitxor
}

void ScriptTest::testCustomExpr()
{    
    JZScriptItem *script = m_project.mainFunction();
    JZNodeEngine *engine = &m_engine;

    JZNode *node_start = script->getNode(0);
    JZNodeParam *node_a = new JZNodeParam();
    JZNodeParam *node_b = new JZNodeParam();
    JZNodeExpression *node_expr = new JZNodeExpression();
    
    m_project.addGlobalVariable("a",Type_int,"2");
    m_project.addGlobalVariable("b",Type_int,"3");
    m_project.addGlobalVariable("c",Type_int,"0");

    node_a->setVariable("a");
    node_b->setVariable("b");

    node_start->id();
    script->addNode(node_a);
    script->addNode(node_b);

    QString error;
    if(!node_expr->setExpr("c = pow(a,b) - pow(b,a)",error))
    {
        QVERIFY2(false,qUtf8Printable(error));
        return;
    }
    script->addNode(node_expr);

    JZNodeSetParam *node_set = new JZNodeSetParam();
    node_set->setVariable("c");
    script->addNode(node_set);

    script->addConnect(node_a->paramOutGemo(0),node_expr->paramInGemo(0));
    script->addConnect(node_b->paramOutGemo(0),node_expr->paramInGemo(1));
    script->addConnect(node_expr->paramOutGemo(0),node_set->paramInGemo(1));

    script->addConnect(node_start->flowOutGemo(),node_set->flowInGemo());

    if(!build())
        return;

    QVariantList in,out;
    call("main",in,out);

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

    JZFunctionDefine fab;
    fab.name = "fab";
    fab.isFlowFunction = false;
    fab.paramIn.push_back(JZParamDefine("n", Type_int));
    fab.paramOut.push_back(JZParamDefine("result", Type_int));
    JZScriptItem *script = m_file->addFunction(fab);

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

    script->addNode(node_branch);
    script->addNode(node_ge);
    script->addNode(func1);
    script->addNode(func2);
    script->addNode(sub1);
    script->addNode(sub2);
    script->addNode(n);
    script->addNode(ret1);
    script->addNode(ret2);
    script->addNode(node_add);        

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

    if(!build())
        return;    
    dumpAsm("fab.jsm");

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

    auto classBase = m_file->addClass("ClassBase");
    auto classA = m_file->addClass("ClassA","ClassBase");
    auto classB = m_file->addClass("ClassB","ClassBase");

    auto addReturn = [](JZScriptItem *script,int num) {
        auto node_start = script->getNode(0);
        JZNodeReturn *node_ret = new JZNodeReturn();
        script->addNode(node_ret);
        node_ret->setParamInValue(0, QString::number(num));
        script->addConnect(node_start->flowOutGemo(),node_ret->flowInGemo());
    };

    JZFunctionDefine define;
    define.className = classBase->className();
    define.isVirtualFunction = true;
    define.name = "getValue";
    define.paramIn.push_back(JZParamDefine("this", "classBase"));
    define.paramOut.push_back(JZParamDefine("ret",Type_int));
    auto func_base = classBase->addMemberFunction(define);
    addReturn(func_base, 0);

    define.className = classA->className();
    auto func_a = classA->addMemberFunction(define);
    addReturn(func_a, 1);

    define.className = classB->className();
    auto func_b = classB->addMemberFunction(define);
    addReturn(func_b, 2);

    if(!build())
        return;

    auto inst = JZNodeObjectManager::instance();
    auto obj_base = JZNodeObjectPtr(inst->create("ClassBase"),true);
    auto obj_a = JZNodeObjectPtr(inst->create("ClassA"), true);
    auto obj_b = JZNodeObjectPtr(inst->create("ClassB"), true);

    QVariantList in, out;
    in.clear();
    in << QVariant::fromValue(obj_base);
    m_engine.call("ClassBase.getValue", in, out);
    QCOMPARE(out[0].toInt(),0);

    in.clear();
    in << QVariant::fromValue(obj_a);
    m_engine.call("ClassBase.getValue", in, out);
    QCOMPARE(out[0].toInt(), 1);

    in.clear();
    in << QVariant::fromValue(obj_b);
    m_engine.call("ClassBase.getValue", in, out);
    QCOMPARE(out[0].toInt(), 2);
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
    if(!build())
        return;

    QString a = "i'love jznode";
    QString b = "jznode";
    QString c = "money";
    
    QVariantList out;
    engine->call("string.left",{a,6},out);
    QCOMPARE(out[0],a.left(6));

    engine->call("string.size",{out[0]},out);
    QCOMPARE(out[0],a.left(6).size());

    engine->call("string.replace",{a,b,c},out);
    QCOMPARE(out[0],a.replace(b,c));
}

void test_script(int argc, char *argv[])
{    
    ScriptTest s; 
    QTest::qExec(&s,argc,argv);
}
