#include "JZNodeCompiler.h"
#include "JZNodeValue.h"
#include <QVector>
#include <QSet>

//NodeInfo
NodeInfo::NodeInfo()
{
    node = nullptr;
    start = -1;
    end = -1;    
    parentId = -1;
}

// JZNodeCompiler
int JZNodeCompiler::paramId(int nodeId,int propId)
{
    Q_ASSERT(nodeId >= 0 && propId >= 0);
    return nodeId * 100 + propId;
}

int JZNodeCompiler::paramId(const JZNodeGemo &gemo)
{
    return paramId(gemo.nodeId,gemo.propId);
}

QString JZNodeCompiler::paramName(int id)
{    
    if(id < Stack_User)
        return QString().asprintf("Node%d.%d",id/100,id%100);
    else if(id < Reg_Start)
        return QString().asprintf("Stack%d",id - Stack_User);
    else if(id == Reg_Cmp)
        return "Reg_Cmp";    

    return QString();        
}

JZNodeGemo JZNodeCompiler::paramGemo(int id)
{
    if(id < Stack_User)
        return JZNodeGemo(id/100,id%100);
    else
        return JZNodeGemo();
}

JZNodeCompiler::JZNodeCompiler()
{    
    m_script = nullptr;
    m_scriptFile = nullptr;
    m_currentGraph = nullptr;
    m_currentNodeInfo = nullptr;
    m_stackId = 0;
}

JZNodeCompiler::~JZNodeCompiler()
{
}

bool JZNodeCompiler::build(JZScriptFile *scriptFile,JZNodeScript *result)
{    
    m_scriptFile = scriptFile;    
    m_script = result;
    m_script->clear();
    m_nodeGraph.clear();
    if(!genGraphs())
        return false;

    bool buildRet = true;
    for(int i = 0; i < m_script->graphs.size(); i++)
    {
        auto graph = m_script->graphs[i].data();
        int buildType = m_scriptFile->itemType();
    
        bool ret = false;
        if(buildType == ProjectItem_scriptFlow)
            ret = bulidControlFlow(graph);
        else if(buildType == ProjectItem_scriptParam)
            ret = buildParamBinding(graph);     

        buildRet = (buildRet && ret);
    } 
    m_script->file = scriptFile->path();
    m_script->nodeInfo = m_nodeInfo; 
    return buildRet;
}

QString JZNodeCompiler::error()
{
    return m_error;
}

void JZNodeCompiler::connectGraph(Graph *graph,JZNode *node)
{
    auto it = m_nodeGraph.find(node);
    if(it != m_nodeGraph.end())
        return;

    m_nodeGraph[node] = graph;
    auto lines = m_scriptFile->connectList();
    for (int i = 0; i < lines.size(); i++)
    {        
        auto &line = lines[i];
        if(line.from.nodeId == node->id())
            connectGraph(graph,m_scriptFile->getNode(line.to.nodeId));
        if(line.to.nodeId == node->id())
            connectGraph(graph,m_scriptFile->getNode(line.from.nodeId));
    }
}

Graph *JZNodeCompiler::getGraph(JZNode *node)
{
    auto it = m_nodeGraph.find(node);
    if(it != m_nodeGraph.end())    
        return it.value();

    GraphPtr graph = GraphPtr(new Graph());
    m_script->graphs.push_back(graph);    
    connectGraph(graph.data(),node);
    return graph.data();
}

bool JZNodeCompiler::genGraphs()
{        
    m_script->graphs.clear();
    auto node_list = m_scriptFile->nodeList();
    for (int i = 0; i < node_list.size(); i++)
    {
        JZNode *node = m_scriptFile->getNode(node_list[i]);        
        Graph *graph = getGraph(node);
        
        GraphNode *graph_node = new GraphNode();        
        graph_node->node = node;
        graph->m_nodes.insert(node->id(), GraphNodePtr(graph_node));
    }

    auto lines = m_scriptFile->connectList();
    for (int i = 0; i < lines.size(); i++)
    {        
        JZNode *node = m_scriptFile->getNode(lines[i].from.nodeId);
        Graph *graph = getGraph(node);

        auto from = graph->m_nodes[lines[i].from.nodeId];
        auto to = graph->m_nodes[lines[i].to.nodeId];
        int from_prop_id = lines[i].from.propId;
        int to_prop_id = lines[i].to.propId;

        auto from_pin = from->paramOut[from_prop_id];
        auto to_pin = to->paramIn[to_prop_id];
        from->next.push_back(lines[i].to);                
        from->paramOut[from_prop_id].push_back(lines[i].to);
        to->paramIn[to_prop_id].push_back(lines[i].from);        
    }

    for(int i = 0; i < m_script->graphs.size(); i++)    
    {
        Graph *graph = m_script->graphs[i].data();        
        if(!graph->check())
        {
            m_error += graph->error;
            return false;
        }
    }

    return true;
}    

bool JZNodeCompiler::bulidControlFlow(Graph *graph)
{    
    m_currentGraph = graph;    

    //build node
    QList<GraphNode *> graph_list = m_currentGraph->topolist;
    for (int graph_idx = 0; graph_idx < graph_list.size(); graph_idx++)
    {
        GraphNode *graph = graph_list[graph_idx];
        JZNode *node = graph->node;
        if(!node->isFlowNode())
            continue;

        m_nodeInfo[node->id()] = NodeInfo();
        m_currentNodeInfo = &m_nodeInfo[node->id()];
        m_currentNodeInfo->node = node;
        m_stackId = Stack_User;

        int start = m_script->statmentList.size();
        QString error;
        JZNodeIRNodeId *node_ir = new JZNodeIRNodeId();
        node_ir->id = node->id();
        addStatement(JZNodeIRPtr(node_ir));        
        if(!node->compiler(this,error))
            return false;        

        m_currentNodeInfo->start = start;
        m_currentNodeInfo->end = m_script->statmentList.size();
    }    

    //set sub node    
    for(int node_idx = 0; node_idx < graph_list.size(); node_idx++)
    {   
        GraphNode *graph_node = graph_list[node_idx];
        NodeInfo &info = m_nodeInfo[graph_node->node->id()];
        for(int i = 0; i < info.jmpSubList.size(); i++)
        {
            int prop = info.jmpSubList[i].prop;
            if(!graph_node->paramOut.contains(prop))
                continue;

            auto &out_list = graph_node->paramOut[prop];
            Q_ASSERT(out_list.size() == 1);

            JZNodeGemo next_gemo = out_list[0];
            JZNodeIRJmp *jmp = new JZNodeIRJmp(OP_jmp);
            jmp->jmpPc = m_nodeInfo[next_gemo.nodeId].start;
            replaceStatement(info.jmpSubList[i].pc,JZNodeIRPtr(jmp));

            replaceSubNode(out_list[0].nodeId,graph_node->node->id(),i);
        }                    
    }

    // connect node
    for(int node_idx = 0; node_idx < graph_list.size(); node_idx++)
    {   
        GraphNode *graph_node = graph_list[node_idx];
        NodeInfo &info = m_nodeInfo[graph_node->node->id()];
        if(info.parentId != -1)
            continue;

        //connect next
        for(int i = 0; i < info.jmpList.size(); i++)
        {
            int prop = info.jmpList[i].prop;
            int pc = info.jmpList[i].pc;
            if(graph_node->paramOut.contains(prop))
            {
                auto &out_list = graph_node->paramOut[info.jmpList[i].prop];
                Q_ASSERT(out_list.size() == 1);

                JZNodeGemo next_gemo = out_list[0];
                JZNodeIRJmp *jmp = new JZNodeIRJmp(OP_jmp);
                jmp->jmpPc = m_nodeInfo[next_gemo.nodeId].start;
                replaceStatement(pc,JZNodeIRPtr(jmp));
            }
            else
            {
                JZNodeIR *ir_return = new JZNodeIR(OP_return);
                replaceStatement(pc,JZNodeIRPtr(ir_return));
            }
        }          
    }

    // add event
    for(int node_idx = 0; node_idx < graph_list.size(); node_idx++)
    {
        JZNode *node = graph_list[node_idx]->node;
        if(node->type() == Node_event)
        {
            JZNodeEvent *node_event = (JZNodeEvent *)graph_list[node_idx]->node;
            QString func_name = "on_node_" + QString::number(node->id()) + "_event_" + QString::number(node_event->id());
            if(!m_script->function(func_name))
            {
                FunctionDefine define;
                define.name = func_name;
                define.addr = m_nodeInfo[node->id()].start;
                define.script = m_script;

                JZEventHandle handle;
                handle.type = node_event->eventType();
                handle.function = define;
                handle.params << node_event->m_params;
                m_script->events.push_back(handle);
            }
        }
    }

    return true;
}

bool JZNodeCompiler::buildDataFlow(const QList<GraphNode*> &graph_list)
{   
    //build node
    for (int graph_idx = 0; graph_idx < graph_list.size(); graph_idx++)
    {
        GraphNode *graph = graph_list[graph_idx];
        JZNode *node = graph->node;
        Q_ASSERT(!node->isFlowNode());

        m_nodeInfo[node->id()] = NodeInfo();        
        m_stackId = Stack_User;

        int start = m_script->statmentList.size();
        QString error;
        JZNodeIRNodeId *node_ir = new JZNodeIRNodeId();
        node_ir->id = node->id();
        addStatement(JZNodeIRPtr(node_ir));
        if(!node->compiler(this,error))
            return false;                       
    }      
    return true;
}

bool JZNodeCompiler::buildParamBinding(Graph *graph)
{       
    m_currentGraph = graph;
    int start = m_script->statmentList.size();
    if(!buildDataFlow(graph->topolist))
        return false;
    addReturn();

    auto graph_list = m_currentGraph->topolist;
    for (int i = 0; i < graph_list.size(); i++)
    { 
        auto node = graph_list[i]->node;
        if(node->type() == Node_param)
        {
            JZNodeParam *node_param = (JZNodeParam*)node;
            QString param_name = node_param->paramId();
            QString func_name = "on_" + param_name + "_changed";
            if(!m_script->function(func_name))
            {
                FunctionDefine define;
                define.name = func_name;
                define.addr = start;
                define.script = m_script;

                JZEventHandle handle;
                handle.type = Event_paramChanged;
                handle.params << node_param->paramId();
                handle.function = define;
                m_script->events.push_back(handle);
            }
        }
    }
    return true;
}

void JZNodeCompiler::replaceSubNode(int id,int parent_id,int flow_index)
{
    NodeInfo &parent_info = m_nodeInfo[parent_id];
    NodeInfo &info = m_nodeInfo[id];
    info.parentId = parent_id;
        
    for(int i = 0; i < info.continueList.size(); i++)
    {
        JZNodeIRJmp *ir_jmp = new JZNodeIRJmp(OP_jmp);
        ir_jmp->jmpPc = parent_info.continuePc[flow_index];
        replaceStatement(info.continueList[i],JZNodeIRPtr(ir_jmp));
    }
    for(int i = 0; i < info.breakList.size(); i++)
    {
        JZNodeIRJmp *ir_jmp = new JZNodeIRJmp(OP_jmp);
        ir_jmp->jmpPc = parent_info.breakPc[flow_index];
        replaceStatement(info.breakList[i],JZNodeIRPtr(ir_jmp));
    }

    auto graph_node = m_currentGraph->graphNode(id);
    for(int i = 0; i < info.jmpList.size(); i++)
    {   
        int prop = info.jmpList[i].prop;
        int pc = info.jmpList[i].pc;
        if(graph_node->paramOut.contains(prop))
        {
            auto &out_list = graph_node->paramOut[prop];
            Q_ASSERT(out_list.size() == 1);

            JZNodeGemo next_gemo = out_list[0];
            JZNodeIRJmp *jmp = new JZNodeIRJmp(OP_jmp);
            jmp->jmpPc = m_nodeInfo[next_gemo.nodeId].start;
            replaceStatement(pc,JZNodeIRPtr(jmp));

            replaceSubNode(next_gemo.nodeId,parent_id,flow_index);
        }
        else
        {
            int continuePc = m_nodeInfo[info.parentId].continuePc[flow_index];
            JZNodeIRJmp *ir_continue = new JZNodeIRJmp(OP_jmp);
            ir_continue->jmpPc = continuePc;
            replaceStatement(pc,JZNodeIRPtr(ir_continue));    
        }                    
    }
}

int JZNodeCompiler::addStatement(JZNodeIRPtr ir)
{   
    Q_ASSERT(ir->pc == -1);
    ir->pc = m_script->statmentList.size();
    m_script->statmentList.push_back(ir);
    return ir->pc;
}

NodeInfo *JZNodeCompiler::currentNodeInfo()
{
    return m_currentNodeInfo;
}

int JZNodeCompiler::currentPc()
{
    return (m_script->statmentList.size() - 1);
}

void JZNodeCompiler::replaceStatement(int pc,JZNodeIRPtr ir)
{
    Q_ASSERT(ir->pc == -1 && pc < m_script->statmentList.size());
    ir->pc = pc;
    m_script->statmentList[pc] = ir;
}

int JZNodeCompiler::addJumpNode(int prop)
{   
    Q_ASSERT(m_currentNodeInfo->node->prop(prop) && m_currentNodeInfo->node->prop(prop)->isFlow()
             && m_currentNodeInfo->node->prop(prop)->isOutput() );
     
    NodeInfo::Jump info;    
    JZNodeIR *jmp = new JZNodeIR(OP_none);
    addStatement(JZNodeIRPtr(jmp));
    info.prop = prop;
    info.pc = jmp->pc;    
    m_currentNodeInfo->jmpList.push_back(info);
    return info.pc;
}

int JZNodeCompiler::addJumpSubNode(int prop)
{
    Q_ASSERT(m_currentNodeInfo->node->prop(prop) && m_currentNodeInfo->node->prop(prop)->isSubFlow());

    NodeInfo::Jump info;    
    JZNodeIR *jmp = new JZNodeIR(OP_nop);
    addStatement(JZNodeIRPtr(jmp));
    info.prop = prop;
    info.pc = jmp->pc;    
    m_currentNodeInfo->jmpSubList.push_back(info);
    return info.pc;
}

void JZNodeCompiler::setBreakContinue(QList<int> breakPc,QList<int> continuePc)
{
    m_currentNodeInfo->breakPc = breakPc;
    m_currentNodeInfo->continuePc = continuePc;
}

int JZNodeCompiler::addContinue()
{
    JZNodeIR *jmp = new JZNodeIRJmp(OP_jmp);    
    addStatement(JZNodeIRPtr(jmp));
    
    m_currentNodeInfo->continueList.push_back(jmp->pc);
    return jmp->pc;
}

int JZNodeCompiler::addBreak()
{
    JZNodeIR *jmp = new JZNodeIRJmp(OP_jmp);
    addStatement(JZNodeIRPtr(jmp));
    
    m_currentNodeInfo->breakList.push_back(jmp->pc);
    return jmp->pc;
}

int JZNodeCompiler::addReturn()
{
    JZNodeIR *ret = new JZNodeIR(OP_return);
    return addStatement(JZNodeIRPtr(ret));
}

int JZNodeCompiler::allocStack()
{
    return m_stackId++;
}

void JZNodeCompiler::freeStack(int id)
{

}

void JZNodeCompiler::addDataInput(int nodeId)
{
    GraphNode* in_node = m_currentGraph->graphNode(nodeId);
    auto in_list = in_node->node->paramInList();
    for(int prop_idx = 0; prop_idx < in_list.size(); prop_idx++)
    {
        int in_prop = in_list[prop_idx];
        if(in_node->paramIn.contains(in_prop))
        {
            QList<JZNodeGemo> &gemo_list = in_node->paramIn[in_prop];
            Q_ASSERT(gemo_list.size() > 0);
            for(int i = 0; i < gemo_list.size(); i++)
            {
                GraphNode *from_node = m_currentGraph->graphNode(gemo_list[i].nodeId);
                if(from_node->node->isFlowNode())
                    break;

                Q_ASSERT(gemo_list.size() == 1);
                int from_id = paramId(gemo_list[i]);
                int to_id = paramId(in_node->node->id(),in_prop);
                addSetVariable(irId(to_id),irId(from_id));
            }
        }
        else
        {
            auto prop = in_node->node->prop(in_prop);
            Q_ASSERT(prop->isEditable());

            int to_id = paramId(in_node->node->id(),in_prop);
            addSetVariable(irId(to_id),irLiteral(prop->value()));
        }
    }
}

void JZNodeCompiler::addFlowInput(int nodeId)
{
    Q_ASSERT(m_currentGraph->graphNode(nodeId)->node->isFlowNode());

    QList<GraphNode*> graph_list;
    QSet<GraphNode*> graphs;

    //广度遍历所有节点
    QList<GraphNode*> in_list;
    in_list << m_currentGraph->graphNode(nodeId);
    
    while(in_list.size() > 0)
    {
        QList<GraphNode*> next_list;
        for(int node_idx = 0; node_idx < in_list.size(); node_idx++)
        {
            auto in_node = in_list[node_idx];
            auto it = in_node->paramIn.begin();
            while(it != in_node->paramIn.end())
            {
                if(!in_node->node->prop(it.key())->isParam())
                {
                    it++;
                    continue;
                }

                QList<JZNodeGemo> &gemo_list = it.value();
                for(int i = 0; i < gemo_list.size(); i++)
                {
                    GraphNode *from_node = m_currentGraph->graphNode(gemo_list[i].nodeId);
                    if(from_node->node->isFlowNode())
                        continue;

                    if(!graphs.contains(from_node))
                    {
                        graphs << from_node;
                        next_list << from_node;
                    }
                }
                it++;
            }
        }
        in_list = next_list;
    }

    //要计算的节点
    for(int i = 0; i < m_currentGraph->topolist.size(); i++)
    {
        auto node = m_currentGraph->topolist[i];
        if(graphs.contains(node))
            graph_list.push_back(node);
    }
    buildDataFlow(graph_list);    
    addDataInput(nodeId);
}

void JZNodeCompiler::addFlowOutput(int nodeId)
{
    auto graph = m_currentGraph->graphNode(nodeId);
    auto node = graph->node;
    //set out put
    auto it_out = graph->paramOut.begin();
    while(it_out != graph->paramOut.end())
    {
        auto &out_list = it_out.value();
        auto pin = node->prop(it_out.key());
        if(pin->isParam())
        {
            for(int out_idx = 0; out_idx < out_list.size(); out_idx++)
            {
                int from_id = paramId(node->id(),it_out.key());
                int to_id = paramId(out_list[out_idx]);
                addSetVariable(irId(to_id),irId(from_id));
            }
        }
        it_out++;
    }
}

int JZNodeCompiler::addExpr(JZNodeIRParam dst,JZNodeIRParam p1,JZNodeIRParam p2,int op)
{
    JZNodeIRExpr *expr = new JZNodeIRExpr(op);
    expr->src1 = p1;
    expr->src2 = p2;
    expr->dst = dst;
    return addStatement(JZNodeIRPtr(expr));
}

int JZNodeCompiler::addCompare(JZNodeIRParam p1,JZNodeIRParam p2,int op)
{
    return addExpr(irId(Reg_Cmp),p1,p2,op);    
}

int JZNodeCompiler::addSetVariable(JZNodeIRParam dst,JZNodeIRParam src)
{
    Q_ASSERT(dst.type != JZNodeIRParam::Literal);

    JZNodeIRSet *op = new JZNodeIRSet();
    op->dst = dst;
    op->src = src;
    return addStatement(JZNodeIRPtr(op));
}
