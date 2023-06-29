#include "JZNodeCompiler.h"
#include "JZNodeValue.h"
#include <QVector>
#include <QSet>
#include "JZNodeFunctionManager.h"

//NodeCompilerInfo
JZNodeCompiler::NodeCompilerInfo::NodeCompilerInfo()
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

void JZNodeCompiler::init(JZScriptFile *scriptFile,JZNodeScript *result)
{
    m_scriptFile = scriptFile;
    m_project = scriptFile->project();
    m_script = result;
    m_script->clear();
    m_nodeGraph.clear();
    m_nodeInfo.clear();
}

bool JZNodeCompiler::genGraphs(JZScriptFile *scriptFile,JZNodeScript *result)
{
    init(scriptFile,result);
    if(!genGraphs())
        return false;

    return true;
}

bool JZNodeCompiler::build(JZScriptFile *scriptFile,JZNodeScript *result)
{        
    if(!genGraphs(scriptFile,result))
        return false;
    if(!checkGraphs())
        return false;

    m_script->file = scriptFile->itemPath();
    m_script->nodeInfo.clear();

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
    for(int graph_idx = 0; graph_idx < m_script->graphs.size(); graph_idx++)
    {
        auto graph = m_script->graphs[graph_idx].data();
        int buildType = m_scriptFile->itemType();

        m_stackId = Stack_User;    
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

            m_script->localVariable.clear();
            FunctionDefine define = scriptFile->function();
            for(int i = 0; i < define.paramIn.size(); i++)
            {
                QString name = define.paramIn[i].name;
                m_script->localVariable[name] = (Stack_User + i);
            }
            m_stackId = Stack_User + define.paramIn.size();
            ret = bulidControlFlow(graph);
            if(ret)
            {                
                define.addr = m_nodeInfo[start_node->id()].start;
                define.script = m_script->file;
                m_script->functionList.push_back(define);
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
        info.start = it->start;
        info.end = it->end;
        info.error = it->error;
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

    JZNodeIRNodeId *node_ir = new JZNodeIRNodeId();
    node_ir->id = node->id();
    addStatement(JZNodeIRPtr(node_ir));

    QString error;
    bool ret = node->compiler(this,error);
    if(!ret)
        m_currentNodeInfo->error = error;

    popCompilerNode();
    return ret;
}

void JZNodeCompiler::pushCompilerNode(int id)
{
    m_compilerNodes.push_back(id);
    m_currentNode = m_scriptFile->getNode(id);
    m_currentNodeInfo = &m_nodeInfo[id];
    if(m_currentNode->isFlowNode())
        m_currentNodeInfo->start = m_script->statmentList.size();
}

void JZNodeCompiler::popCompilerNode()
{
    if(m_currentNode->isFlowNode())
        m_currentNodeInfo->end = m_script->statmentList.size();

    m_compilerNodes.pop_back();
    if(m_compilerNodes.size() > 0)
    {
        int id = m_compilerNodes.back();
        m_currentNode = m_scriptFile->getNode(id);
        m_currentNodeInfo = &m_nodeInfo[id];
    }
    else
    {
        m_currentNode = nullptr;
        m_currentNodeInfo = nullptr;
    }
}

QString JZNodeCompiler::error()
{
    return m_error;
}

JZNodeIRParam JZNodeCompiler::localVariable(JZNodeIRParam param)
{
    QString name = param.ref();
    if(!param.ref().contains("."))
    {
        Q_ASSERT(m_script->localVariable.contains(name));
        return irId(m_script->localVariable[name]);
    }
    
    QStringList list = name.split(".");
    Q_ASSERT(m_script->localVariable.contains(list[0]));
    list[0] = QString::number(m_script->localVariable[list[0]]);
    return irRef(list.join("."));
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

bool JZNodeCompiler::checkBuildResult()
{
    bool ok = true;

    auto it = m_nodeInfo.begin();
    while(it != m_nodeInfo.end())
    {
        if(!it->error.isEmpty())
        {
            m_error += "node" + QString::number(it->node_id) + ": " + it->error + "\n";
            ok = false;
        }
        it++;
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
            define.paramIn = node_event->params();
            define.addr = m_nodeInfo[node->id()].start;
            define.script = m_script->file;

            JZEventHandle handle;
            handle.type = node_event->eventType();
            handle.function = define;           
            if(node_event->type() == Node_singleEvent)
                handle.sender = ((JZNodeSingleEvent*)node_event)->variable();
            m_script->events.push_back(handle);            
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
    if(!checkBuildResult())
        return false;

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
            jmp->jmpPc = m_nodeInfo[next_gemo.nodeId].start;
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
    addReturn();

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
            define.addr = start;
            define.script = m_script->file;

            JZEventHandle handle;
            handle.type = Event_paramChanged;
            handle.sender = param_name;
            handle.function = define;
            m_script->events.push_back(handle);
        }
    }
    return true;
}

void JZNodeCompiler::buildWatchInfo(Graph *graph)
{
    auto graph_list = m_currentGraph->topolist;
    for(int node_idx = 0; node_idx < graph_list.size(); node_idx++)
    {
       auto node = graph_list[node_idx];
       auto it = node->paramOut.begin();
       while(it != node->paramOut.end())
       {
           auto pin = graph_list[node_idx]->node->prop(it.key());
           if(pin->isParam())
           {
                auto &out_list = it.value();
                for(int i = 0; i < out_list.size(); i++)
                {
                    auto node = m_scriptFile->getNode(out_list[i].nodeId);
                    if(node->type() == Node_print)
                    {
                        m_script->watchList.push_back(JZNodeIRParam());
                    }

                }
           }
           it++;
       }
    }
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
        ir_jmp->jmpPc = parent_info.continuePc[flow_index];
        replaceStatement(info.continueList[i],JZNodeIRPtr(ir_jmp));
    }
    //替换break
    for(int i = 0; i < info.breakList.size(); i++)
    {
        JZNodeIRJmp *ir_jmp = new JZNodeIRJmp(OP_jmp);
        ir_jmp->jmpPc = parent_info.breakPc[flow_index];
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
    ir->nodeId = m_currentNode->id();
    m_script->statmentList.push_back(ir);
    return ir->pc;
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
    Q_ASSERT(ir->pc == -1 && pc < m_script->statmentList.size());
    ir->pc = pc;
    m_script->statmentList[pc] = ir;
}

int JZNodeCompiler::addJumpNode(int prop)
{       
    Q_ASSERT(m_currentNode->prop(prop) && m_currentNode->prop(prop)->isFlow()
             && m_currentNode->prop(prop)->isOutput() );
     
    NodeCompilerInfo::Jump info;
    JZNodeIR *jmp = new JZNodeIR(OP_none);
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

void JZNodeCompiler::addCall(JZNodeIRParam function,QList<JZNodeIRParam> paramIn,QList<JZNodeIRParam> paramOut)
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

bool JZNodeCompiler::checkVariableExist(QString name,QString &error)
{
    if(name.isEmpty())
    {
        error = "no variable name input";
        return false;
    }

    auto info = m_scriptFile->getVariableInfo(name);
    if(!info)
    {
        error = "no sunch element " + name;
        return false;
    }
    return true;
}

bool JZNodeCompiler::checkVariableType(QString name,QString className,QString &error)
{        
    if(!checkVariableExist(name,error))
        return false;

    auto info = m_scriptFile->getVariableInfo(name);
    int needType = JZNodeObjectManager::instance()->getClassId(className);
    if(!JZNodeType::isInherits(info->dataType,needType))
    {
        error = name + " is not " + className;
        return false;
    }
    return true;    
}

bool JZNodeCompiler::addDataInput(int nodeId,QString &error)
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
            if(!prop->value().isValid())
            {
                error = pinName(prop) + " not set";
                return false;
            }

            int to_id = paramId(in_node->node->id(),in_prop);
            addSetVariable(irId(to_id),irLiteral(prop->value()));
        }
    }    
    return true;
}

bool JZNodeCompiler::addFlowInput(int nodeId,QString &error)
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
    if(!buildDataFlow(graph_list))
        return false;
    if(!addDataInput(nodeId,error))
        return false;
    
    return true;
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
    Q_ASSERT(src.type != JZNodeIRParam::None && dst.type != JZNodeIRParam::None);
    Q_ASSERT(dst.type != JZNodeIRParam::Literal);

    JZNodeIRSet *op = new JZNodeIRSet();    
    op->dst = dst;
    op->src = src;
    return addStatement(JZNodeIRPtr(op));
}
