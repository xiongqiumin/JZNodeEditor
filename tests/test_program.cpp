#include <QEventLoop>
#include <QDebug>
#include "JZProject.h"
#include "JZNodeCompiler.h"
#include "JZNodeEngine.h"
#include "JZNodeValue.h"
#include "JZNodeProgram.h"
#include "JZNodeBuilder.h"

class ScriptTest
{
public:    
    ScriptTest()
    {        
        scriptFlow = new JZScriptFile(ProjectItem_scriptFlow,false);
        scriptFlow->setName("script_flow.jz");

        scriptParam = new JZScriptFile(ProjectItem_scriptParam,false);
        scriptParam->setName("script_param.jz");
        
        project.addItem("./",scriptFlow);
        project.addItem("./",scriptParam);
    }        

    bool run(bool async = false)
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

        JZEvent event;
        event.setEventType(Event_programStart);
        auto list = program.matchEvent(&event);
        Q_ASSERT(list.size() == 1);

        const JZEventHandle *handle = list[0];      
        QVariantList in = event.params;
        QVariantList out;        
        engine.call(&handle->function,in,out);
        return true;
    }

    void stop()
    {
        engine.stop();
    }

    JZProject project;
    JZScriptFile *scriptFlow;
    JZScriptFile *scriptParam;

    JZNodeProgram program;
    JZNodeEngine engine;
};

void testProjectSave()
{
    
}

void testSet()
{

}

void testBranch()
{

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
    node_set->setParamId("i");

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
    node_param->setParamId("i");
    node_set->setParamId("i");

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
    JZNodeEvent *node_start = new JZNodeEvent();
    JZNodeWhile *node_while = new JZNodeWhile();   
    JZNodeLiteral *node_true = new JZNodeLiteral(); 

    script->addNode(JZNodePtr(node_start));
    script->addNode(JZNodePtr(node_while));

    node_true->setLiteral(true);
    
    node_start->setEventType(Event_programStart);
    script->addConnect(JZNodeGemo(node_start->id(),node_start->flowOut()),JZNodeGemo(node_while->id(),node_while->flowIn()));
    script->addConnect(JZNodeGemo(node_true->id(),node_true->paramOut(0)),JZNodeGemo(node_while->id(),node_while->paramIn(0)));
    for(int i = 0; i < 100; i++)
    {
        JZNodeSetParam *node_set = new JZNodeSetParam();
        node_set->setParamId("i");
        node_set->setPropValue(node_set->paramIn(0),i);
        script->addNode(JZNodePtr(node_set));

        if(i == 0)
            script->addConnect(JZNodeGemo(node_while->id(),node_while->subFlowOut(0)),JZNodeGemo(node_set->id(),node_set->flowIn()));
    }
}

void testBuild()
{
    //testWhileLoop();
    //testFor();
    testBreakPoint();
}
