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

class ScriptTest
{
public:    
    ScriptTest()
    {        
        scriptFlow = (JZScriptFile*)project.getItem("./程序流程/流程");
        scriptParam = (JZScriptFile*)project.getItem("./数据联动/联动");
    }        

    void call()
    {
        JZEvent event;
        event.setEventType(Event_programStart);
        auto list = program.matchEvent(&event);
        for(int i = 0; i < list.size(); i++)
        {
            const JZEventHandle *handle = list[0];
            QVariantList in = event.params;
            QVariantList out;
            engine.call(&handle->function,in,out);
        }
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

        JZEvent event;
        event.setEventType(Event_paramChanged);
        event.params << name;
        auto list = program.matchEvent(&event);
        for(int i = 0; i < list.size(); i++)
        {
            const JZEventHandle *handle = list[i];
            QVariantList in,out;
            engine.call(&handle->function,in,out);
        }
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

        project.addVariable("i",0);

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
            node_set->setParamId("i",true);
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

    project->addVariable("a",10);
    project->addVariable("b",20);

    JZNodeSetParamData *node_c = new JZNodeSetParamData();
    JZNodeAdd *node_add = new JZNodeAdd();
    JZNodeParam *node_a = new JZNodeParam();
    JZNodeParam *node_b = new JZNodeParam();

    script->addNode(JZNodePtr(node_a));
    script->addNode(JZNodePtr(node_b));
    script->addNode(JZNodePtr(node_c));
    script->addNode(JZNodePtr(node_add));

    node_a->setParamId("a",true);
    node_b->setParamId("b",true);
    node_c->setParamId("c");
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

    JZNodeEvent *node_start = new JZNodeEvent();
    JZNodeSequence *node_seq = new JZNodeSequence();

    int start_id = script->addNode(JZNodePtr(node_start));
    int for_id = script->addNode(JZNodePtr(node_seq));

    project->addVariable("a",0);
    project->addVariable("b",0);
    project->addVariable("c",0);
    project->addVariable("d",0);

    node_seq->addSequeue();
    node_seq->addSequeue();
    node_seq->addSequeue();
    node_seq->addSequeue();

    node_start->setEventType(Event_programStart);
    script->addConnect(JZNodeGemo(start_id,node_start->flowOut()),JZNodeGemo(for_id,node_seq->flowIn()));
    for(int i = 0; i < 4; i++)
    {
        JZNodeSetParam *node_set = new JZNodeSetParam();
        node_set->setParamId(QString('a' + i),true);        

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

    test.project.addVariable("i",0);
    node_set->setParamId("i",true);

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

    JZNodeEvent *node_start = new JZNodeEvent();
    JZNodeForEach *node_for = new JZNodeForEach();
    JZNodeParam *node_param = new JZNodeParam();
    JZNodeParam *node_sum = new JZNodeParam();
    JZNodeAdd *node_add = new JZNodeAdd();
    JZNodeSetParam *node_set = new JZNodeSetParam();
    node_param->setParamId("a",true);
    node_sum->setParamId("sum",true);
    node_set->setParamId("sum",true);

    int start_id = script->addNode(JZNodePtr(node_start));
    int for_id = script->addNode(JZNodePtr(node_for));
    script->addNode(JZNodePtr(node_param));
    script->addNode(JZNodePtr(node_sum));
    script->addNode(JZNodePtr(node_add));
    script->addNode(JZNodePtr(node_set));

    //start
    node_start->setEventType(Event_programStart);
    script->addConnect(JZNodeGemo(start_id,node_start->flowOut()),JZNodeGemo(for_id,node_for->flowIn()));

    script->addConnect(node_param->paramOutGemo(0),node_for->paramInGemo(0));

    // sum = sum + i
    script->addConnect(node_sum->paramOutGemo(0),node_add->paramInGemo(0));
    script->addConnect(node_for->paramOutGemo(0),node_add->paramInGemo(1));

    script->addConnect(node_add->paramOutGemo(0),node_set->paramInGemo(0));
    script->addConnect(node_for->subFlowOutGemo(0),node_set->flowInGemo());

    project->addVariable("sum",0);
    project->addClassVariable("a","list");
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

    test.project.addVariable("i",1);
    node_param->setParamId("i",true);
    node_set->setParamId("i",true);

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
        int pt_id = engine->addBreakPoint(script->path(),node_id);
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

        engine->removeBreakPoint(pt_id);
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
        int pt_id = client.addBreakPoint(script->path(),node_id);
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

        client.removeBreakPoint(pt_id);
        client.resume();
    }
    client.stop();
    client.disconnectFromServer();
    
    server.quit();
    server.wait();
}

void testExpr()
{
    ScriptTest test;
    JZScriptFile *script = test.scriptFlow;

    JZNodeEvent *node_start = new JZNodeEvent();  
    JZNodeSetParam *node_set = new JZNodeSetParam();

    script->addNode(JZNodePtr(node_start));
    script->addNode(JZNodePtr(node_set));
    script->addConnect(node_start->flowOutGemo(),node_set->flowInGemo());
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

    JZScriptFunctionFile *file = new JZScriptFunctionFile();
    test.project.addItem("./",file);

    file->addFunction("fab",{"n"},{""});
    JZScriptFile *script = (JZScriptFile*)file->getItem("fab");
    FunctionDefine fab = script->function();

    JZNodeFunctionStart *node_start = new JZNodeFunctionStart();    
    JZNodeBranch *node_branch = new JZNodeBranch();    
    JZNodeGE *node_ge = new JZNodeGE();
    node_ge->setPropValue(node_ge->paramIn(1),2);
    JZNodeAdd *node_add = new JZNodeAdd();    

    JZNodeFunction *func1 = new JZNodeFunction();
    func1->setFunction(&fab,false);
    JZNodeFunction *func2 = new JZNodeFunction();
    func2->setFunction(&fab,false);

    JZNodeSub *sub1 = new JZNodeSub();
    JZNodeSub *sub2 = new JZNodeSub();

    JZNodeParam *n = new JZNodeParam();
    n->setParamId("n",false);

    JZNodeReturn *ret1 = new JZNodeReturn();
    JZNodeReturn *ret2 = new JZNodeReturn();    
    ret1->addParamIn("n",Prop_edit);
    ret2->addParamIn("n",Prop_edit);

    script->addNode(JZNodePtr(node_start));
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

    JZNodeObjectDefine classBase;
    classBase.className = "ClassBase";
    classBase.params["a"] = 50;

    JZNodeObjectDefine classA;
    classA.className = "ClassA";
    
    JZNodeObjectDefine classB;
    classB.className = "ClassB";

    project->registObject(classBase);
    project->registObject(classA,"ClassBase");
    project->registObject(classB,"ClassBase");

    project->addClassVariable("a","ClassA");
    project->addClassVariable("b","ClassB");

    if(!test.run())
        return;

    QVariant value;
    value = engine->getVariable("a.a");
    qDebug() << value;

    engine->setVariable("a.a",100);
    value = engine->getVariable("a.a");
    qDebug() << value;

    engine->setVariable("b.a",200);
    value = engine->getVariable("b.a");    
    qDebug() << value;       
}

void testCClass()
{
    ScriptTest test;
    JZProject *project = &test.project;
    JZNodeEngine *engine = &test.engine;    

    project->addClassVariable("a","string");
    project->addClassVariable("b","string");
    project->addClassVariable("c","string");
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
    testParamBinding();
    testWhileLoop();
    testFor();
    testClass();
    testCClass();
    testSequeue();
    testExpr();
    testFunction();
    testForEach();
    //testBreakPoint();
    //testDebugServer();
}
