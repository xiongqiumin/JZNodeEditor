#include "JZNodeCompiler.h"
#include "JZNodeValue.h"

// GraphNode
GraphNode::GraphNode()
{
    node = nullptr;
}

// Graph
Graph::Graph()
{
}

Graph::~Graph()
{
}

void Graph::clear()
{
    topolist.clear();
    m_nodes.clear();
    error.clear();
}

bool Graph::check()
{
    if (!toposort())
        return false;

    for (auto v : m_nodes)
    {
        if(v->paramIn.size() != v->node->propCount(Prop_in))
            error += v->node->name() + "输入不全\n";
    }

    return error.isEmpty();
}

bool Graph::toposort()
{
    QList<GraphNode *> result;
    QMap<int, int> nodeMap;
    for (auto v : m_nodes)
        nodeMap[v.data()->node->id()] = 0;
    for (auto v : m_nodes)
    {
        auto &next = v->next;
        for (int j = 0; j < next.size(); j++)
            nodeMap[next[j].nodeId]++;
    }

    while (true)
    {
        QList<GraphNode *> tmp;
        auto it = nodeMap.begin();
        while (it != nodeMap.end())
        {
            if (it.value() == 0)
            {
                auto cur_node = m_nodes[it.key()].data();
                auto &next = cur_node->next;
                for (int i = 0; i < next.size(); i++)
                    nodeMap[next[i].nodeId]--;

                tmp.push_back(cur_node);
                it = nodeMap.erase(it);
            }
            else
                it++;
        }
        if (tmp.size() == 0)
        {
            if (nodeMap.size() != 0)
            {
                error += "存在环,请检查";
                return false;
            }

            break;
        }
        std::sort(tmp.begin(), tmp.end(), [](const GraphNode *n1, const GraphNode *n2)
                  { return n1->node->id() < n2->node->id(); });
        result.append(tmp);
    }
    topolist = result;
    return true;
}

// JZNodeCompiler
JZNodeCompiler::JZNodeCompiler()
{
    m_project = nullptr;
    m_buildType = Build_none;    
}

JZNodeCompiler::~JZNodeCompiler()
{
}

QString JZNodeCompiler::error()
{
    return m_error;
}

int JZNodeCompiler::paramId(int nodeId,int propId)
{
    return m_program.paramId(nodeId,propId);
}

int JZNodeCompiler::paramId(const JZNodeGemo &gemo)
{
    return m_program.paramId(gemo);
}

void JZNodeCompiler::connectGraph(Graph *graph,JZNode *node)
{
    auto it = m_nodeGraph.find(node);
    if(it != m_nodeGraph.end())
        return;

    m_nodeGraph[node] = graph;
    auto lines = m_project->connectList();
    for (int i = 0; i < lines.size(); i++)
    {        
        auto &line = lines[i];
        if(line.from.nodeId == node->id())
            connectGraph(graph,m_project->getNode(line.to.nodeId));
        if(line.to.nodeId == node->id())
            connectGraph(graph,m_project->getNode(line.from.nodeId));
    }
}

Graph *JZNodeCompiler::getGraph(JZNode *node)
{
    auto it = m_nodeGraph.find(node);
    if(it != m_nodeGraph.end())    
        return it.value();

    GraphPtr graph = GraphPtr(new Graph());
    m_graphs.push_back(graph);    
    connectGraph(graph.data(),node);
    return graph.data();
}

bool JZNodeCompiler::genGraphs()
{        
    m_graphs.clear();
    auto node_list = m_project->nodeList();
    for (int i = 0; i < node_list.size(); i++)
    {
        JZNode *node = m_project->getNode(node_list[i]);        
        Graph *graph = getGraph(node);
        
        GraphNode *graph_node = new GraphNode();        
        graph_node->node = node;
        graph->m_nodes.insert(node->id(), GraphNodePtr(graph_node));
    }

    auto lines = m_project->connectList();
    for (int i = 0; i < lines.size(); i++)
    {        
        JZNode *node = m_project->getNode(lines[i].from.nodeId);
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

    for(int i = 0; i < m_graphs.size(); i++)    
    {
        Graph *graph = m_graphs[i].data();  
        if(!graph->check())
        {
            m_error += graph->error;
            return false;
        }
    }

    return true;
}

bool JZNodeCompiler::buildGraph(Graph *graph)
{    
    m_currentGraph = graph;    

    QList<GraphNode *> graph_list = m_currentGraph->topolist;
    for (int graph_idx = 0; graph_idx < graph_list.size(); graph_idx++)
    {
        GraphNode *graph = graph_list[graph_idx];
        JZNode *node = graph->node;

        m_nodeInfo[node->id()] = NodeInfo();
        m_currentNodeInfo = &m_nodeInfo[node->id()];
        m_regId = Reg_User;

        int start = m_program.opList.size();
        QString error;
        if(!node->compiler(this,error))
            return false;

        //set output
        auto it_out = graph->paramOut.begin();
        while(it_out != graph->paramOut.end())
        {
            auto &out_list = it_out.value();
            auto pin = node->prop(it_out.key());
            if(!(pin->flag() & Prop_flow))
            {
                for(int out_idx = 0; out_idx < out_list.size(); out_idx++)
                {
                    int from_id = m_program.paramId(node->id(),it_out.key());
                    int to_id = m_program.paramId(out_list[out_idx]);

                    JZNodeIR op;
                    op.type = OP_set;
                    op.params << QString::number(to_id) << QString::number(from_id);
                    addStatement(op);
                }
            }            
            it_out++;
        }

        if(m_currentNodeInfo->jmpList.size() == 0)
            addJumpNode(0);        

        //set jump flow                        
        m_currentNodeInfo->start = start;
        m_currentNodeInfo->end = m_program.opList.size() - 1;
        m_program.opList[start].memo = node->name();
        
    }    
    
    if(m_buildType == Build_flow)
        addFlowEvent();
    else if(m_buildType == Build_paramBinding)
        addParamChangedEvent();
    else if(m_buildType == Build_function)
    {
        int return_pc = addStatement(JZNodeIR(OP_return));
        for(int i = 0; i < 1000; i++){
            if(m_program.opList[i].type == OP_return){
                JZNodeIR jmp(OP_jmp);
                jmp.params << return_pc;
                m_program.opList[i] = jmp;
            }
        }
    }
    
    return true;
}

bool JZNodeCompiler::addFlowEvent()
{   
    QList<GraphNode *> graph_list = m_currentGraph->topolist;
    for (int graph_idx = 0; graph_idx < graph_list.size(); graph_idx++)
    {
        JZNodeEvent *node = dynamic_cast<JZNodeEvent*>(graph_list[graph_idx]->node);
        if(!node || node->propInList(Prop_flow).size() != 0)        
            continue;

        JZEventHandle handle;
        handle.type = node->m_eventType;
        handle.pc = m_nodeInfo[node->id()].start;
        handle.params << node->m_params;
        m_program.m_events.push_back(handle);            
    }        

    return true;
}

bool JZNodeCompiler::addParamChangedEvent()
{        
    QList<GraphNode*> graph_list = m_currentGraph->topolist;
    for(int node_idx = 0; node_idx < graph_list.size(); node_idx++)
    {                
        GraphNode *graph_node = graph_list[node_idx];
        JZNode *node = graph_node->node;

        NodeInfo &info = m_nodeInfo[node->id()];
        Q_ASSERT(info.jmpList.size() == 1);
        int pc = info.jmpList[0].pc;
        if(node_idx != graph_list.size() - 1)
        {
            int next_node = m_currentGraph->topolist[node_idx + 1]->node->id();
            JZNodeIR jmp(OP_jmp);
            jmp.params << QString::number(m_nodeInfo[next_node].start);
            m_program.opList[pc] = jmp;
        }
        else
        {
            m_program.opList[pc] = JZNodeIR(OP_return);
        }

        // add param changed handle function
        auto inList = node->propInList(Prop_param);
        if(inList.size() == 0)
        {
            auto outList = node->propOutList(Prop_param);
            //对每一个变化的参数重新计算节点的全部输出, 会有些冗余
            for(int out_idx = 0; out_idx < outList.size(); out_idx++)
            {
                int param_id = m_program.paramId(node->id(),outList[out_idx]);

                FunctionDefine define;
                define.name = "EventHandle" + QString::number(m_program.m_events.size());

                JZNodeIR jmp(OP_jmp);
                jmp.params << m_nodeInfo[node->id()].start;
                addStatement(jmp);                                

                JZEventHandle handle;
                handle.type = Event_paramChanged;                
                handle.params << QString::number(param_id);
                handle.function = define;
                m_program.m_events.push_back(handle);
            }
        }
    }

    return true;
}

bool JZNodeCompiler::build(JZProject *project,JZNodeProgram &result)
{
    m_error.clear();    
    m_project = project;    
    m_program = JZNodeProgram();

    m_buildType = Build_paramBinding;
    genGraphs();
    for(int i = 0; i < m_graphs.size(); i++)
    {
        auto graph = m_graphs[i].data();
        if(!buildGraph(graph))
            return false;
    }    
        
    result = m_program;
    return true;
}

const QList<Graph*> &JZNodeCompiler::graphs() const
{
    QList<Graph*> list;
    for(int i = 0; i < m_graphs.size(); i++)         
        list.push_back(m_graphs[i].data());
    
    return list;    
}

int JZNodeCompiler::addStatement(const JZNodeIR &ir)
{
    m_program.opList.push_back(ir);
    return m_program.opList.size() - 1;
}

void JZNodeCompiler::addJumpNode(int idx)
{    
    NodeInfo::Jump info;    
    JZNodeIR jmp(OP_none);
    info.pc = addStatement(jmp);
    
    m_currentNodeInfo->jmpList.push_back(info);
}

int JZNodeCompiler::allocReg()
{
    return m_regId++;
}

void JZNodeCompiler::freeReg(int id)
{

}

void JZNodeCompiler::copyVariable(int src,int dst)
{
    JZNodeIR op(OP_set);
    op.params << QString::number(dst) << QString::number(src);
    addStatement(op);
}
