﻿#include <QVector>
#include <QSet>
#include <QDebug>
#include "JZNodeCompiler.h"
#include "JZNodeValue.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeFunction.h"
#include "JZClassItem.h"
#include "JZNodeUtils.h"
#include "JZNodeBuilder.h"

// GraphNode
GraphNode::GraphNode()
{
    node = nullptr;
    isReached = false;
}

QList<JZNodeGemo> GraphNode::outPinList()
{
    QList<JZNodeGemo> next;
    auto it_out = paramOut.begin();
    while (it_out != paramOut.end())
    {
        auto &out_list = it_out.value();
        for (int j = 0; j < out_list.size(); j++)
            next << out_list[j];

        it_out++;
    }
    return next;
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
    return pin(gemo.nodeId, gemo.pinId);
}

JZNodePin *Graph::pin(int nodeId, int pinId)
{
    auto n = node(nodeId);
    if (n)
        return n->pin(pinId);
    else
        return nullptr;
}

void Graph::walkParamNode(GraphNode *node)
{
    Q_ASSERT(node->node->isParamNode());

    QList<GraphNode*> cur_list;
    QList<GraphNode*> next_list;
    cur_list.push_back(node);

    while (cur_list.size() > 0)
    {
        for (int cur_idx = 0; cur_idx < cur_list.size(); cur_idx++)
        {
            auto cur = cur_list[cur_idx];
            cur->isReached = true;

            auto in_it = cur->paramIn.begin();
            while (in_it != cur->paramIn.end())
            {
                auto &in_list = in_it.value();
                for (int in_idx = 0; in_idx < in_list.size(); in_idx++)
                {
                    auto pin = this->pin(in_list[in_idx]);
                    if (!pin->isParam())
                        continue;

                    auto in_node = graphNode(in_list[in_idx].nodeId);
                    if (!in_node->isReached && in_node->node->isParamNode())
                    {
                        if(!next_list.contains(in_node))
                            next_list << in_node;
                    }
                }
                in_it++;
            }    
        }

        std::swap(next_list, cur_list);
        next_list.clear();
    }
}

void Graph::walkFlowNode(GraphNode *node)
{
    QList<GraphNode*> cur_list;
    QList<GraphNode*> next_list;    
    cur_list.push_back(node);

    while (cur_list.size() > 0)
    {
        for (int cur_idx = 0; cur_idx < cur_list.size(); cur_idx++)
        {
            auto cur = cur_list[cur_idx];
            cur->isReached = true;

            auto in_it = cur->paramIn.begin();
            while (in_it != cur->paramIn.end())
            {
                auto &in_list = in_it.value();
                for (int in_idx = 0; in_idx < in_list.size(); in_idx++)
                {
                    auto pin = this->pin(in_list[in_idx]);
                    if (!pin->isParam())
                        continue;

                    auto in_node = graphNode(in_list[in_idx].nodeId);
                    if (!in_node->isReached && in_node->node->isParamNode())
                        walkParamNode(in_node);
                }
                in_it++;
            }

            auto out_it = cur->paramOut.begin();
            while (out_it != cur->paramOut.end())
            {
                auto &out_list = out_it.value();
                for (int out_idx = 0; out_idx < out_list.size(); out_idx++)
                {
                    auto pin = this->pin(out_list[out_idx]);
                    if (!(pin->isFlow() || pin->isSubFlow()))
                        continue;

                    auto out_node = graphNode(out_list[out_idx].nodeId);
                    if (!out_node->isReached)
                    {
                        if(!next_list.contains(out_node))
                            next_list << out_node;
                    }
                }
                out_it++;
            }            
        }

        std::swap(next_list, cur_list);
        next_list.clear();
    }
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
    QMap<int, int> nodeMap; //入度
    for (auto v : m_nodes)
        nodeMap[v.data()->node->id()] = 0;
    for (auto v : m_nodes)
    {
        auto next = v->outPinList();
        for(int j = 0; j < next.size(); j++)
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
                auto next = cur_node->outPinList();
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
        
        std::sort(tmp.begin(), tmp.end(), [](const GraphNode *n1, const GraphNode *n2){ 
            return n1->node->id() < n2->node->id(); 
        });
        result.append(tmp);
    }
    topolist = result;    
    return true;
}

//JZNodeIRFlowOut
JZNodeIRFlowOut::JZNodeIRFlowOut()
{
    type = OP_ComilerFlowOut;
    fromId = -1;
    toId = -1;
}

//JZNodeIRStackInit
JZNodeIRStackInit::JZNodeIRStackInit()
{
    type = OP_ComilerStackInit;
}

//NodeCompilerInfo
NodeCompilerInfo::NodeCompilerInfo()
{
    node_id = -1;
    node_type = Node_none;    
    start = -1;
    parentId = -1;    
    allSubReturn = -1;    
    autoAddDebugStart = true;
}

//NodeCompilerStack
JZNodeCompiler::NodeCompilerStack::NodeCompilerStack()
{
    nodeInfo = nullptr;
    isFlow = false;
    start = -1;
    debugStart = -1;
}

// JZNodeCompiler
int JZNodeCompiler::paramId(int nodeId,int pinId)
{
    Q_ASSERT(nodeId >= 0 && pinId >= 0);
    return nodeId * 100 + pinId;
}

int JZNodeCompiler::paramId(const JZNodeGemo &gemo)
{
    return paramId(gemo.nodeId,gemo.pinId);
}

QString JZNodeCompiler::paramName(int id)
{    
    if(id < Stack_User)
        return QString().asprintf("Node%d.%d",id/100,id%100);
    else if(id < Reg_Start)
        return QString().asprintf("Stack%d",id - Stack_User);
    else if(id == Reg_Cmp)
        return "Reg_Cmp";    
    else if(id >= Reg_CallIn && id < Reg_CallOut)
        return "Reg_CallIn_" + QString::number(id - Reg_CallIn);
    else if(id >= Reg_CallOut)
        return "Reg_CallOut_" + QString::number(id - Reg_CallOut);

    return QString();        
}

JZNodeGemo JZNodeCompiler::paramGemo(int id)
{
    if(id < Stack_User)
        return JZNodeGemo(id/100,id%100);
    else
        return JZNodeGemo();
}

VariableCoor JZNodeCompiler::variableCoor(JZScriptItem *file, QString name)
{
    if (name.startsWith("this."))
        return Variable_member;
    else
    {
        QString base = name.split(",")[0];
        if (file->localVariable(base))
            return Variable_local;

        if (file->project()->globalVariable(base))
            return Variable_global;
        else
        {
            auto class_file = file->getClassFile();
            if (class_file && class_file->memberVariable(name,true))
                return Variable_member;
            else
                return Variable_none;
        }
    }
}

JZNodeCompiler::JZNodeCompiler()
{    
    m_script = nullptr;
    m_scriptFile = nullptr;
    m_originGraph = nullptr;
    m_statmentList = nullptr;
    m_regCallFunction = nullptr;
    m_builder = nullptr;

    m_buildGraph = GraphPtr(new Graph());
}

JZNodeCompiler::~JZNodeCompiler()
{
}

void JZNodeCompiler::init(JZScriptItem *scriptFile)
{
    m_scriptFile = scriptFile;
    m_project = scriptFile->project();        
    m_script = nullptr;    
    m_originGraph = nullptr;
    m_statmentList = nullptr;
    m_regCallFunction = nullptr;    

    m_nodeGraph.clear();
    m_nodeInfo.clear();
    m_className.clear();    
    m_graphList.clear();        
    m_error.clear();
    resetStack();
}

void JZNodeCompiler::setBuilder(JZNodeBuilder *builder)
{
    m_builder = builder;
}

bool JZNodeCompiler::genGraphs(JZScriptItem *scriptFile, QVector<GraphPtr> &list)
{
    init(scriptFile);
    if(!genGraphs())
        return false;

    list = m_graphList;
    return true;
}

CompilerResult JZNodeCompiler::compilerResult()
{    
    return m_compilerInfo;
}

void JZNodeCompiler::updateBuildGraph(const QList<GraphNode*> &root_list)
{
    m_buildGraph->clear();

    //clone
    auto it = m_originGraph->m_nodes.begin();
    while (it != m_originGraph->m_nodes.end())
    {
        GraphNode *node = new GraphNode();
        *node = *it.value().data();
        m_buildGraph->m_nodes[it.key()] = GraphNodePtr(node);
        it++;
    }
    
    for (int i = 0; i < m_originGraph->topolist.size(); i++)
    {
        int node_id = m_originGraph->topolist[i]->node->id();
        auto node = m_buildGraph->m_nodes[node_id].data();
        m_buildGraph->topolist.push_back(node);
    }

    //filter
    for (auto node : m_buildGraph->topolist)
        node->isReached = false;
    for (int i = 0; i < root_list.size(); i++)
    {
        int node_id = root_list[i]->node->id();
        auto graph_node = m_buildGraph->m_nodes[node_id].data();
        if (graph_node->node->flowInCount() == 0 && graph_node->node->isFlowNode())
            m_buildGraph->walkFlowNode(graph_node);
    }
   
    QList<GraphNode*> topolist;
    for (int i = 0; i < m_buildGraph->topolist.size(); i++)
    {
        if (m_buildGraph->topolist[i]->isReached)
            topolist << m_buildGraph->topolist[i];
    }
    m_buildGraph->topolist = topolist;

    //unlink
    auto unlinkNoReached = [this](QMap<int, QList<JZNodeGemo>> &gemo_map)
    {   
        auto map_it = gemo_map.begin();
        while (map_it != gemo_map.end())
        {
            auto &gemo_list = map_it.value();
            for (int i = gemo_list.size() - 1; i >= 0; i--)
            {
                GraphNode *other = m_buildGraph->graphNode(gemo_list[i].nodeId);
                if (!other->isReached)
                    gemo_list.removeAt(i);
            }
            if (gemo_list.size() == 0)
                map_it = gemo_map.erase(map_it);
            else
                map_it++;
        }
    };        
    for (int i = 0; i < m_buildGraph->topolist.size(); i++)
    {
        auto node = m_buildGraph->topolist[i];
        unlinkNoReached(node->paramIn);
        unlinkNoReached(node->paramOut);
    }
}

bool JZNodeCompiler::checkFunction()
{
    JZNode *start_node = m_originGraph->topolist[0]->node;

    QString check_error;
    auto class_file = m_scriptFile->getClassFile(); 
    if(class_file)
    {
        auto meta = JZNodeObjectManager::instance()->meta(class_file->className()); 
        meta->checkFunction(m_scriptFile->function().name,check_error);
    }

    if(check_error.isEmpty())
    {
        QStringList list = m_scriptFile->localVariableList(true);
        for(int i = 0; i < list.size(); i++)
        {
            for(int j = 0; j < list.size(); j++)
            {
                if(i != j && list[i] == list[j])
                {
                    check_error = list[i] + "重定义";
                    return false;
                }
            }
        }
    }

    if(!check_error.isEmpty())
    {
        NodeCompilerInfo info;
        info.node_id = start_node->id();
        info.error = check_error;
        m_nodeInfo[start_node->id()] = info;
    }

    return check_error.isEmpty();
}

bool JZNodeCompiler::build(JZScriptItem *scriptFile,JZNodeScript *result)
{        
    init(scriptFile);
    if (!genGraphs())
        return false;
    if(!checkGraphs())
        return false;

    m_script = result;
    m_script->clear();
    m_script->file = scriptFile->itemPath();    
    m_compilerInfo = CompilerResult();
    m_compilerInfo.result = false;

    JZScriptClassItem *class_file = m_project->getItemClass(scriptFile);
    if (class_file)
    {
        m_className = class_file->className();
        m_script->className = m_className;    
    }
           
    bool buildRet = true;
    for(int graph_idx = 0; graph_idx < m_graphList.size(); graph_idx++)
    {
        m_originGraph = m_graphList[graph_idx].data();                
        
        int buildType = m_scriptFile->itemType();

        resetStack();
        if (buildType == ProjectItem_scriptParamBinding)
        {
            buildParamBinding();
        }
        else if (buildType == ProjectItem_scriptFunction)
        {   
            //未连接的语句
            JZNode *start_node = m_originGraph->topolist[0]->node;
            if(start_node->type() != Node_functionStart)
            {
                JZNode *flow_node = start_node;
                for(int i = 0; i < m_originGraph->topolist.size(); i++)
                {
                    if(m_originGraph->topolist[i]->node->isFlowNode())
                    {
                        flow_node = m_originGraph->topolist[i]->node;
                        break;
                    }
                }

                NodeCompilerInfo info;
                info.node_id = flow_node->id();
                info.error = "孤立节点,未连接";
                m_nodeInfo[info.node_id] = info;
                goto buildEnd;
            }

            QList<GraphNode*> event_list;            
            event_list.push_back(m_originGraph->topolist[0]);
            if(!checkFunction())
                goto buildEnd;

            //确保每个flowIn 都被连接
            for(int i = 0; i < m_originGraph->topolist.size(); i++)
            {
                auto graph_node = m_originGraph->topolist[i];
                if(graph_node->node->flowInCount() == 1 && 
                    !graph_node->paramIn.contains(graph_node->node->flowIn()))
                {
                    NodeCompilerInfo info;
                    info.node_id = graph_node->node->id();
                    info.error = "需要连接输入";
                    m_nodeInfo[info.node_id] = info;
                }
            }

            //更新输入输出
            for(int i = 0; i < m_originGraph->topolist.size(); i++)
            {
                QString error;
                auto graph_node = m_originGraph->topolist[i];
                if(!graph_node->node->update(error))
                {
                    NodeCompilerInfo info;
                    info.node_id = graph_node->node->id();
                    info.error = error;
                    m_nodeInfo[info.node_id] = info;
                }
            }

            //重新检测连接
            for(int i = 0; i < m_originGraph->topolist.size(); i++)
            {
                QString error;
                auto graph_node = m_originGraph->topolist[i];

                auto in_it = graph_node->paramIn.begin();
                while(in_it != graph_node->paramIn.end())
                {
                    JZNodeGemo to = JZNodeGemo(graph_node->node->id(),in_it.key());

                    auto &in_list = in_it.value();
                    for(int in_idx = 0; in_idx < in_list.size(); in_idx++)
                    {
                        JZNodeGemo from = in_list[in_idx];
                        if(!m_scriptFile->checkConnectType(from,to,error))
                        {
                            NodeCompilerInfo info;
                            info.node_id = graph_node->node->id();
                            info.error = error;
                            m_nodeInfo[info.node_id] = info;
                        }
                    }
                    in_it++;
                }
            }
            if(isBuildError())
                goto buildEnd;

            updateBuildGraph(event_list); //todo 似乎不需要这个了
            Q_ASSERT(m_buildGraph->topolist.size() > 0);

            int start_pc = m_script->statmentList.size();
            if(!bulidControlFlow())
                goto buildEnd;

            JZNodeEvent *node_event = dynamic_cast<JZNodeEvent*>(event_list[0]->node);
            JZFunctionDefine define = node_event->function();
            addFunction(define, start_pc);
        }                        

buildEnd:        
        if(!checkBuildResult())
            buildRet = false;
    }
    
    m_compilerInfo.result = buildRet;
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

QString JZNodeCompiler::pinName(JZNodePin *pin)
{
    if(!pin->name().isEmpty())
        return pin->name();
    else
        return "pin" + QString::number(pin->id());
}

void JZNodeCompiler::setOutPinTypeDefault(JZNode *node)
{    
    auto list = node->paramOutList();
    for (int i = 0; i < list.size(); i++)
    {
        int pin_id = list[i];
        auto pin = node->pin(pin_id);
        if(pin->dataType().size() == 1)
            m_nodeInfo[node->id()].pinType[pin_id] = pin->dataTypeId().front();
    }
}

void JZNodeCompiler::updateFlowOut()
{
    auto it = m_nodeInfo.begin();
    while (it != m_nodeInfo.end())
    {
        m_statmentList = &it->statmentList;
        for (int i = 0; i < m_statmentList->size(); i++)
        {
            if (m_statmentList->at(i)->type != OP_ComilerFlowOut)
                continue;

            JZNodeIRFlowOut *stmt = (JZNodeIRFlowOut*)m_statmentList->at(i).data();
            auto to_gemo = paramGemo(stmt->toId);
            if (!m_nodeInfo[to_gemo.nodeId].pinType.contains(to_gemo.pinId))
            {
                replaceStatement(i, JZNodeIRPtr(new JZNodeIR(OP_nop)));
                continue;
            }

            int next_pc = m_statmentList->size();
            addSetVariableConvert(irId(stmt->toId), irId(stmt->fromId));

            QList<JZNodeIRPtr> new_list = m_statmentList->mid(next_pc);
            for (int j = 0; j < new_list.size(); j++)
                new_list[j]->pc = -1;
            *m_statmentList = m_statmentList->mid(0, next_pc);
            replaceStatement(i, new_list);

            addNodeFlowPc(it.key(), i, new_list.size() - 1);
        }
        m_statmentList = nullptr;

        it++;
    }
}

void JZNodeCompiler::addNodeFlowPc(int node_id, int cond,int pc)
{
    auto add_pc1 = [](QList<NodeCompilerInfo::Jump> &list, int pc_cond,int add_pc)
    {
        for (int i = 0; i < list.size(); i++)
        {
            if(list[i].pc >= pc_cond)
                list[i].pc += add_pc;
        }
    };

    auto add_pc2 = [](QList<int> &list, int pc_cond, int add_pc) 
    {
        for (int i = 0; i < list.size(); i++)
        {
            if (list[i] >= pc_cond)
                list[i] += add_pc;
        }
    };

    auto &node_info = m_nodeInfo[node_id];
    add_pc1(node_info.jmpList, cond, pc);
    add_pc1(node_info.jmpSubList, cond, pc);
    add_pc2(node_info.continuePc, cond, pc);
    add_pc2(node_info.breakPc, cond, pc);
    add_pc2(node_info.continueList, cond, pc);
    add_pc2(node_info.breakList, cond, pc);
}

void JZNodeCompiler::linkNodes(QList<GraphNode *> flow_list)
{
    for (int node_idx = 0; node_idx < m_buildGraph->topolist.size(); node_idx++)
    {
        auto node = m_buildGraph->topolist[node_idx]->node;
        if (!node->isFlowNode())
            continue;

        auto &node_info = m_nodeInfo[node->id()];
        node_info.start = m_script->statmentList.size();
        addNodeFlowPc(node->id(), 0, node_info.start);

        int cur_node_id = -1, cur_node_start = -1;
        int flow_debug_start = -1;        
        auto &node_stmt_list = node_info.statmentList;
        for (int i = 0; i < node_stmt_list.size(); i++)
        {
            auto stmt = node_stmt_list[i];            
            if (stmt->type == OP_jmp || stmt->type == OP_je || stmt->type == OP_jne)
            {
                JZNodeIRJmp *jmp = (JZNodeIRJmp*)stmt.data();                            
                int new_jmp = jmp->jmpPc + node_info.start;
                jmp->jmpPc = new_jmp;   
            }
            
            stmt->pc = m_script->statmentList.size();
            m_script->statmentList.push_back(stmt);            
        }
        
        auto &dataRanges = m_nodeInfo[node->id()].dataRanges;
        auto it = dataRanges.begin();
        while (it != dataRanges.end())
        {
            auto &rg_list = it.value();
            for (int i = 0; i < rg_list.size(); i++)
            {
                auto rg = rg_list[i];
                rg.start += node_info.start;
                rg.debugStart += node_info.start;
                rg.end += node_info.start;
                m_nodeInfo[it.key()].ranges << rg;
            }
            it++;
        }
        
        auto &flow_rng = m_nodeInfo[node->id()].ranges[0];
        flow_rng.start += node_info.start;
        flow_rng.debugStart += node_info.start;
        flow_rng.end += node_info.start;
    }

    m_statmentList = &m_script->statmentList;
    //替换 subFlowOut 为实际子节点地址, 子节点的后续节点也在此处处理
    for(int node_idx = 0; node_idx < flow_list.size(); node_idx++)
    {   
        GraphNode *graph_node = flow_list[node_idx];
        NodeCompilerInfo &info = m_nodeInfo[graph_node->node->id()];
        for(int i = 0; i < info.jmpSubList.size(); i++)
        {
            int pin = info.jmpSubList[i].pin;
            if(!graph_node->paramOut.contains(pin))
                continue;

            auto &out_list = graph_node->paramOut[pin];
            Q_ASSERT(out_list.size() == 1);
            JZNodeGemo next_gemo = out_list[0];            

            JZNodeIRJmp *jmp = new JZNodeIRJmp(OP_jmp);
            jmp->jmpPc = m_nodeInfo[next_gemo.nodeId].start;
            replaceStatement(info.jmpSubList[i].pc,JZNodeIRPtr(jmp));

            replaceSubNode(out_list[0].nodeId,graph_node->node->id(),i);
        }                            
    }

    //替换 flowOut 为实际节点地址
    bool is_return_value = (m_scriptFile->itemType() == ProjectItem_scriptFunction &&
        m_scriptFile->function().paramOut.size() != 0);
    for(int node_idx = 0; node_idx < flow_list.size(); node_idx++)
    {   
        GraphNode *graph_node = flow_list[node_idx];
        NodeCompilerInfo &info = m_nodeInfo[graph_node->node->id()];
        if(info.parentId != -1)
            continue;

        info.allSubReturn = isAllFlowReturn(info.node_id, true);
        //connect next
        for(int jmp_idx = 0; jmp_idx < info.jmpList.size(); jmp_idx++)
        {
            int pin = info.jmpList[jmp_idx].pin;
            int pc = info.jmpList[jmp_idx].pc;

            if(graph_node->paramOut.contains(pin))
            {
                auto &out_list = graph_node->paramOut[info.jmpList[jmp_idx].pin];
                Q_ASSERT(out_list.size() == 1);

                JZNodeGemo next_gemo = out_list[0];
                JZNodeIRJmp *jmp = new JZNodeIRJmp(OP_jmp);
                jmp->jmpPc = m_nodeInfo[next_gemo.nodeId].start;
                replaceStatement(pc,JZNodeIRPtr(jmp));
            }
            else
            {                
                if (is_return_value && !info.allSubReturn)
                {
                    QString error_tips = graph_node->node->idName() + ",输出" + QString::number(jmp_idx + 1) + "需要连接return,并给与返回值";
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
                    //应当已经在子节点中return
                    JZNodeIRAssert *ir_assert = new JZNodeIRAssert();
                    ir_assert->tips = irLiteral("unexception flow return");
                    replaceStatement(pc, JZNodeIRPtr(ir_assert));
                }                
            }
        }
    }   
}

void JZNodeCompiler::updateDebugInfo()
{
}

void JZNodeCompiler::updateDispayNode()
{
    for (int node_idx = 0; node_idx < m_originGraph->topolist.size(); node_idx++)
    {
        auto graph_node = m_originGraph->topolist[node_idx];
        if (graph_node->node->type() != Node_display)
            continue;

        auto it = graph_node->paramIn.begin();
        while (it != graph_node->paramIn.end())
        {
            auto &in_list = it.value();
            if (in_list.size() > 0)
            {
                NodeWatch watch;
                watch.traget = paramId(graph_node->node->id(), it.key());
                watch.source = paramId(in_list[0]);
                m_compilerInfo.watchList.push_back(watch);
            }            
            it++;
        }
    }
}

void JZNodeCompiler::updateDepend(const JZFunction *jzfunc)
{
    ScriptDepend depend;
    depend.function = jzfunc->define;
    for (int i = jzfunc->addr; i < jzfunc->addrEnd; i++)
    {
        auto *stmt = m_script->statmentList[i].data();
        switch (stmt->type)
        {
            case OP_call:
            {
                JZNodeIRCall *ir_call = dynamic_cast<JZNodeIRCall*>(stmt);
                auto func = JZNodeFunctionManager::instance()->function(ir_call->function);
                int node_id = -1;
                int pre_count = func->paramIn.size() + 1;
                if(i >= pre_count && m_script->statmentList[i - pre_count]->type == OP_nodeId)
                {
                    auto ir_node = dynamic_cast<JZNodeIRNodeId*>(m_script->statmentList[i - pre_count].data());
                    if(m_scriptFile->getNode(ir_node->id)->type() == Node_function)
                        node_id = ir_node->id;
                }

                if(node_id != -1)
                {
                    ScriptDepend::FunctionHook hook;
                    hook.nodeId = node_id;
                    hook.pc = i;
                    hook.function = func->fullName();

                    hook.params = func->paramOut; 
                    depend.hook.push_back(hook);
                }
                break;
            }            
            default:
                break;
        }
    }

    m_compilerInfo.depend[depend.function.fullName()] = depend;
}

JZNode* JZNodeCompiler::currentNode()
{
    if (m_compilerNodeStack.size() == 0)
        return nullptr;

    return m_scriptFile->getNode(m_compilerNodeStack.back().nodeInfo->node_id);
}

NodeCompilerInfo *JZNodeCompiler::currentNodeInfo()
{
    if (m_compilerNodeStack.size() == 0)
        return nullptr;

    return m_compilerNodeStack.back().nodeInfo;
}

bool JZNodeCompiler::hasPinType(int node_id, int prop_id)
{
    return m_nodeInfo[node_id].pinType.contains(prop_id);
}

int JZNodeCompiler::pinInputCount(int nodeId, int pinId)
{
    if (m_buildGraph->graphNode(nodeId)->paramIn.contains(pinId))
        return m_buildGraph->graphNode(nodeId)->paramIn[pinId].size();
    else
        return 0;
}

void JZNodeCompiler::setPinType(int node_id, int prop_id, int type)
{
    Q_ASSERT(type != Type_none && m_originGraph->pin(node_id,prop_id));
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
    return pinType(gemo.nodeId, gemo.pinId);
}

bool JZNodeCompiler::isPinLiteral(int nodeId, int pinId)
{
    if (m_buildGraph->graphNode(nodeId)->paramIn.contains(pinId))
    {
        auto it = m_buildGraph->graphNode(nodeId)->paramIn.find(pinId);
        if(it->size() != 1)
            return false;

        auto in_gemo = it->at(0);
        auto in_node = m_buildGraph->graphNode(in_gemo.nodeId)->node;
        return in_node->pin(in_gemo.pinId)->isLiteral();
    }
    else
    {
        return !m_buildGraph->node(nodeId)->pinValue(pinId).isEmpty();
    }
}

void JZNodeCompiler::setPinLiteral(int node_id, int pin_id,const QVariant &v)
{
    Q_ASSERT(m_originGraph->pin(node_id,pin_id));
    m_nodeInfo[node_id].pinLiteral[pin_id] = v;
}

QVariant JZNodeCompiler::pinLiteral(int nodeId, int pinId)
{
    int data_type = pinType(nodeId, pinId);
    if (m_buildGraph->graphNode(nodeId)->paramIn.contains(pinId))
    {
        auto it = m_buildGraph->graphNode(nodeId)->paramIn.find(pinId);
        if(it->size() != 1)
            return false;

        auto in_gemo = it->at(0);
        Q_ASSERT(m_nodeInfo[in_gemo.nodeId].pinLiteral.contains(in_gemo.pinId));
        return m_nodeInfo[in_gemo.nodeId].pinLiteral[in_gemo.pinId];
    }
    else
    {
        QString value = m_buildGraph->node(nodeId)->pinValue(pinId);
        return JZNodeType::initValue(data_type, value);
    }
}

bool JZNodeCompiler::compilerNode(JZNode *node)
{     
    if(checkBuildStop())
        return false;

    pushCompilerNode(node->id());
    
    int build_start_pc = nextPc();    
    QString error;

    setOutPinTypeDefault(node);
    bool ret = node->compiler(this,error);    
    if (ret)
    {                   
        auto &env = m_compilerNodeStack.back();        
        Q_ASSERT(env.debugStart != -1);

        NodeRange range;
        range.start = env.start;        
        range.end = nextPc();        
        if (m_compilerNodeStack.size() > 1 && m_compilerNodeStack[0].isFlow)
        {
            auto &top = m_compilerNodeStack[0];
            m_compilerNodeStack[0].nodeInfo->dataRanges[node->id()].push_back(range);
        }
        else
        {
            currentNodeInfo()->ranges.push_back(range);
        }                
        
        Q_ASSERT(node->flowOutCount() == currentNodeInfo()->jmpList.size());
        Q_ASSERT(node->subFlowCount() == currentNodeInfo()->jmpSubList.size());
        auto out_list = node->paramOutList();
        for (int i = 0; i < out_list.size(); i++)
        {
            Q_ASSERT(m_nodeInfo[node->id()].pinType.contains(out_list[i]));            
        }        
    }
    else
        currentNodeInfo()->error = error;
            
    popCompilerNode();

    return ret;
}

void JZNodeCompiler::pushCompilerNode(int id)
{
    Q_ASSERT(id >= 0);
    auto node = m_scriptFile->getNode(id);
    if(!m_nodeInfo.contains(id))
    {
        NodeCompilerInfo info;        
        info.node_id = node->id();
        info.node_type = node->type();
        m_nodeInfo[info.node_id] = info;
    }

    NodeCompilerStack stack;
    stack.nodeInfo = &m_nodeInfo[id];    
    stack.isFlow = node->isFlowNode();
    m_compilerNodeStack.push_back(stack);    

    if (currentNode()->isFlowNode())
    {
        Q_ASSERT(!m_statmentList);
        m_statmentList = &currentNodeInfo()->statmentList;
        m_statmentList->clear();
    }
    m_compilerNodeStack.back().start = nextPc();
}

void JZNodeCompiler::popCompilerNode()
{   
    if (currentNode()->isFlowNode())            
        m_statmentList = nullptr;

    m_compilerNodeStack.pop_back();   
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
        int from_prop_id = lines[i].from.pinId;
        int to_prop_id = lines[i].to.pinId;
                     
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

bool JZNodeCompiler::isBuildError()
{
    auto it = m_nodeInfo.begin();
    while(it != m_nodeInfo.end())
    {
        auto &nodeInfo = it.value();
        if(!nodeInfo.error.isEmpty())
            return true;

        it++;     
    }
    return false;
}

bool JZNodeCompiler::checkBuildResult()
{
    bool ok = true;

    auto it = m_nodeInfo.begin();
    while(it != m_nodeInfo.end())
    {
        auto node = m_originGraph->node(it.key());

        auto &nodeInfo = it.value();
        if(!nodeInfo.error.isEmpty())
        {
            QString name = node->name();
            QString error = makeLink(nodeInfo.error,m_scriptFile->itemPath(),nodeInfo.node_id) + "\n";
            m_error += error;

            m_compilerInfo.nodeError[node->id()] = error;
            ok = false;
        }   
        it++;     
    }

    return ok;
}

bool JZNodeCompiler::checkBuildStop()
{
    if(m_builder)
        return m_builder->isBuildInterrupt();

    return false;
}

void JZNodeCompiler::addFunction(const JZFunctionDefine &define, int start_addr)
{        
    JZFunctionDebugInfo func_debug;
    //编译结果    
    auto it = m_nodeInfo.begin();
    while (it != m_nodeInfo.end())
    {
        auto node = m_scriptFile->getNode(it->node_id);

        NodeInfo info;
        info.node_id = it->node_id;
        info.node_type = it->node_type;
        info.isFlow = node->isFlowNode();
        info.pcRanges << it->ranges;

        auto in_list = node->paramInList();
        for (int i = 0; i < in_list.size(); i++)
        {
            if (node->pin(in_list[i])->flag() & Pin_noValue)
                continue;

            NodeParamInfo param_info;
            param_info.define.name = node->pin(in_list[i])->name();
            param_info.define.dataType = pinType(node->id(), in_list[i]);
            param_info.id = paramId(node->id(), in_list[i]);
            info.paramIn.push_back(param_info);
        }

        auto out_list = node->paramOutList();
        for (int i = 0; i < out_list.size(); i++)
        {
            if (node->pin(out_list[i])->flag() & Pin_noValue)
                continue;

            NodeParamInfo param_info;
            param_info.define.name = node->pin(out_list[i])->name();
            param_info.define.dataType = pinType(node->id(), out_list[i]);
            param_info.id = paramId(node->id(), out_list[i]);
            info.paramOut.push_back(param_info);
        }
        func_debug.nodeInfo[it.key()] = info;

        it++;
    }

    JZFunction impl;
    impl.define = define;
    impl.addr = start_addr;
    impl.addrEnd = m_script->statmentList.size();
    impl.path = m_script->file;
    updateDepend(&impl);

    m_script->functionList.push_back(impl);
    m_script->functionDebugList.push_back(func_debug);
}

bool JZNodeCompiler::checkPinInType(int node_id, int prop_check_id, QString &error)
{   
    auto typeListName = [](QList<int> types)->QString{
        QStringList list;
        for(int i = 0; i < types.size(); i++)
        {
            list << JZNodeType::typeToName(types[i]);
        }
        return list.join(",");
    };

    GraphNode *graph = m_buildGraph->graphNode(node_id);
    
    error.clear();
    //获得输入类型    
    QMap<int, int> in_type;
    auto param_in = graph->node->paramInList();
    for (int param_idx = 0; param_idx < param_in.size(); param_idx++)
    {
        auto prop_in_id = param_in[param_idx];
        if (prop_check_id != -1 && prop_in_id != prop_check_id)
            continue;
        if (m_nodeInfo[node_id].pinType.contains(prop_in_id))
            continue;

        auto pin = graph->node->pin(prop_in_id);
        QString pin_name = "输入节点" + graph->node->pinName(prop_in_id);            
        int pin_type = Type_none;
        if (graph->paramIn.contains(prop_in_id))  //有输入
        {
            QList<int> from_type;
            auto from_list = graph->paramIn[prop_in_id];
            for (int i = 0; i < from_list.size(); i++)
            {   
                auto from_gemo = from_list[i];
                if (!m_nodeInfo[from_gemo.nodeId].pinType.contains(from_gemo.pinId))
                {
                    error = pin_name + "前置依赖未设置";
                    return false;
                }

                from_type.push_back(pinType(from_gemo));
            }
            pin_type = JZNodeType::matchType(from_type, pin->dataTypeId());
            if (pin_type == Type_none)
            {
                error = pin_name + "无法确定输入类型,输出" + typeListName(from_type) + ",需要" + typeListName(pin->dataTypeId());
                return false;
            }
        }
        else
        {
            if (!pin->value().isEmpty()) //默认值
            {         
                if(pin->dataType().size() == 1)
                {
                    pin_type = pin->dataTypeId()[0];
                    if(pin_type == Type_arg)
                        pin_type = JZNodeType::stringType(pin->value());
                }   
                else if(pin->dataType().size() > 1)
                {
                    int pin_value_type = JZNodeType::stringType(pin->value());
                    pin_type = JZNodeType::matchType( {pin_value_type} , pin->dataTypeId());
                }

                if (pin_type == Type_none)
                {
                    error = pin_name + "无法将输入" + pin->value() + "转换为" + typeListName(pin->dataTypeId());
                    return false;
                }
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
                    if (func->isMemberFunction() && JZNodeObjectManager::instance()->isInherits(m_className, func->className))
                    {
                        is_this = true;
                        pin_type = JZNodeObjectManager::instance()->getClassId(func->className);
                    }
                }
                    
                if (!is_this)
                {
                    error = pin_name + "未设置";
                    return false;
                }
            }
        }
        in_type[prop_in_id] = pin_type;
    }

calc_end:
    auto it_type = in_type.begin();
    while (it_type != in_type.end())
    {
        m_nodeInfo[node_id].pinType.insert(it_type.key(), it_type.value());
        it_type++;
    }    
    return error.isEmpty();
}

bool JZNodeCompiler::bulidControlFlow()
{        
    updateDispayNode();

    //build node
    QList<GraphNode *> graph_list = m_buildGraph->topolist;
    QList<GraphNode *> flow_list;
    for (int graph_idx = 0; graph_idx < graph_list.size(); graph_idx++)
    {
        GraphNode *graph = graph_list[graph_idx];
        JZNode *node = graph->node;
        if(!node->isFlowNode())
            continue;
        
        compilerNode(node);      
        flow_list << graph;
    }        
    if (isBuildError())
        return false;

    updateFlowOut();
    linkNodes(flow_list);
    updateDebugInfo();   
    
    if (isBuildError())
        return false;

    //init function stack
    auto createAlloc = [](int id,int dataType)->JZNodeIRPtr
    {
        Q_ASSERT(id < Reg_Start);

        JZNodeIRAlloc *alloc = new JZNodeIRAlloc();
        alloc->allocType = JZNodeIRAlloc::StackId;
        alloc->id = id;
        alloc->dataType = dataType;
        return JZNodeIRPtr(alloc);
    };

    for (int pc = 0; pc < m_statmentList->size(); pc++)
    {
        if (m_statmentList->at(pc)->type != OP_ComilerStackInit)
            continue;

        QList<JZNodeIRPtr> ir_list;
        
        for (int node_idx = 0; node_idx < m_buildGraph->topolist.size(); node_idx++)
        {
            auto node = m_buildGraph->topolist[node_idx]->node;        
            auto in_list = node->paramInList();
            for (int j = 0; j < in_list.size(); j++)
            {
                if (node->pin(in_list[j])->flag() & Pin_noValue)
                    continue;

                int pin_id = paramId(node->id(),in_list[j]);
                int pin_type = pinType(node->id(),in_list[j]);
                ir_list << createAlloc(pin_id, pin_type);
            }

            auto out_list = node->paramOutList();
            for (int j = 0; j < out_list.size(); j++)
            {
                if (node->pin(out_list[j])->flag() & Pin_noValue)
                    continue;
                
                int pin_id = paramId(node->id(),out_list[j]);
                int pin_type = pinType(node->id(),out_list[j]);
                ir_list << createAlloc(pin_id, pin_type);
            }
        }

        auto it = m_stackType.begin();
        while(it != m_stackType.end())
        {
            ir_list << createAlloc(it.key(),it.value());
            it++;
        }
        replaceStatement(pc, ir_list);        
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

bool JZNodeCompiler::buildParamBinding()
{           
    int start_pc = m_script->statmentList.size();
    buildDataFlow(m_buildGraph->topolist);    
    if(!checkBuildResult())
        return false;
    
    addStatement(JZNodeIRPtr(new JZNodeIR(OP_return)));
    for (int node_idx = 0; node_idx < m_originGraph->topolist.size(); node_idx++)
    {

    }
    
    return true;
}

int JZNodeCompiler::isAllFlowReturn(int id, bool root)
{
    NodeCompilerInfo &info = m_nodeInfo[id];
    auto graph_node = m_buildGraph->graphNode(id);
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
            int pin = info.jmpSubList[i].pin;
            if (!graph_node->paramOut.contains(pin))
                continue;

            auto &out_list = graph_node->paramOut[pin];
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
                int pin = info.jmpList[i].pin;
                if (!graph_node->paramOut.contains(pin))
                    continue;

                auto &out_list = graph_node->paramOut[pin];
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
    auto graph_node = m_buildGraph->graphNode(id);
    for(int i = 0; i < info.jmpList.size(); i++)
    {   
        int pin = info.jmpList[i].pin;
        int pc = info.jmpList[i].pc;
        if(graph_node->paramOut.contains(pin))
        {
            auto &out_list = graph_node->paramOut[pin];
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
    ir->pc = m_statmentList->size();    
    m_statmentList->push_back(ir);
    return ir->pc;
}

JZScriptItem *JZNodeCompiler::currentFile()
{
    return m_scriptFile;
}

Graph *JZNodeCompiler::currentGraph()
{
    return m_buildGraph.data();
}

int JZNodeCompiler::currentPc()
{
    return (m_statmentList->size() - 1);
}

int JZNodeCompiler::nextPc()
{
    return m_statmentList->size();
}

const JZFunctionDefine *JZNodeCompiler::function(QString name)
{
    const JZFunctionDefine *func = m_project->function(name);
    if(func)
        return func;
        
    return JZNodeFunctionManager::instance()->function(name);
}

JZNodeIR *JZNodeCompiler::lastStatment()
{
    if (m_statmentList->size() > 0)
        return m_statmentList->back().data();
    else
        return nullptr;
}

void JZNodeCompiler::removeStatement(int pc)
{
    m_statmentList->removeAt(pc);
    for (int i = 0; i < m_statmentList->size(); i++)
    {
        auto stmt = m_statmentList->at(i).data();
        stmt->pc = i;
        if (stmt->type == OP_jmp || stmt->type == OP_je || stmt->type == OP_jne)
        {
            JZNodeIRJmp *jmp = (JZNodeIRJmp *)stmt;
            if (jmp->jmpPc > pc)
            {
                int old_pc = jmp->jmpPc;
                jmp->jmpPc = old_pc - 1;
            }
        }
    }
}

void JZNodeCompiler::replaceStatement(int pc,JZNodeIRPtr ir)
{
    Q_ASSERT(ir->pc == -1 && pc < m_statmentList->size());
    ir->pc = pc;        
    m_statmentList->replace(pc, ir);
}

void JZNodeCompiler::replaceStatement(int pc, QList<JZNodeIRPtr> ir_list)
{
    if(ir_list.size() == 0)
    {
        removeStatement(pc);
        return;
    }

    replaceStatement(pc,ir_list[0]);
    for (int i = 1; i < ir_list.size(); i++)
        m_statmentList->insert(pc + i, ir_list[i]);

    int pc_add = ir_list.size() - 1;
    for (int i = 0; i < m_statmentList->size(); i++)
    {
        auto stmt = m_statmentList->at(i).data();
        stmt->pc = i;
        if (stmt->type == OP_jmp || stmt->type == OP_je || stmt->type == OP_jne)
        {
            JZNodeIRJmp *jmp = (JZNodeIRJmp *)stmt;
            if (jmp->jmpPc > pc)
            {
                int old_pc = jmp->jmpPc;
                jmp->jmpPc = old_pc + pc_add;
            }
        }
    }
}

int JZNodeCompiler::addJumpNode(int pin)
{
    Q_ASSERT(currentNode()->pin(pin) && currentNode()->pin(pin)->isFlow()
        && currentNode()->pin(pin)->isOutput());

    NodeCompilerInfo::Jump info;
    JZNodeIR *jmp = new JZNodeIR(OP_none);
    jmp->memo = "flow" + QString::number(currentNode()->flowOutList().indexOf(pin));
    addStatement(JZNodeIRPtr(jmp));
    info.pin = pin;
    info.pc = jmp->pc;
    currentNodeInfo()->jmpList.push_back(info);
    return info.pc;
}

int JZNodeCompiler::addJumpSubNode(int pin)
{
    Q_ASSERT(currentNode()->pin(pin) && currentNode()->pin(pin)->isSubFlow());

    NodeCompilerInfo::Jump info;
    JZNodeIR *jmp = new JZNodeIR(OP_nop);
    jmp->memo = "subflow" + QString::number(currentNode()->subFlowList().indexOf(pin));
    addStatement(JZNodeIRPtr(jmp));
    info.pin = pin;
    info.pc = jmp->pc;
    currentNodeInfo()->jmpSubList.push_back(info);
    return info.pc;
}

void JZNodeCompiler::setBreakContinue(const QList<int> &breakPc, const QList<int> &continuePc)
{
    currentNodeInfo()->breakPc = breakPc;
    currentNodeInfo()->continuePc = continuePc;
}

int JZNodeCompiler::addContinue()
{
    JZNodeIR *jmp = new JZNodeIRJmp(OP_jmp);
    addStatement(JZNodeIRPtr(jmp));

    currentNodeInfo()->continueList.push_back(jmp->pc);
    return jmp->pc;
}

int JZNodeCompiler::addBreak()
{
    JZNodeIR *jmp = new JZNodeIRJmp(OP_jmp);
    addStatement(JZNodeIRPtr(jmp));

    currentNodeInfo()->breakList.push_back(jmp->pc);
    return jmp->pc;
}

void JZNodeCompiler::addCall(const QString &function, const QList<JZNodeIRParam> &paramIn,const QList<JZNodeIRParam> &paramOut)
{    
    auto func = JZNodeFunctionManager::instance()->function(function);
    addCall(func, paramIn, paramOut); 
}

void JZNodeCompiler::addCall(const JZFunctionDefine *func, const QList<JZNodeIRParam> &paramIn, const QList<JZNodeIRParam> &paramOut)
{
    Q_ASSERT(func && (func->isVariadicFunction() || func->paramIn.size() == paramIn.size()) && func->paramOut.size() >= paramOut.size());

    setRegCallFunction(func);
    for(int i = 0; i < paramIn.size(); i++)
        addSetVariable(irId(Reg_CallIn + i),paramIn[i]);

    JZNodeIRCall *call = new JZNodeIRCall();
    call->function = func->fullName();
    call->inCount = paramIn.size();
    addStatement(JZNodeIRPtr(call));

    for(int i = 0; i < paramOut.size(); i++)
        addSetVariable(paramOut[i],irId(Reg_CallOut + i));
    setRegCallFunction(nullptr);
    addStatement(JZNodeIRPtr(new JZNodeIR(OP_clearReg)));    
}

void JZNodeCompiler::addCallConvert(const QString &function, const QList<JZNodeIRParam> &paramIn, const QList<JZNodeIRParam> &paramOut)
{
    auto func = JZNodeFunctionManager::instance()->function(function);
    addCallConvert(func, paramIn, paramOut); 
}

void JZNodeCompiler::addCallConvert(const JZFunctionDefine *func, const QList<JZNodeIRParam> &org_paramIn, const QList<JZNodeIRParam> &org_paramOut)
{
    Q_ASSERT(func && func->paramIn.size() == org_paramIn.size() && func->paramOut.size() >= org_paramOut.size());

    QList<JZNodeIRParam> param_in;
    for(int i = 0; i < org_paramIn.size(); i++)
    {
        int func_param_type = func->paramIn[i].dataType();
        int param_type = irParamType(org_paramIn[i]);
        if(func_param_type == param_type)
        {
            param_in << org_paramIn[i];
        }
        else
        {
            int id = allocStack(func_param_type);
            addConvert(org_paramIn[i],func_param_type,irId(id));
            param_in << irId(id);
        }
    }   

    QList<JZNodeIRParam> param_out;
    for(int i = 0; i < org_paramOut.size(); i++)
    {
        int func_param_type = func->paramOut[i].dataType();
        int param_type = irParamType(org_paramOut[i]);
        if(func_param_type == param_type)
        {
            param_out << org_paramOut[i];
        }
        else
        {
            int id = allocStack(func_param_type);
            param_out << irId(id);
        }
    }

    addCall(func,param_in,param_out);
    for(int i = 0; i < org_paramOut.size(); i++)
    {   
        int func_param_type = func->paramOut[i].dataType();
        int param_type = irParamType(org_paramOut[i]);
        if(func_param_type == param_type)
            addConvert(param_out[i],param_type,org_paramOut[i]);
    }
}

void JZNodeCompiler::resetStack()
{
    m_stackId = Stack_User;
    m_nodeInfo.clear();
    m_stackType.clear();
    m_statmentList = nullptr;
    m_regCallFunction = nullptr;
}

int JZNodeCompiler::allocStack(int dataType)
{
    Q_ASSERT(dataType != Type_none);

    int id = m_stackId++;    
    m_stackType[id] = dataType;
    return id;        
}

JZNodeIRParam JZNodeCompiler::paramRef(QString name)
{
    auto coor = variableCoor(m_scriptFile,name);
    if (coor == Variable_member && !name.startsWith("this."))
        name = "this." + name;
    return irRef(name);
}

void JZNodeCompiler::addFunctionAlloc(const JZFunctionDefine &define)
{
    int start_pc = nextPc();
    setRegCallFunction(&define);
    for (int i = 0; i < define.paramIn.size(); i++)
    {
        if (i != 0 || !define.isMemberFunction())
        {
            addAlloc(JZNodeIRAlloc::Stack,define.paramIn[i].name, define.paramIn[i].dataType());
            addSetVariable(irRef(define.paramIn[i].name),irId(Reg_CallIn + i));
        }
    }
    setRegCallFunction(nullptr);

    auto list = m_scriptFile->localVariableList(false);
    for (int i = 0; i < list.size(); i++)
    {
        auto param = m_scriptFile->localVariable(list[i]);                
        addAlloc(JZNodeIRAlloc::Stack, param->name,param->dataType());
        if(!param->value.isEmpty())
            addSetVariable(irRef(param->name),irLiteral(JZNodeType::initValue(param->dataType(), param->value)));
    }

    addStatement(JZNodeIRPtr(new JZNodeIRStackInit()));
}

const JZParamDefine *JZNodeCompiler::getVariableInfo(JZScriptItem *file,const QString &name)
{
    auto project = file->project();    
    if (name.startsWith("this."))
    {        
        JZScriptClassItem *class_file = project->getItemClass(file);
        auto def = JZNodeObjectManager::instance()->meta(class_file->className());
        if (!def)
            return nullptr;

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
        
        auto def = file->localVariable(base_name);        
        if (!def)
        {
            JZScriptClassItem *class_file = project->getItemClass(file);
            if (class_file)
            {
                auto class_meta = JZNodeObjectManager::instance()->meta(class_file->className());
                def = class_meta->param(base_name);
            }
        }
        if (!def)
            def = project->globalVariable(base_name);
        if (!def)
            return nullptr;

        if (param_name.isEmpty())
            return def;
        else
        {
            auto obj_def = JZNodeObjectManager::instance()->meta(def->dataType());
            return obj_def->param(param_name);
        }           
    }
}

void JZNodeCompiler::setRegCallFunction(const JZFunctionDefine *func)
{
    Q_ASSERT((m_regCallFunction == nullptr && func) || (m_regCallFunction && func == nullptr));
    m_regCallFunction = func;
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
        error = "no such element " + name;
        return false;
    }
    if (info->dataType() == Type_none)
    {
        error = name + " param define is out of date";
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
    if(!JZNodeType::isInherits(info->dataType(),needType))
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
 
    GraphNode* in_node = m_buildGraph->graphNode(nodeId);
    auto in_list = in_node->node->paramInList();
    for (int prop_idx = 0; prop_idx < in_list.size(); prop_idx++)
    {
        int in_prop = in_list[prop_idx];
        if (prop_id != -1 && in_prop != prop_id)
            continue;

        auto pin = in_node->node->pin(in_prop);
        if (pin->flag() & Pin_noValue)
            continue;
        
        int pin_type = pinType(nodeId,in_prop);
        if (in_node->paramIn.contains(in_prop))
        {
            QList<JZNodeGemo> &gemo_list = in_node->paramIn[in_prop];
            Q_ASSERT(gemo_list.size() > 0);
            for (int i = 0; i < gemo_list.size(); i++)
            {
                GraphNode *from_node = m_buildGraph->graphNode(gemo_list[i].nodeId);
                if (from_node->node->isFlowNode())
                    break;

                Q_ASSERT(gemo_list.size() == 1);
                int from_id = paramId(gemo_list[i]);
                int to_id = paramId(nodeId, in_prop);
                addSetVariableConvert(irId(to_id), irId(from_id));
            }
        }
        else
        {   
            bool is_member_function = false;
            const JZFunctionDefine *func = nullptr;
            if (prop_idx == 0 && in_node->node->type() == Node_function)
            {
                JZNodeFunction *func_node = (JZNodeFunction*)in_node->node;
                func = JZNodeFunctionManager::instance()->function(func_node->function());
                is_member_function = func->isMemberFunction();
            }
            
            int to_id = paramId(in_node->node->id(), in_prop); 
            if (pin->value().isEmpty())
            {
                if(is_member_function && JZNodeType::isInherits(m_className,func->className))
                {
                    addSetVariable(irId(to_id), irThis());
                }
                else
                {
                    Q_ASSERT(false);
                    return false;
                }
            }            
            else
            {
                if(is_member_function)
                {
                    QString param_name = pin->value();
                    auto param_def = getVariableInfo(param_name);
                    if(!param_def || param_def->dataType() != pin_type)
                    {
                        error = "无效的参数:" + param_name;
                        return false;
                    }
                    addSetVariable(irId(to_id),irRef(param_name));
                }
                else
                {
                    int match_type = pinType(nodeId,in_prop);
                    auto v = JZNodeType::initValue(match_type, pin->value());
                    if (!v.isValid())
                    {
                        error = "无效的输入:" + pin->value() + ",需要" + JZNodeType::typeToName(match_type);
                        return false;
                    }
                    if (JZNodeType::variantType(v) == Type_any)
                    {
                        auto any = v.value<JZNodeVariantAny>();
                        addSetVariableConvert(irId(to_id),irLiteral(any.value));
                    }
                    else
                        addSetVariable(irId(to_id), irLiteral(v));
                }
            }
        }
    }

    Q_ASSERT(m_compilerNodeStack.back().nodeInfo->node_id == nodeId);
    if(m_compilerNodeStack.back().debugStart == -1 && m_nodeInfo[nodeId].autoAddDebugStart)
        addNodeDebug(nodeId);

    return true;
}

bool JZNodeCompiler::addDataInput(int nodeId,QString &error)
{
    return addDataInput(nodeId, -1, error);    
}

bool JZNodeCompiler::addFlowInput(int nodeId, int prop_id, QString &error)
{    
    Q_ASSERT(m_buildGraph->graphNode(nodeId)->node->isFlowNode() || m_buildGraph->graphNode(nodeId)->node->type() == Node_and
        || m_buildGraph->graphNode(nodeId)->node->type() == Node_or);

    QList<GraphNode*> graph_list;
    QSet<GraphNode*> graphs;

    //广度遍历所有节点
    QList<GraphNode*> in_list;
    in_list << m_buildGraph->graphNode(nodeId);

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
                if (!in_node->node->pin(it.key())->isParam())
                {
                    it++;
                    continue;
                }
                if (in_node->node->id() == nodeId && prop_id != -1 && it.key() != prop_id)
                {
                    it++;
                    continue;
                }

                QList<JZNodeGemo> &gemo_list = it.value();
                for (int i = 0; i < gemo_list.size(); i++)
                {
                    GraphNode *from_node = m_buildGraph->graphNode(gemo_list[i].nodeId);
                    if (from_node->node->isFlowNode())
                    {
                        auto n = from_node->node;
                        JZNodeEvent *node_event = dynamic_cast<JZNodeEvent*>(n);
                        if (!node_event && m_scriptFile->getConnectPin(n->id(), n->flowIn()).size() == 0)
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
    for (int i = 0; i < m_buildGraph->topolist.size(); i++)
    {
        auto node = m_buildGraph->topolist[i];
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
    auto graph = m_buildGraph->graphNode(nodeId);
    Q_ASSERT(graph);
    auto node = graph->node;
    //set out put
    auto it_out = graph->paramOut.begin();
    while(it_out != graph->paramOut.end())
    {
        auto &out_list = it_out.value();
        auto pin = node->pin(it_out.key());
        if(pin->isParam())
        {
            for(int out_idx = 0; out_idx < out_list.size(); out_idx++)
            {
                JZNodeIRFlowOut *ir_flowout = new JZNodeIRFlowOut();
                ir_flowout->fromId = paramId(node->id(),it_out.key());
                ir_flowout->toId = paramId(out_list[out_idx]);
                addStatement(JZNodeIRPtr(ir_flowout));
            }
        }
        it_out++;
    }
}

void JZNodeCompiler::addAlloc(int allocType, QString name, int dataType)
{
    Q_ASSERT(dataType != Type_none);
    
    JZNodeIRAlloc *alloc = new JZNodeIRAlloc();
    alloc->allocType = allocType;
    alloc->name = name;
    alloc->dataType = dataType;
    addStatement(JZNodeIRPtr(alloc));
}

void JZNodeCompiler::addAssert(const JZNodeIRParam &tips)
{
    JZNodeIRAssert *assert = new JZNodeIRAssert();
    assert->tips = tips;
    addStatement(JZNodeIRPtr(assert));
}

int JZNodeCompiler::addNop()
{
    JZNodeIR *nop = new JZNodeIR(OP_nop);
    return addStatement(JZNodeIRPtr(nop));
}

int JZNodeCompiler::addNodeDebug(int id)
{
    auto node = m_scriptFile->getNode(id);    
    JZNodeIRNodeId *node_ir = new JZNodeIRNodeId();
    node_ir->id = id;
    node_ir->memo = node->name() + "(" + QString::number(node->id()) + ")";
    m_compilerNodeStack.back().debugStart = addStatement(JZNodeIRPtr(node_ir));
    return node_ir->pc;    
}

void JZNodeCompiler::setAutoAddNodeDebug(int m_id,bool flag)
{
    m_nodeInfo[m_id].autoAddDebugStart = flag;
}

int JZNodeCompiler::addExpr(const JZNodeIRParam &dst,const JZNodeIRParam &p1,const JZNodeIRParam &p2,int op)
{
    Q_ASSERT(irParamTypeMatch(p1,p2,false));
    Q_ASSERT(JZNodeType::calcExprType(irParamType(p1),irParamType(p2),op) == irParamType(dst));

    JZNodeIRExpr *expr = new JZNodeIRExpr(op);
    expr->src1 = p1;
    expr->src2 = p2;
    expr->dst = dst;
    return addStatement(JZNodeIRPtr(expr));
}

void JZNodeCompiler::addExprConvert(const JZNodeIRParam &dst, const JZNodeIRParam &p1, const JZNodeIRParam &p2,int op)
{   
    int t1 = irParamType(p1);
    int t2 = irParamType(p2);
    int tDst = irParamType(dst);
    int upType = JZNodeType::upType(t1,t2);
    int dstType = JZNodeType::calcExprType(upType,upType,op);

    JZNodeIRParam tmp_dst = dst;
    JZNodeIRParam tmp_p1 = p1;
    JZNodeIRParam tmp_p2 = p2;
    if(t1 != upType)
    {
        int id = allocStack(upType);
        addConvert(p1,upType,irId(id));
        tmp_p1 = irId(id);
    }
    if(t2 != upType)
    {
        int id = allocStack(upType);
        addConvert(p2,upType,irId(id));
        tmp_p2 = irId(id);
    }
    if(tDst != dstType)
    {
        int id = allocStack(dstType);
        tmp_dst = irId(id);
    }

    addExpr(tmp_dst,tmp_p1,tmp_p2,op);
    if(tDst != dstType)
        addConvert(tmp_dst,tDst,dst);
}

int JZNodeCompiler::addCompare(const JZNodeIRParam &p1,const JZNodeIRParam &p2,int op)
{
    Q_ASSERT(irParamTypeMatch(p1,p2,false));
    return addExpr(irId(Reg_Cmp),p1,p2,op);    
}

void JZNodeCompiler::addCompareConvert(const JZNodeIRParam &p1, const JZNodeIRParam &p2,int op)
{
    addCompare(p1,p2,op);
}

int JZNodeCompiler::irParamType(const JZNodeIRParam &param)
{
    if(param.isId())
    {
        int id = param.id(); 
        if(id < Reg_Start)
        {
            if(id < Stack_User)
                return pinType(paramGemo(param.id()));
            else
                return m_stackType[param.id()];
        }
        else if(id == Reg_Cmp)
            return Type_bool;
        else
        {
            auto func = m_regCallFunction;
            Q_ASSERT(m_regCallFunction);
            if(id < Reg_CallOut)
            {
                if(func->isVariadicFunction() && (id - Reg_CallIn >= func->paramIn.size() - 1))
                    return Type_arg;
                    
                return func->paramIn[id - Reg_CallIn].dataType();
            }
            else
                return func->paramOut[id - Reg_CallOut].dataType();
        }
    }
    else if(param.isLiteral())
        return JZNodeType::variantType(param.literal());
    else if(param.isRef())
    {
        auto ref = getVariableInfo(param.ref());
        Q_ASSERT(ref);
        return ref->dataType();
    }
    else if(param.isThis())
        return JZNodeType::nameToType(m_className);

    Q_ASSERT(0);
    return Type_none;
}

bool JZNodeCompiler::irParamTypeMatch(const JZNodeIRParam &p1,const JZNodeIRParam &p2,bool isSet)
{
    int t1 = irParamType(p1);
    int t2 = irParamType(p2);
    if(isSet)
        return JZNodeType::isSameType(t1,t2);
    else  //compare
        return JZNodeType::isSameType(t1,t2) || JZNodeType::isSameType(t2,t1);
}

void JZNodeCompiler::addWatchDisplay(const JZNodeIRParam &dst)
{
    if (dst.isId())
    {
        for (int i = 0; i < m_compilerInfo.watchList.size(); i++)
        {
            auto &watch = m_compilerInfo.watchList[i];
            if (watch.source == dst.id())
            {
                JZNodeIRWatch *ir_watch = new JZNodeIRWatch();
                ir_watch->source = irId(watch.source);
                ir_watch->traget = irId(watch.traget);
                addStatement(JZNodeIRPtr(ir_watch));
            }
        }
    }
}

void JZNodeCompiler::addSetVariable(const JZNodeIRParam &dst, const JZNodeIRParam &src)
{
    Q_ASSERT(irParamTypeMatch(src,dst,true));
    if(dst.isId() && dst.id() < Stack_User && src.isLiteral())
    {
        auto gemo = paramGemo(dst.id());
        setPinLiteral(gemo.nodeId,gemo.pinId, src.value);
    }
    
    bool clone = false;
    int dst_type = irParamType(dst);
    if(dst_type >= Type_class)
    {
        auto meta = JZNodeObjectManager::instance()->meta(dst_type);
        if(meta->isValueType())
            clone = true;
    }
    if(!clone)
    {
        JZNodeIRSet *op = new JZNodeIRSet();    
        op->dst = dst;
        op->src = src;
        addStatement(JZNodeIRPtr(op));
    }
    else
    {
        JZNodeIRClone *op = new JZNodeIRClone();    
        op->dst = dst;
        op->src = src;
        addStatement(JZNodeIRPtr(op));
    }
    addWatchDisplay(dst);
}

void JZNodeCompiler::addSetVariableConvert(const JZNodeIRParam &dst,const JZNodeIRParam &src)
{
    Q_ASSERT(src.type != JZNodeIRParam::None && dst.type != JZNodeIRParam::None);
    Q_ASSERT(dst.type != JZNodeIRParam::Literal);

    int from_type = irParamType(src);
    int to_type = irParamType(dst);
    if (from_type == to_type)
    {
        addSetVariable(dst,src);
    }
    else
    {
        addConvert(src,to_type,dst);
    }
}

void JZNodeCompiler::addConvert(const JZNodeIRParam &src, int dst_type, const JZNodeIRParam &dst)
{
    Q_ASSERT(JZNodeType::canConvertExplicitly(irParamType(src),dst_type));
    Q_ASSERT(JZNodeType::isSameType(dst_type,irParamType(dst)));

    JZNodeIRConvert *op = new JZNodeIRConvert();    
    op->dst = dst;
    op->src = src;
    op->dstType = dst_type;
    addStatement(JZNodeIRPtr(op));

    addWatchDisplay(dst);
}