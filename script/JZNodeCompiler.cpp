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
    allSubReturn = -1;
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

CompilerInfo JZNodeCompiler::compilerInfo()
{
    CompilerInfo info;
    info.nodeInfo = m_nodeInfo;    
    return info;
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
        m_currentGraph = graph;                

        int buildType = m_scriptFile->itemType();
        m_stackId = Stack_User;    
        m_localVaribales.clear();

        bool ret = false;
        if(buildType == ProjectItem_scriptFlow)
        {
            ret = bulidControlFlow();
            if(ret)
                addEventHandle(graph->topolist);
        }
        else if(buildType == ProjectItem_scriptParamBinding)
            ret = buildParamBinding();
        else if(buildType == ProjectItem_scriptFunction)
        {
            JZNode *start_node = graph->topolist[0]->node;
            if(start_node->type() != Node_functionStart)
                continue;
            
            FunctionDefine define = scriptFile->function();            
            m_stackId = Stack_User;

            int start_pc = nextPc();
            ret = bulidControlFlow();
            if(ret)
            {           
                addFunction(define, start_pc);
            }
        }

        //add display
        if (ret)
        {
            for (int node_idx = 0; node_idx < graph->topolist.size(); node_idx++)
            {
                auto node = graph->topolist[node_idx];
                if (node->node->type() != Node_display)
                    continue;

                auto in_map = node->paramIn;
                auto it = in_map.begin();
                while (it != in_map.end())
                {



                    it++;
                }
            }
        }
        buildRet = (buildRet && ret);
    }
    
    //编译结果
    auto it = m_nodeInfo.begin();
    while(it != m_nodeInfo.end())
    {
        auto node = scriptFile->getNode(it->node_id);

        NodeInfo info;
        info.node_id = it->node_id;
        info.node_type = it->node_type;
        info.isFlow = node->isFlowNode();
        info.pcRanges = it->ranges;

        auto in_list = node->paramInList();
        for (int i = 0; i < in_list.size(); i++)
        {
            info.paramInId.push_back(paramId(node->id(), in_list[i]));
            info.paramIn.push_back(node->prop(in_list[i])->name());
        }
        auto out_list = node->paramOutList();
        for (int i = 0; i < out_list.size(); i++)
        {
            info.paramOutId.push_back(paramId(node->id(), out_list[i]));
            info.paramOut.push_back(node->prop(out_list[i])->name());
        }
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

void JZNodeCompiler::setOutPinTypeDefault(JZNode *node)
{    
    auto list = node->paramOutList();
    for (int i = 0; i < list.size(); i++)
    {
        int pin_id = list[i];
        auto pin = node->prop(pin_id);
        if(pin->dataType().size() == 1)
            m_nodeInfo[node->id()].pinType[pin_id] = pin->dataType().front();
    }
}

void JZNodeCompiler::setPinType(int node_id, int prop_id, int type)
{
    Q_ASSERT(type != Type_none);
    auto &info = m_nodeInfo[node_id];
    info.pinType[prop_id] = type;
}

int JZNodeCompiler::pinType(int node_id, int prop_id)
{
    Q_ASSERT(m_nodeInfo[node_id].pinType.contains(prop_id));
    auto &info = m_nodeInfo[node_id];
    return info.pinType[prop_id];
}

int JZNodeCompiler::pinType(JZNodeGemo gemo)
{
    return pinType(gemo.nodeId, gemo.propId);
}

bool JZNodeCompiler::compilerNode(JZNode *node)
{   
    if (node->type() == Node_display)
        return true;

    pushCompilerNode(node->id());

    NodeRange range;
    JZNodeIRNodeId *node_ir = new JZNodeIRNodeId();
    node_ir->id = node->id();
    node_ir->memo = node->name() + "(" + QString::number(node->id()) + ")";
    range.start = addStatement(JZNodeIRPtr(node_ir));    

    QString error;
    bool ret = node->compiler(this,error);
    if (ret)
    {                
        setOutPinTypeDefault(node);        
        
        Q_ASSERT(node->flowOutCount() == m_currentNodeInfo->jmpList.size());
        Q_ASSERT(node->subFlowCount() == m_currentNodeInfo->jmpSubList.size());
        auto out_list = node->paramOutList();
        for (int i = 0; i < out_list.size(); i++)
        {
            Q_ASSERT(m_nodeInfo[node->id()].pinType.contains(out_list[i]));            
        }        
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

void JZNodeCompiler::addLocalVariable(JZNodeIRParam param)
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
            int start_pc = m_nodeInfo[node->id()].start;            

            FunctionDefine def = node_event->function();
            if(!def.isNull())
                addFunction(def, start_pc);
        }
    }
}

void JZNodeCompiler::addFunction(const FunctionDefine &define, int node_pc)
{    
    int start_pc = nextPc();    
    for (int i = 0; i < define.paramIn.size(); i++)
    {
        addAllocLocal(&define.paramIn[i]);
        addSetVariable(irRef(define.paramIn[i].name), irId(Reg_Call + i));
    }
    for (int i = 0; i < m_localVaribales.size(); i++)
        addAllocLocal(&m_localVaribales[i]);

    JZNodeIRJmp *jmp = new JZNodeIRJmp(OP_jmp);
    jmp->jmpPc = irLiteral(node_pc);
    addStatement(JZNodeIRPtr(jmp));

    m_script->functionList.push_back(define);
    m_script->functionList.back().addr = start_pc;
    m_script->functionList.back().file = m_script->file;

    JZFunction runtime;
    runtime.file = m_script->file;
    runtime.localVariables = m_localVaribales;

    m_script->runtimeInfo[define.fullName()] = runtime;
}

bool JZNodeCompiler::checkPinInType(int node_id, int prop_check_id, QString &error)
{                    
    GraphNode *graph = m_currentGraph->graphNode(node_id);    
    
    error.clear();
    //获得输入类型    
    QMap<int, int> in_type;
    auto param_in = graph->node->paramInList();
    for (int param_idx = 0; param_idx < param_in.size(); param_idx++)
    {
        auto prop_in_id = param_in[param_idx];
        if (prop_check_id != -1 && prop_in_id != prop_check_id)
            continue;

        auto pin = graph->node->prop(prop_in_id);
            
        int pin_type = Type_none;
        if (graph->paramIn.contains(prop_in_id))  //有输入
        {
            QList<int> from_type;
            auto from_list = graph->paramIn[prop_in_id];
            for (int i = 0; i < from_list.size(); i++)
            {   
                auto from_gemo = from_list[i];
                if (!m_nodeInfo[from_gemo.nodeId].pinType.contains(from_gemo.propId))
                {
                    error = "前置依赖未设置";
                    return false;
                }

                from_type.push_back(pinType(from_gemo));
            }
            pin_type = JZNodeType::matchType(pin->dataType(),from_type);
            if (pin_type == Type_none)
                error = "无法确定输入类型";                    
        }
        else
        {
            if (pin->value().isValid()) //默认值
            {                    
                pin_type = JZNodeType::matchType(pin->dataType(), pin->value());
            }
            else
            {
                //是不是this, this在当前class内可以不设置
                bool is_this = false;
                int prop_idx = graph->node->paramInList().indexOf(prop_in_id);
                if (prop_idx == 0 && graph->node->type() == Node_function)
                {
                    JZNodeFunction *func_node = (JZNodeFunction*)graph->node;
                    auto func = JZNodeFunctionManager::instance()->function(func_node->function());
                    if (!func->className.isEmpty() && JZNodeObjectManager::instance()->isInherits(m_className, func->className))
                    {
                        is_this = true;
                        pin_type = JZNodeObjectManager::instance()->getClassId(func->className);
                    }
                }
                    
                if(!is_this)
                    error = "未设置";                                 
            }
        }

        if (pin_type != Type_none)
            in_type[prop_in_id] = pin_type;
    }

calc_end:
    m_nodeInfo[node_id].pinType = in_type;
    return error.isEmpty();
}

bool JZNodeCompiler::bulidControlFlow()
{    
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
    bool is_return_value = (m_scriptFile->itemType() == ProjectItem_scriptFunction &&
        m_scriptFile->function().paramOut.size() != 0);
    for(int node_idx = 0; node_idx < graph_list.size(); node_idx++)
    {   
        GraphNode *graph_node = graph_list[node_idx];
        NodeCompilerInfo &info = m_nodeInfo[graph_node->node->id()];
        if(info.parentId != -1)
            continue;

        info.allSubReturn = isAllFlowReturn(info.node_id, true);
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
                if (is_return_value && !info.allSubReturn)
                {
                    QString error_tips = "输出" + QString::number(i + 1) + "需要连接return,并给与返回值";
                    m_nodeInfo[graph_node->node->id()].error = error_tips;
                    continue;
                }

                if (!is_return_value)
                {
                    JZNodeIR *ir_return = new JZNodeIR(OP_return);
                    replaceStatement(pc, JZNodeIRPtr(ir_return));
                }
                else
                {
                    JZNodeIRAssert *ir_assert = new JZNodeIRAssert();
                    ir_assert->tips = irLiteral("unexception flow return");
                    replaceStatement(pc, JZNodeIRPtr(ir_assert));
                }                
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

bool JZNodeCompiler::buildParamBinding()
{           
    int start = m_script->statmentList.size();
    buildDataFlow(m_currentGraph->topolist);
    addStatement(JZNodeIRPtr(new JZNodeIR(OP_return)));

    if(!checkBuildResult())
        return false;

    addEventHandle(m_currentGraph->topolist);
    return true;
}

int JZNodeCompiler::isAllFlowReturn(int id, bool root)
{
    NodeCompilerInfo &info = m_nodeInfo[id];
    auto graph_node = m_currentGraph->graphNode(id);
    if (info.allSubReturn != -1)
        return info.allSubReturn;

    int all_sub_return = 0;
    if (info.jmpList.size() == 0 && info.jmpSubList.size() == 0)
    {
        all_sub_return = (info.node_type == Node_return);        
    }
    else
    {
        //子节点
        int ret_count = 0;
        for (int i = 0; i < info.jmpSubList.size(); i++)
        {
            int prop = info.jmpSubList[i].prop;
            if (!graph_node->paramOut.contains(prop))
                continue;

            auto &out_list = graph_node->paramOut[prop];
            if (isAllFlowReturn(out_list[0].nodeId, false))
                ret_count++;
        }
        if (ret_count != 0 && ret_count == info.jmpSubList.size())
            all_sub_return = 1;

        //兄弟节点
        if (all_sub_return != 1 && !root)
        {
            ret_count = 0;
            for (int i = 0; i < info.jmpList.size(); i++)
            {
                int prop = info.jmpList[i].prop;
                if (!graph_node->paramOut.contains(prop))
                    continue;

                auto &out_list = graph_node->paramOut[prop];
                if (isAllFlowReturn(out_list[0].nodeId, false))
                    ret_count++;
            }
            if (ret_count == info.jmpList.size())
                all_sub_return = 1;
        }
    }

    info.allSubReturn = all_sub_return;
    return info.allSubReturn;
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

JZScriptFile *JZNodeCompiler::currentFile()
{
    return m_scriptFile;
}

Graph *JZNodeCompiler::currentGraph()
{
    return m_currentGraph;
}

int JZNodeCompiler::currentPc()
{
    return (m_script->statmentList.size() - 1);
}

int JZNodeCompiler::nextPc()
{
    return m_script->statmentList.size();
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
    if (function.isLiteral())
    {
        auto func = JZNodeFunctionManager::instance()->function(function.literal().toString());
        Q_ASSERT(func);
    }

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
    if (!checkPinInType(nodeId, prop_id, error))
        return false;
 
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
                Q_ASSERT(false);
                return false;
            }            
            
            int match_type = m_nodeInfo[nodeId].pinType[in_prop];
            addSetVariable(irId(to_id), irLiteral(JZNodeType::matchValue(match_type,prop->value())));
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
    if (!addDataInput(nodeId, prop_id, error))
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

void JZNodeCompiler::addAllocLocal(const JZParamDefine *def, const JZNodeIRParam &value)
{
    JZNodeIRAlloc *alloc = new JZNodeIRAlloc();
    alloc->allocType = JZNodeIRAlloc::Stack;
    alloc->name = def->name;
    alloc->dataType = def->dataType;
    if (value.isNull())
        alloc->value = irLiteral(def->initialValue());
    else
        alloc->value = value;
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
    addLocalVariable(dst);
    addLocalVariable(src);

    JZNodeIRSet *op = new JZNodeIRSet();    
    op->dst = dst;
    op->src = src;
    return addStatement(JZNodeIRPtr(op));
}