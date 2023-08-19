#include "JZNodeCompiler.h"
#include "JZNodeValue.h"
#include <QVector>
#include <QSet>
#include "JZNodeFunctionManager.h"
#include "JZNodeFunction.h"
#include "JZClassFile.h"

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

GraphNode *Graph::graphNode(int id)
{
    auto it = m_nodes.find(id);
    if (it != m_nodes.end())
        return it.value().data();
    else
        return nullptr;
}

JZNode *Graph::node(int id)
{
    auto node = graphNode(id);
    if (node)
        return node->node;
    else
        return nullptr;
}

JZNodePin *Graph::pin(JZNodeGemo gemo)
{
    return pin(gemo.nodeId, gemo.propId);
}

JZNodePin *Graph::pin(int nodeId, int pinId)
{
    auto n = node(nodeId);
    if (n)
        return n->prop(pinId);
    else
        return nullptr;
}

bool Graph::check()
{
    //排序          
    if (!toposort())
        return false;
    /*
    //计算节点类型传递
    for (GraphNodePtr v : m_nodes)
    {
    auto it = v->paramOut.begin();
    while(it != v->paramOut.end())
    {
    int pin_id = it.key();
    JZNodePin *prop = v->node->prop(pin_id);
    QMap<int,int> outType = v->node->calcPropOutType(v->pinType);

    QList<JZNodeGemo> &list = it.value();
    for(int i = 0; i < list.size(); i++)
    {
    int next_node_id = list[i].nodeId;
    int next_pin_id = list[i].propId;

    GraphNode *next = m_nodes[next_node_id].data();
    next->pinType[next_pin_id] = outType[pin_id];
    }
    it++;
    }
    }

    for (auto &v : m_nodes)
    {
    auto in_list = v->node->paramInList();
    for(int i = 0; i < in_list.size(); i++)
    {
    int id = in_list[i];
    if(!v->paramIn.contains(id) && !v->node->prop(id)->isEditable())
    {
    error += v->node->name() + "(" + QString::number(v->node->id()) + ")" + "输入不全\n";
    break;
    }
    }
    }
    */
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
                error += "存在环,请检查,";
                QStringList names;
                auto it_loop = nodeMap.begin();
                while (it_loop != nodeMap.end())
                {
                    names.push_back(m_nodes[it_loop.key()]->node->name());
                    it_loop++;
                }
                error += names.join(" -> ");
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

//NodeCompilerInfo
NodeCompilerInfo::NodeCompilerInfo()
{
    node_id = -1;
    node_type = Node_none;
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
    else if(id >= Reg_Call)
        return "Reg_Call" + QString::number(id - Reg_Call);

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
    m_currentNode = nullptr;
    m_stackId = Stack_User;
}

JZNodeCompiler::~JZNodeCompiler()
{
}

void JZNodeCompiler::init(JZScriptFile *scriptFile)
{
    m_project = scriptFile->project();
    m_scriptFile = scriptFile;

    m_script = nullptr;        
    m_nodeGraph.clear();
    m_nodeInfo.clear();
    m_className.clear();
    m_localVaribales.clear();
    m_graphList.clear();
}

bool JZNodeCompiler::genGraphs(JZScriptFile *scriptFile, QVector<GraphPtr> &list)
{
    init(scriptFile);
    if(!genGraphs())
        return false;

    list = m_graphList;
    return true;
}

const QMap<int, NodeCompilerInfo> &JZNodeCompiler::compilerInfo()
{
    return m_nodeInfo;
}

bool JZNodeCompiler::build(JZScriptFile *scriptFile,JZNodeScript *result)
{        
    init(scriptFile);
    if (!genGraphs())
        return false;
    if(!checkGraphs())
        return false;

    m_script = result;
    m_script->clear();
    m_script->file = scriptFile->itemPath();    

    JZScriptClassFile *class_file = m_project->getClassFile(scriptFile);
    if (class_file)
        m_className = class_file->name();       

    auto node_list = m_scriptFile->nodeList();
    for (int i = 0; i < node_list.size(); i++)
    {
        NodeCompilerInfo info;
        auto node = m_scriptFile->getNode(node_list[i]);
        info.node_id = node->id();
        info.node_type = node->type();
        m_nodeInfo[info.node_id] = info;
    }                    

    bool buildRet = true;
    for(int graph_idx = 0; graph_idx < m_graphList.size(); graph_idx++)
    {
        auto graph = m_graphList[graph_idx].data();
        int buildType = m_scriptFile->itemType();

        m_stackId = Stack_User;    
        m_localVaribales.clear();
        bool ret = false;
        if(buildType == ProjectItem_scriptFlow)
        {
            ret = bulidControlFlow(graph);
            if(ret)
                addEventHandle(graph->topolist);
        }
        else if(buildType == ProjectItem_scriptParamBinding)
            ret = buildParamBinding(graph);
        else if(buildType == ProjectItem_scriptFunction)
        {
            JZNode *start_node = graph->topolist[0]->node;
            if(start_node->type() != Node_functionStart)
                continue;
            
            FunctionDefine define = scriptFile->function();            
            m_stackId = Stack_User;
            ret = bulidControlFlow(graph);
            if(ret)
            {           
                JZFunction runtime;
                runtime.addr = m_nodeInfo[start_node->id()].start;
                runtime.file = m_script->file;
                runtime.localVariables = m_localVaribales;
                m_script->functionList.push_back(define);
                m_script->runtimeInfo[define.fullName()] = runtime;
            }
        }
        buildWatchInfo(graph);
        buildRet = (buildRet && ret);
    }
    
    auto it = m_nodeInfo.begin();
    while(it != m_nodeInfo.end())
    {
        NodeInfo info;
        info.node_id = it->node_id;
        info.node_type = it->node_type;
        info.isFlow = scriptFile->getNode(info.node_id)->isFlowNode();        
        info.pcRanges = it->ranges;
        m_script->nodeInfo[it.key()] = info;

        it++;
    }
    return buildRet;
}

QString JZNodeCompiler::nodeName(JZNode *node)
{
    QString name = "node(";
    if(!node->name().isEmpty())
        name += "name=" + node->name() + ",";
    name += "id=" + QString::number(node->id()) + ")";
    return name;
}

QString JZNodeCompiler::pinName(JZNodePin *prop)
{
    if(!prop->name().isEmpty())
        return prop->name();
    else
        return "pin" + QString::number(prop->id());
}

bool JZNodeCompiler::compilerNode(JZNode *node)
{
    pushCompilerNode(node->id());

    NodeRange range;
    JZNodeIRNodeId *node_ir = new JZNodeIRNodeId();
    node_ir->id = node->id();
    node_ir->memo = node->name();
    range.start = addStatement(JZNodeIRPtr(node_ir));

    QString error;
    bool ret = node->compiler(this,error);
    if (ret)
    {        
        Q_ASSERT(node->flowOutCount() == m_currentNodeInfo->jmpList.size());
        Q_ASSERT(node->subFlowCount() == m_currentNodeInfo->jmpSubList.size());
    }
    else
        m_currentNodeInfo->error = error;
    
    range.end = currentPc() + 1;
    m_currentNodeInfo->ranges.push_back(range);
    popCompilerNode();

    return ret;
}

void JZNodeCompiler::pushCompilerNode(int id)
{
    m_compilerNodeStack.push_back(id);
    m_currentNode = m_scriptFile->getNode(id);
    m_currentNodeInfo = &m_nodeInfo[id];
    if(m_currentNode->isFlowNode())
        m_currentNodeInfo->start = m_script->statmentList.size();
}

void JZNodeCompiler::popCompilerNode()
{
    if(m_currentNode->isFlowNode())
        m_currentNodeInfo->end = m_script->statmentList.size();

    m_compilerNodeStack.pop_back();
    if(m_compilerNodeStack.size() > 0)
    {
        int id = m_compilerNodeStack.back();
        m_currentNode = m_scriptFile->getNode(id);
        m_currentNodeInfo = &m_nodeInfo[id];
    }
    else
    {
        m_currentNode = nullptr;
        m_currentNodeInfo = nullptr;
    }
}

void JZNodeCompiler::allocLocalVariable(JZNodeIRParam param)
{
    if (!param.isRef())
        return;

    auto info = m_scriptFile->localVariableInfo(param.ref());
    if (!info)
        return;

    for (int i = 0; i < m_localVaribales.size(); i++)
    {
        if (m_localVaribales[i].name == param.ref())
            return;
    }
    m_localVaribales.push_back(*info);
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
    m_graphList.push_back(graph);    
    connectGraph(graph.data(),node);
    return graph.data();
}

bool JZNodeCompiler::genGraphs()
{        
    m_graphList.clear();
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

    for(int i = 0; i < m_graphList.size(); i++)    
    {
        Graph *graph = m_graphList[i].data();        
        if(!graph->toposort())
        {
            m_error += graph->error;
            return false;
        }
    }
    return true;
}

bool JZNodeCompiler::checkGraphs()
{
    for(int i = 0; i < m_graphList.size(); i++)
    {
        Graph *graph = m_graphList[i].data();
        if(!graph->check())
        {
            m_error += graph->error;
            return false;
        }
    }
    return true;
}    

bool JZNodeCompiler::checkBuildResult()
{
    bool ok = true;

    for(int i = 0; i < m_currentGraph->topolist.size(); i++)    
    {
        int id = m_currentGraph->topolist[i]->node->id();
        auto &nodeInfo = m_nodeInfo[id];
        if(!nodeInfo.error.isEmpty())
        {
            QString name = m_currentGraph->topolist[i]->node->name();
            QString error = "link:" + m_scriptFile->itemPath() + "(" + name  +  ",id=" + QString::number(nodeInfo.node_id) + "); " + nodeInfo.error + "\n";
            m_error += error;
            ok = false;
        }        
    }
    return ok;
}

void JZNodeCompiler::addEventHandle(const QList<GraphNode*> &graph_list)
{
    // add event
    for(int node_idx = 0; node_idx < graph_list.size(); node_idx++)
    {
        JZNode *node = graph_list[node_idx]->node;
        JZNodeEvent *node_event = dynamic_cast<JZNodeEvent*>(graph_list[node_idx]->node);
        if(node_event)
        {
            QString func_name = "on_event_" + node_event->name() + "_node" + QString::number(node->id());
            FunctionDefine define;
            define.name = func_name;
            define.className = m_className;
            define.paramIn = node_event->params();

            JZFunction runtime;
            runtime.addr = m_nodeInfo[node->id()].start;
            runtime.file = m_script->file;
            runtime.localVariables = m_localVaribales;

            JZEventHandle handle;
            handle.type = node_event->eventType();
            handle.function = define;           
            if(node_event->type() == Node_singleEvent)
                handle.sender = ((JZNodeSingleEvent*)node_event)->variable();
            m_script->events.push_back(handle);        
            m_script->runtimeInfo[define.fullName()] = runtime;
        }
    }
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
        
        compilerNode(node);
    }        

    //替换 subFlowOut 为实际子节点地址, 子节点的后续节点也在此处处理
    for(int node_idx = 0; node_idx < graph_list.size(); node_idx++)
    {   
        GraphNode *graph_node = graph_list[node_idx];
        NodeCompilerInfo &info = m_nodeInfo[graph_node->node->id()];
        for(int i = 0; i < info.jmpSubList.size(); i++)
        {
            int prop = info.jmpSubList[i].prop;
            if(!graph_node->paramOut.contains(prop))
                continue;

            auto &out_list = graph_node->paramOut[prop];
            Q_ASSERT(out_list.size() == 1);

            JZNodeGemo next_gemo = out_list[0];
            JZNodeIRJmp *jmp = new JZNodeIRJmp(OP_jmp);
            jmp->jmpPc = irLiteral(m_nodeInfo[next_gemo.nodeId].start);
            replaceStatement(info.jmpSubList[i].pc,JZNodeIRPtr(jmp));

            replaceSubNode(out_list[0].nodeId,graph_node->node->id(),i);
        }                    
    }

    //替换 flowOut 为实际节点地址
    for(int node_idx = 0; node_idx < graph_list.size(); node_idx++)
    {   
        GraphNode *graph_node = graph_list[node_idx];
        NodeCompilerInfo &info = m_nodeInfo[graph_node->node->id()];
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
                jmp->jmpPc = irLiteral(m_nodeInfo[next_gemo.nodeId].start);
                replaceStatement(pc,JZNodeIRPtr(jmp));
            }
            else
            {
                if (m_scriptFile->itemType() == ProjectItem_scriptFunction &&
                    m_scriptFile->function().paramOut.size() != 0)
                {
                    QString error_tips = "输出" + QString::number(i + 1) + "需要连接return,并给与返回值";
                    m_nodeInfo[graph_node->node->id()].error = error_tips;
                    continue;
                }

                JZNodeIR *ir_return = new JZNodeIR(OP_return);
                replaceStatement(pc,JZNodeIRPtr(ir_return));
            }
        }          
    }    

    if (!checkBuildResult())
        return false;

    return true;
}

bool JZNodeCompiler::buildDataFlow(const QList<GraphNode*> &graph_list)
{   
    if(graph_list.size() == 0)
        return true;

    bool ok = true;
    //build node
    for (int graph_idx = 0; graph_idx < graph_list.size(); graph_idx++)
    {
        GraphNode *graph = graph_list[graph_idx];
        JZNode *node = graph->node;
        Q_ASSERT(!node->isFlowNode());
        
        if(!compilerNode(node))
            ok = false;
    }          
    return ok;
}

bool JZNodeCompiler::buildParamBinding(Graph *graph)
{       
    m_currentGraph = graph;
    int start = m_script->statmentList.size();
    buildDataFlow(graph->topolist);    
    addStatement(JZNodeIRPtr(new JZNodeIR(OP_return)));

    if(!checkBuildResult())
        return false;

    auto graph_list = m_currentGraph->topolist;
    for (int i = 0; i < graph_list.size(); i++)
    { 
        auto node = graph_list[i]->node;
        if(node->type() == Node_param)
        {
            JZNodeParam *node_param = (JZNodeParam*)node;
            QString param_name = node_param->variable();
            QString func_name = "on_" + param_name + "_changed";

            FunctionDefine define;
            define.name = func_name;            
            define.className = m_className;

            JZFunction runtime;
            runtime.addr = start;
            runtime.file = m_script->file;
            runtime.localVariables = m_localVaribales;

            JZEventHandle handle;
            handle.type = Event_paramChanged;
            handle.sender = param_name;
            handle.function = define;
            m_script->events.push_back(handle);
            m_script->runtimeInfo[define.fullName()] = runtime;
        }
    }
    return true;
}

void JZNodeCompiler::buildWatchInfo(Graph *graph)
{    
}

void JZNodeCompiler::replaceSubNode(int id,int parent_id,int flow_index)
{
    NodeCompilerInfo &parent_info = m_nodeInfo[parent_id];
    NodeCompilerInfo &info = m_nodeInfo[id];
    info.parentId = parent_id;
        
    //替换continue
    for(int i = 0; i < info.continueList.size(); i++)
    {        
        JZNodeIRJmp *ir_jmp = new JZNodeIRJmp(OP_jmp);
        ir_jmp->jmpPc = irLiteral(parent_info.continuePc[flow_index]);
        replaceStatement(info.continueList[i],JZNodeIRPtr(ir_jmp));
    }
    //替换break
    for(int i = 0; i < info.breakList.size(); i++)
    {
        JZNodeIRJmp *ir_jmp = new JZNodeIRJmp(OP_jmp);
        ir_jmp->jmpPc = irLiteral(parent_info.breakPc[flow_index]);
        replaceStatement(info.breakList[i],JZNodeIRPtr(ir_jmp));
    }

    //替换子节点后续
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
            jmp->jmpPc = irLiteral(m_nodeInfo[next_gemo.nodeId].start);
            replaceStatement(pc,JZNodeIRPtr(jmp));

            replaceSubNode(next_gemo.nodeId,parent_id,flow_index);
        }
        else
        {
            int continuePc = m_nodeInfo[info.parentId].continuePc[flow_index];
            JZNodeIRJmp *ir_continue = new JZNodeIRJmp(OP_jmp);            
            ir_continue->jmpPc = irLiteral(continuePc);
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

Graph *JZNodeCompiler::currentGraph()
{
    return m_currentGraph;
}

int JZNodeCompiler::currentPc()
{
    return (m_script->statmentList.size() - 1);
}

const FunctionDefine *JZNodeCompiler::function(QString name)
{
    const FunctionDefine *func = m_project->function(name);
    if(func)
        return func;
    return JZNodeFunctionManager::instance()->function(name);
}

void JZNodeCompiler::replaceStatement(int pc,JZNodeIRPtr ir)
{
    QString memo = m_script->statmentList[pc]->memo;

    Q_ASSERT(ir->pc == -1 && pc < m_script->statmentList.size());
    ir->pc = pc;    
    ir->memo = memo;
    m_script->statmentList[pc] = ir;
}

int JZNodeCompiler::addJumpNode(int prop)
{       
    Q_ASSERT(m_currentNode->prop(prop) && m_currentNode->prop(prop)->isFlow()
             && m_currentNode->prop(prop)->isOutput() );
     
    NodeCompilerInfo::Jump info;
    JZNodeIR *jmp = new JZNodeIR(OP_none);
    jmp->memo = "flow" + QString::number(m_currentNode->flowOutList().indexOf(prop));
    addStatement(JZNodeIRPtr(jmp));
    info.prop = prop;
    info.pc = jmp->pc;    
    m_currentNodeInfo->jmpList.push_back(info);
    return info.pc;
}

int JZNodeCompiler::addJumpSubNode(int prop)
{
    Q_ASSERT(m_currentNode->prop(prop) && m_currentNode->prop(prop)->isSubFlow());

    NodeCompilerInfo::Jump info;
    JZNodeIR *jmp = new JZNodeIR(OP_nop);
    jmp->memo = "subflow" + QString::number(m_currentNode->subFlowList().indexOf(prop));
    addStatement(JZNodeIRPtr(jmp));
    info.prop = prop;
    info.pc = jmp->pc;    
    m_currentNodeInfo->jmpSubList.push_back(info);
    return info.pc;
}

void JZNodeCompiler::setBreakContinue(const QList<int> &breakPc,const QList<int> &continuePc)
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

void JZNodeCompiler::addCall(const JZNodeIRParam &function, const QList<JZNodeIRParam> &paramIn,const QList<JZNodeIRParam> &paramOut)
{
    for(int i = 0; i < paramIn.size(); i++)
        addSetVariable(irId(Reg_Call+i),paramIn[i]);

    JZNodeIRCall *call = new JZNodeIRCall();
    call->function = function;
    addStatement(JZNodeIRPtr(call));

    for(int i = 0; i < paramOut.size(); i++)
        addSetVariable(paramOut[i],irId(Reg_Call+i));
}

int JZNodeCompiler::allocStack()
{
    Q_ASSERT(m_stackId < Reg_Start - 1);
    return m_stackId++;
}

void JZNodeCompiler::freeStack(int id)
{
}

const JZParamDefine *JZNodeCompiler::getVariableInfo(JZScriptFile *file,const QString &name)
{
    auto project = file->project();
    if (name.startsWith("this."))
    {
        JZScriptClassFile *class_file = project->getClassFile(file);
        auto def = JZNodeObjectManager::instance()->meta(class_file->className());
        Q_ASSERT(def);

        QString param_name = name.mid(5);
        return def->param(param_name);
    }
    else
    {
        int gap = name.indexOf(".");
        QString base_name;
        QString param_name;
        if (gap == -1) 
        {
            base_name = name;
        }
        else
        {
            base_name = name.left(gap);
            param_name = name.mid(gap + 1);
        }

        auto def = file->localVariableInfo(base_name);
        if (!def)
            def = project->globalVariableInfo(base_name);
        if (!def)
            return nullptr;

        if (param_name.isEmpty())
            return def;
        else
        {
            auto obj_def = JZNodeObjectManager::instance()->meta(def->dataType);
            return obj_def->param(param_name);
        }            
    }
}

const JZParamDefine *JZNodeCompiler::getVariableInfo(const QString &name)
{
    return getVariableInfo(m_scriptFile, name);
}

bool JZNodeCompiler::checkVariableExist(const QString &name,QString &error)
{
    if(name.isEmpty())
    {
        error = "no variable name input";
        return false;
    }

    auto info = getVariableInfo(name);
    if(!info)
    {
        error = "no sunch element " + name;
        return false;
    }
    return true;
}

bool JZNodeCompiler::checkVariableType(const QString &name,const QString &className,QString &error)
{        
    if(!checkVariableExist(name,error))
        return false;

    auto info = getVariableInfo(name);
    int needType = JZNodeObjectManager::instance()->getClassId(className);
    if(!JZNodeType::isInherits(info->dataType,needType))
    {
        error = name + " is not " + className;
        return false;
    }
    return true;    
}

bool JZNodeCompiler::addDataInput(int nodeId, int prop_id, QString &error)
{
    GraphNode* in_node = m_currentGraph->graphNode(nodeId);
    auto in_list = in_node->node->paramInList();
    for (int prop_idx = 0; prop_idx < in_list.size(); prop_idx++)
    {
        int in_prop = in_list[prop_idx];
        if (prop_id != -1 && in_prop != prop_id)
            continue;

        if (in_node->paramIn.contains(in_prop))
        {
            QList<JZNodeGemo> &gemo_list = in_node->paramIn[in_prop];
            Q_ASSERT(gemo_list.size() > 0);
            for (int i = 0; i < gemo_list.size(); i++)
            {
                GraphNode *from_node = m_currentGraph->graphNode(gemo_list[i].nodeId);
                if (from_node->node->isFlowNode())
                    break;

                Q_ASSERT(gemo_list.size() == 1);
                int from_id = paramId(gemo_list[i]);
                int to_id = paramId(in_node->node->id(), in_prop);
                addSetVariable(irId(to_id), irId(from_id));
            }
        }
        else
        {
            int to_id = paramId(in_node->node->id(), in_prop);
            auto prop = in_node->node->prop(in_prop);            
            if (!prop->value().isValid())
            {
                //成员函数可以不输入this
                if (prop_idx == 0 && in_node->node->type() == Node_function)
                {
                    JZNodeFunction *func_node = (JZNodeFunction*)in_node->node;
                    auto func = JZNodeFunctionManager::instance()->function(func_node->function());
                    if (!func->className.isEmpty() && JZNodeObjectManager::instance()->isInherits(m_className,func->className))
                    {
                        addSetVariable(irId(to_id), irThis());
                        continue;
                    }
                }

                error = pinName(prop) + " not set";
                return false;
            }            

            //对于比较，立即数类型等于其他类型
            QList<int> match_type = prop->dataType();
            if(in_node->node->type() >= Node_eq && in_node->node->type() <= Node_gt)
            {
                QList<int> other_type;
                for (int other_prop_idx = 0; other_prop_idx < in_list.size(); other_prop_idx++)
                {
                    int other_prop = in_list[other_prop_idx];
                    if (in_node->paramIn.contains(other_prop))
                    {
                        QList<JZNodeGemo> &gemo_list = in_node->paramIn[other_prop];
                        for (int i = 0; i < gemo_list.size(); i++)
                        {
                            GraphNode *from_node = m_currentGraph->graphNode(gemo_list[i].nodeId);
                            other_type = from_node->node->prop(gemo_list[i].propId)->dataType();
                        }
                    }
                }
                if (!other_type.isEmpty())
                    match_type = other_type;
            }

            addSetVariable(irId(to_id), irLiteral(JZNodeType::matchValue(prop->value(), match_type)));
        }
    }
    return true;
}

bool JZNodeCompiler::addDataInput(int nodeId,QString &error)
{
    return addDataInput(nodeId, -1, error);    
}

bool JZNodeCompiler::addFlowInput(int nodeId, int prop_id, QString &error)
{    
    Q_ASSERT(m_currentGraph->graphNode(nodeId)->node->isFlowNode() || m_currentGraph->graphNode(nodeId)->node->type() == Node_and
        || m_currentGraph->graphNode(nodeId)->node->type() == Node_or);

    QList<GraphNode*> graph_list;
    QSet<GraphNode*> graphs;

    //广度遍历所有节点
    QList<GraphNode*> in_list;
    in_list << m_currentGraph->graphNode(nodeId);

    while (in_list.size() > 0)
    {
        QList<GraphNode*> next_list;
        for (int node_idx = 0; node_idx < in_list.size(); node_idx++)
        {
            auto in_node = in_list[node_idx];
            auto it = in_node->paramIn.begin();

            //对每个节点遍历输入
            while (it != in_node->paramIn.end())
            {
                if (!in_node->node->prop(it.key())->isParam())
                {
                    it++;
                    continue;
                }
                // 对第一个输入进行过滤
                if (in_node->node->id() == nodeId && prop_id != -1 && it.key() != prop_id)
                {
                    it++;
                    continue;
                }

                QList<JZNodeGemo> &gemo_list = it.value();
                for (int i = 0; i < gemo_list.size(); i++)
                {
                    GraphNode *from_node = m_currentGraph->graphNode(gemo_list[i].nodeId);
                    if (from_node->node->isFlowNode())
                    {
                        auto n = from_node->node;
                        JZNodeEvent *node_event = dynamic_cast<JZNodeEvent*>(n);
                        if (!node_event && m_scriptFile->getConnectId(n->id(), n->flowIn()).size() == 0)
                        {
                            error += n->name() + "(" + QString::number(n->id()) + ")" + "没有连接输入流程";
                            return false;
                        }
                        continue;
                    }
                                  
                    auto from_type = from_node->node->prop(gemo_list[i].propId)->dataType();
                    auto in_type = in_node->node->prop(it.key())->dataType();
                    bool ok = JZNodeType::canConvert(from_type, in_type);
                    if (!ok)
                    {
                        error = "数据类型不匹配";
                        return false;
                    }

                    if (!graphs.contains(from_node))
                    {
                        graphs << from_node;
                        //and 和 or 节点按照 flow 节点处理
                        if(from_node->node->type() != Node_and && from_node->node->type() != Node_or)
                            next_list << from_node;
                    }
                }
                it++;
            }
        }
        in_list = next_list;
    }

    //要计算的节点，从topolist 获取保证计算顺序
    for (int i = 0; i < m_currentGraph->topolist.size(); i++)
    {
        auto node = m_currentGraph->topolist[i];
        if (graphs.contains(node))
            graph_list.push_back(node);
    }
    if (!buildDataFlow(graph_list))
        return false;
    if (!addDataInput(nodeId, error))
        return false;

    return true;
}

bool JZNodeCompiler::addFlowInput(int nodeId,QString &error)
{
    return addFlowInput(nodeId, -1, error);    
}

void JZNodeCompiler::addFlowOutput(int nodeId)
{
    auto graph = m_currentGraph->graphNode(nodeId);
    Q_ASSERT(graph);
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

void JZNodeCompiler::addAllocLocal(JZParamDefine *def)
{
    JZNodeIRAlloc *alloc = new JZNodeIRAlloc();
    alloc->allocType = JZNodeIRAlloc::Stack;
    alloc->name = def->name;
    alloc->dataType = def->dataType;
    if (def->value.isValid())
        alloc->value = def->value;
    else if (JZNodeType::isBaseType(def->dataType))
        alloc->value = QVariant(JZNodeType::typeToQMeta(def->dataType));
    addStatement(JZNodeIRPtr(alloc));
}

void JZNodeCompiler::addAssert(const JZNodeIRParam &tips)
{
    JZNodeIRAssert *assert = new JZNodeIRAssert();
    assert->tips = tips;
    addStatement(JZNodeIRPtr(assert));
}

void JZNodeCompiler::allocFunctionVariable()
{
    auto define = m_scriptFile->function();
    for(int i = 0; i < define.paramIn.size(); i++)
    {
        if (i == 0 && !define.className.isEmpty())
        {
            addSetVariable(irRef("this"), irId(Reg_Call + i));
        }
        else
        {
            m_localVaribales.push_back(define.paramIn[i]);
            addSetVariable(irRef(define.paramIn[i].name), irId(Reg_Call + i));            
        }
    }
}

int JZNodeCompiler::addNop()
{
    JZNodeIR *nop = new JZNodeIR(OP_nop);
    return addStatement(JZNodeIRPtr(nop));
}

int JZNodeCompiler::addExpr(const JZNodeIRParam &dst,const JZNodeIRParam &p1,const JZNodeIRParam &p2,int op)
{
    JZNodeIRExpr *expr = new JZNodeIRExpr(op);
    expr->src1 = p1;
    expr->src2 = p2;
    expr->dst = dst;
    return addStatement(JZNodeIRPtr(expr));
}

int JZNodeCompiler::addCompare(const JZNodeIRParam &p1,const JZNodeIRParam &p2,int op)
{
    return addExpr(irId(Reg_Cmp),p1,p2,op);    
}

int JZNodeCompiler::addSetVariable(const JZNodeIRParam &dst,const JZNodeIRParam &src)
{
    Q_ASSERT(src.type != JZNodeIRParam::None && dst.type != JZNodeIRParam::None);
    Q_ASSERT(dst.type != JZNodeIRParam::Literal);
    allocLocalVariable(dst);
    allocLocalVariable(src);

    JZNodeIRSet *op = new JZNodeIRSet();    
    op->dst = dst;
    op->src = src;
    return addStatement(JZNodeIRPtr(op));
}
