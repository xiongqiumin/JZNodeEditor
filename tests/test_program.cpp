#include <QEventLoop>
#include <QDebug>
#include "JZProject.h"
#include "JZNodeCompiler.h"
#include "JZNodeEngine.h"
#include "JZNodeValue.h"
#include "JZNodeProgram.h"
#include "JZNodeBuilder.h"
#include "JZNodeDebugServer.h"
#include "JZNodeDebugClient.h"
#include "JZNodeFunction.h"
#include "JZNodeBind.h"
#include "JZNodeFactory.h"
#include "JZNodeObject.h"
#include "JZNodeBind.h"
#include "JZParamFile.h"
#include <math.h>

class ScriptTest
{
public:    
    ScriptTest()
    {        
        project.initConsole();
        scriptFlow = (JZScriptFile*)project.getItem(project.mainScript());
        paramDef = (JZParamFile*)project.getItem("./param.def");

        scriptParam = new JZScriptFile(ProjectItem_scriptParamBinding);
        scriptParam->setName("param.jz");
        project.addItem("./",scriptParam);
        Q_ASSERT(scriptFlow && scriptParam);
    }        

    void call()
    {
        JZEvent event;
        event.eventType = Event_programStart;
        engine.dealEvent(&event);
    }

    bool init()
    {
        JZNodeBuilder builder;
        if(!builder.build(&project,&program))
        {
            qDebug().noquote() << "build failed: " << builder.error();
            return false;
        }

        qDebug().noquote() << program.dump();
        engine.setProgram(&program);
        engine.init();
        return true;
    }

    bool run(bool async = false)
    {        
        if(!init())
            return false;

        if(async)
        {
            asyncThread = std::thread([this]{
                    this->call();
                });
        }
        else
        {
            call();
        }

        return true;
    }

    void paramChanged(QString name,QVariant value)
    {
        engine.setVariable(name,value);
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
    QMap<int,int> initWhileSetCase()
    {
        auto script = scriptFlow;
        JZNodeEvent *node_start = new JZNodeEvent();
        JZNodeWhile *node_while = new JZNodeWhile();   
        JZNodeLiteral *node_true = new JZNodeLiteral(); 

        paramDef->addVariable("i",Type_int);

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

    void stop()
    {
        engine.stop();
        asyncThread.join();
    }

    JZProject project;
    JZScriptFile *scriptFlow;
    JZScriptFile *scriptParam;
    JZParamFile *paramDef;

    JZNodeProgram program;
    JZNodeEngine engine;
    std::thread asyncThread;
};

void testProjectSave()
{
    
}

void testParamBinding()
{
    /*
        c = a + b
    */

    ScriptTest test;
    JZScriptFile *script = test.scriptParam;
    JZProject *project = &test.project;
    JZNodeEngine *engine = &test.engine;
    JZParamFile *paramDefine = test.paramDef;
    paramDefine->addVariable("a",Type_int,10);
    paramDefine->addVariable("b",Type_int,20);

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

    if(!test.run())
        return;

    qDebug() << engine->getVariable("c");

    test.paramChanged("a",200);
    qDebug() << engine->getVariable("c");

    test.paramChanged("b",200);
    qDebug() << engine->getVariable("c");
}

void testSequeue()
{
    /*
        a = 1 + 2
        b = 2 + 3
        c = 3 + 4
        d = 4 + 5
    */
    ScriptTest test;
    JZScriptFile *script = test.scriptFlow;
    JZProject *project = &test.project;
    JZNodeEngine *engine = &test.engine;
    auto *paramDef = test.paramDef;

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
    if(!test.run())
        return;

    int a = engine->getVariable("a").toInt();
    int b = engine->getVariable("b").toInt();
    int c = engine->getVariable("c").toInt();
    int d = engine->getVariable("d").toInt();
    qDebug() << a << b << c << d;
}

void testFor()
{
    /*
        for(int i = 5; i < 100; i++)
        {
            if(i > 20)
                break;            
        }    
    */

    ScriptTest test;
    JZScriptFile *script = test.scriptFlow;

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

    test.paramDef->addVariable("i",Type_int);
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
    
    if(test.run())
    {
        int i = test.engine.getVariable("i").toInt();
        qDebug() << "i = " << i;
    }
}

void testForEach()
{
    /*
        for(int i = 0; i < list.size(); i++)
        {
            sum = sum + i;
        }
    */
    ScriptTest test;
    JZScriptFile *script = test.scriptFlow;
    JZProject *project = &test.project;
    JZNodeEngine *engine = &test.engine;
    JZParamFile *paramDef = test.paramDef;
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
    int for_id = script->addNode(JZNodePtr(node_for));
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

    if(!test.init())
        return;

    QVariantList out;
    QVariant list = engine->getVariable("a");
    qDebug() << list;
    for(int i = 0; i < 10; i++)
        engine->call("list.push_back",{list,i+1},out);

    test.call();

    QVariant sum = engine->getVariable("sum");
    qDebug() << sum;
}

void testWhileLoop()
{
    /*
        while(i < 10)   
            i = i + 1;
    */

    ScriptTest test;
    JZScriptFile *script = test.scriptFlow;

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

    //test.project.addVariable("i",1);
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
    
    if(test.run())
    {
        int i = test.engine.getVariable("i").toInt();
        qDebug() << "i = " << i;
    }
}

void testBreakPoint()
{
    ScriptTest test;
    JZScriptFile *script = test.scriptFlow;
    JZNodeEngine *engine = &test.engine;
    QMap<int,int> node_value = test.initWhileSetCase();
    if(!test.run(true))
        return;

    qDebug() << "test pause/resume";
    for(int i = 0; i < 20; i++)
    {
        QThread::msleep(100);
        engine->pause();

        auto info = engine->runtimeInfo();
        if(node_value.contains(info.nodeId))
        {
            int value = engine->getVariable("i").toInt();
            if(!(abs(value - node_value[info.nodeId]) <= 1))
            {
                qDebug() << "Error:" << info.nodeId;
            }
        }        
        engine->resume();

    }

    qDebug() << "test breakpoint";
    for(int i = 0; i < 20; i++)
    {
        int node_id = node_value.keys()[rand()%100];
        engine->addBreakPoint(script->path(),node_id);
        QThread::msleep(100);

        auto info = engine->runtimeInfo();
        if(info.status == Status_pause)
        {
            int value = engine->getVariable("i").toInt();
            if(node_value[info.nodeId] - value != 1)
                qDebug() << "Error:" << value << node_value[info.nodeId];
        }
        engine->stepOver();
        QThread::msleep(10);
        info = engine->runtimeInfo();
        if(info.status == Status_pause)
        {
            int value = engine->getVariable("i").toInt();
            if((node_value[info.nodeId]+100)%100 - value != 1)
                qDebug() << "after stepover Error:" << value << node_value[info.nodeId];
        }

        engine->removeBreakPoint(script->path(),node_id);
        engine->resume();
    }
    test.stop();
}

void testDebugServer()
{       
    ScriptTest test;
    JZScriptFile *script = test.scriptFlow;
    JZNodeEngine *engine = &test.engine;
    QMap<int,int> node_value = test.initWhileSetCase();        
    if(!test.run(true))
        return;

    JZNodeDebugServer server;
    JZNodeDebugClient client;
    server.setEngine(engine);
    if(!server.startServer(18888))
    {
        qDebug() << "start server failded";
        return;
    }

    if(!client.connectToServer("127.0.0.1",18888))
    {
        qDebug() << "connect to server failded";
        return;
    }

    qDebug() << "test pause/resume";
    for(int i = 0; i < 20; i++)
    {
        QThread::msleep(100);
        client.pause();

        auto info = client.runtimeInfo();
        if(node_value.contains(info.nodeId))
        {
            int value = client.getVariable("i").toInt();
            if(!(abs(value - node_value[info.nodeId]) <= 1))
            {
                qDebug() << "Error:" << info.nodeId;
            }
        }        
        client.resume();
    }

    qDebug() << "test breakpoint";
    for(int i = 0; i < 20; i++)
    {
        int node_id = node_value.keys()[rand()%100];
        client.addBreakPoint(script->path(),node_id);
        QThread::msleep(100);

        auto info = client.runtimeInfo();
        if(info.status == Status_pause)
        {
            int value = client.getVariable("i").toInt();
            if(node_value[info.nodeId] - value != 1)
                qDebug() << "Error:" << value << node_value[info.nodeId];
        }
        client.stepOver();
        QThread::msleep(10);
        info = client.runtimeInfo();
        if(info.status == Status_pause)
        {
            int value = client.getVariable("i").toInt();
            if((node_value[info.nodeId]+100)%100 - value != 1)
                qDebug() << "after stepover Error:" << value << node_value[info.nodeId];
        }

        client.removeBreakPoint(script->path(),node_id);
        client.resume();
    }
    client.stop();
    server.stopServer();
}

void testExpr()
{
    ScriptTest test;
    JZScriptFile *script = test.scriptFlow;
    JZNodeEngine *engine = &test.engine;
    JZProject *project = &test.project;
    JZParamFile *paramDef = test.paramDef;

    QVector<int> op_type = {Node_add,Node_sub,Node_mul,Node_div,Node_mod,Node_eq,Node_ne,Node_le,
        Node_ge,Node_lt,Node_gt,Node_and,Node_or,Node_bitand,Node_bitor,Node_bitxor};

    JZNodeEvent *node_start = new JZNodeEvent();      
    JZNodeParam *node_a = new JZNodeParam();
    JZNodeParam *node_b = new JZNodeParam();

    node_start->setEventType(Event_programStart);
    paramDef->addVariable("a",Type_int,100);
    paramDef->addVariable("b",Type_int,50);

    node_a->setVariable("a");
    node_b->setVariable("b");

    script->addNode(JZNodePtr(node_start));    
    script->addNode(JZNodePtr(node_a));
    script->addNode(JZNodePtr(node_b));    

    QMap<int,int> node_value;
    JZNodeSetParam *pre_node = nullptr;
    for(int i = 0; i < op_type.size(); i++)
    {
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

    if(!test.run())
        return;

    for(int i = 0; i < op_type.size(); i++)
    {
        QVariant v = engine->getVariable("i" + QString::number(i));
        qDebug() << v;
    }
}

void testCustomExpr()
{
    ScriptTest test;
    JZScriptFile *script = test.scriptFlow;
    JZNodeEngine *engine = &test.engine;
    JZProject *project = &test.project;
    JZParamFile *paramDef = test.paramDef;

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
        qDebug() << error;
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

    if(!test.run())
        return;

    int a = engine->getVariable("a").toInt();
    int b = engine->getVariable("b").toInt();
    QVariant c = engine->getVariable("c");
    qDebug() << c << pow(a,b) - pow(b,a);
}

void testFunction()
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
    ScriptTest test;

    FunctionParam inParam,outParam;
    inParam.name = "n";
    inParam.dataType = Type_int;
    outParam.name = "result";
    outParam.dataType = Type_int;

    FunctionDefine fab;
    fab.name = "fab";
    fab.paramIn.push_back(inParam);
    fab.paramOut.push_back(outParam);
    JZScriptFile *script = test.project.addFunction(fab);

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

    if(!test.run())
        return;

    QVariantList out;
    for(int i = 0; i <= 15; i++)
    {
        test.engine.call("fab",{i},out);
        qDebug() << "fab("+ QString::number(i) + ")" << out;
    }
}

void testClass()
{
    ScriptTest test;
    JZScriptFile *script = test.scriptFlow;
    JZProject *project = &test.project;    
    JZNodeEngine *engine = &test.engine;
    JZParamFile *paramDef = test.paramDef;

    auto classBase = project->addClass("ClassBase");
    project->addClass("ClassA","ClassBase");
    project->addClass("ClassB","ClassBase");

    classBase->addMemberVariable("a",Type_int,50);
    paramDef->addVariable("a",JZNodeObjectManager::instance()->getClassId("ClassA"));
    paramDef->addVariable("b",JZNodeObjectManager::instance()->getClassId("ClassB"));

    if(!test.run())
        return;
/*
    QVariant value;
    value = engine->getVariable("a.a");
    qDebug() << value;

    engine->setVariable("a.a",100);
    value = engine->getVariable("a.a");
    qDebug() << value;

    engine->setVariable("b.a",200);
    value = engine->getVariable("b.a");    
    qDebug() << value;       
*/
}

void testBind()
{
    auto add = [](int a,int b,int c)->int{ return a + b + c; };
    auto str_left = [](const QString &inst,int size)->QString{
        return inst.left(size);
    };

    QString str = "test QString>()lalala";
    QVariantList out;

    auto impl_add = jzbind::createFuncion(add);
    impl_add->call({100,200,35},out);

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

void testCClass()
{
    ScriptTest test;
    JZProject *project = &test.project;
    JZNodeEngine *engine = &test.engine;    
    JZParamFile *paramDef = test.paramDef;

    paramDef->addVariable("a",Type_string);
    paramDef->addVariable("b",Type_string);
    paramDef->addVariable("c",Type_string);
    if(!test.run())
        return;

    engine->setVariable("a","i'love jznode");
    engine->setVariable("b","jznode");
    engine->setVariable("c","money");

    QVariant a = engine->getVariable("a");
    QVariant b = engine->getVariable("b");
    QVariant c = engine->getVariable("c");
    QVariantList out;
    engine->call("string.left",{a,6},out);
    qDebug() << out;

    engine->call("string.size",{out[0]},out);
    qDebug() << out;

    engine->call("string.replace",{a,b,c},out);
    qDebug() << out;
}

void testBuild()
{    
    testBind();
    testParamBinding();
    testWhileLoop();
    testFor();
    testClass();
    testCClass();
    testSequeue();
    testExpr();
    testCustomExpr();
    testFunction();
    testForEach();
    testBreakPoint();    
    testDebugServer();   
}
