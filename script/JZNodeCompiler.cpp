#include <QVector>
#include <QSet>
#include <QDebug>
#include "JZNodeCompiler.h"
#include "JZNodeValue.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeFunction.h"
#include "JZClassItem.h"
#include "JZNodeUtils.h"
#include "JZNodeBuilder.h"
#include "JZRegExpHelp.h"
#include "JZNodeFlow.h"
#include "JZNodeOperator.h"
#include "LogManager.h"

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

QList<GraphNode*> Graph::eventList()
{
    QList<GraphNode*> event_list;
    for (int i = 0; i < topolist.size(); i++)
    {
        auto event_node = dynamic_cast<JZNodeEvent*>(topolist[i]->node);
        if (event_node)
            event_list << topolist[i];
    }
    return event_list;
}

GraphNode* Graph::firstNode()
{
    if (topolist.size() == 0)
        return nullptr;

    GraphNode *first = nullptr;
    for (int i = 0; i < topolist.size(); i++)
    {        
        if (topolist[i]->node->isFlowNode())
            return topolist[i];
    }
    return topolist.front();
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

QList<JZNodeGemo> Graph::pinInput(int nodeId, int pinId)
{
    auto n = graphNode(nodeId);
    return n->paramIn[pinId];
}

QList<JZNodeGemo> Graph::pinOutput(int nodeId, int pinId)
{
    auto n = graphNode(nodeId);
    return n->paramOut[pinId];
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

//JZNodeComilerIR
JZNodeComilerBreakContinue::JZNodeComilerBreakContinue(int node_id, JumpType jmp)
    :JZNodeIR((JZNodeIRType)OP_ComilerBreakContinue)
{
    nodeId = node_id;
    jumpType = jmp;
}

bool JZNodeComilerBreakContinue::isBreak()
{
    return (jumpType == Jmp_break);
}

//JZNodeIRFlowOut
JZNodeIRFlowOut::JZNodeIRFlowOut()
{
    type = (JZNodeIRType)OP_ComilerFlowOut;
    fromId = -1;
    toId = -1;
}

//JZNodeIRStackInit
JZNodeIRStackInit::JZNodeIRStackInit()
{
    type = (JZNodeIRType)OP_ComilerStackInit;
}

//JZNodeIRAutoInit
JZNodeIRAutoInit::JZNodeIRAutoInit()
{
    type = (JZNodeIRType)OP_ComilerAllocAuto;
}

//NodeCompilerInfo
NodeCompilerInfo::NodeCompilerInfo()
{
    node_id = -1;
    node_type = Node_none;    
    start = -1;          
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


//JZMacroIRReplace
JZMacroIRReplace::JZMacroIRReplace(JZNodeCompiler* c)
{
    m_compiler = c;
    m_stackId = Stack_User;
}

void JZMacroIRReplace::setLocalMap(QMap<QString, int> localMap)
{
    m_localMap = localMap;
}

int JZMacroIRReplace::stackId()
{
    return m_stackId;
}

void JZMacroIRReplace::setStackId(int stackId)
{
    m_stackId = stackId;
}

int JZMacroIRReplace::stackType(int id)
{
    Q_ASSERT(m_newStackType.contains(id));
    return m_newStackType[id];
}

void JZMacroIRReplace::replace(QList<JZNodeIRPtr>& ir_list)
{
    for (int i = 0; i < ir_list.size(); i++)
    {
        auto op = ir_list[i].data();
        switch (op->type)
        {
        case OP_alloc:
        {
            JZNodeIRAlloc* ir_alloc = (JZNodeIRAlloc*)op;
            if (ir_alloc->dst.isStack())
            {
                replaceIr(ir_alloc->dst);
            }
            break;
        }
        case OP_set:
        {
            JZNodeIRSet* ir_set = (JZNodeIRSet*)op;
            replaceIr(ir_set->dst);
            replaceIr(ir_set->src);
            break;
        }
        case OP_convert:
        {
            JZNodeIRConvert* ir_set = (JZNodeIRConvert*)op;
            replaceIr(ir_set->dst);
            replaceIr(ir_set->src);
            break;
        }
        case OP_add:
        case OP_sub:
        case OP_mul:
        case OP_div:
        case OP_mod:
        case OP_eq:
        case OP_ne:
        case OP_le:
        case OP_ge:
        case OP_lt:
        case OP_gt:
        case OP_and:
        case OP_or:
        case OP_bitand:
        case OP_bitor:
        case OP_bitxor:
        {
            JZNodeIRExpr* ir_expr = (JZNodeIRExpr*)op;
            replaceIr(ir_expr->dst);
            replaceIr(ir_expr->src1);
            replaceIr(ir_expr->src2);
            break;
        }
        case OP_bitreverse:
        case OP_not:
        {
            JZNodeIRExpr* ir_expr = (JZNodeIRExpr*)op;
            replaceIr(ir_expr->dst);
            replaceIr(ir_expr->src1);
            break;
        }
        }
    }
}

void JZMacroIRReplace::replaceIr(JZNodeIRParam& ir)
{
    if (ir.isRef())
    {
        //替换局部变量
        if (m_localMap.contains(ir.ref()))
            ir = irId(m_localMap[ir.ref()]);
    }
    else if (ir.isStack())
    {
        //替换栈变量
        int id = ir.id();
        if (!m_idMap.contains(id))
        {
            int new_stack_id = m_stackId;
            m_idMap[id] = irId(m_stackId++);
            m_newStackType[new_stack_id] = m_compiler->stackType(id);
        }
        ir = m_idMap[id];
    }
}

// JZNodeCompiler
int JZNodeCompiler::paramId(int nodeId,int pinId)
{
    return JZNodeGemo::paramId(nodeId,pinId);
}

int JZNodeCompiler::paramId(const JZNodeGemo &gemo)
{
    return gemo.paramId();
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

QString JZNodeCompiler::paramName(const JZNodeGemo& gemo)
{
    return paramName(gemo.paramId());
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
            auto class_file = file->getClassItem();
            if (class_file && class_file->memberVariable(name,true))
                return Variable_member;
            else
                return Variable_none;
        }
    }
}

QString JZNodeCompiler::errorString(CompilerTip tip, QStringList args)
{
    if (tip == Error_noType)
        return "no such type " + args[0];
    else if(tip == Error_noVariable)
        return "no such variable " + args[0];
    else if(tip == Error_varNameEmpty)
        return "variable name is empty";
    else if (tip == Error_varNameInvaild)
        return "variable name " + args[0] + " is invaild";
    else if(tip == Error_noFunction)
        return "no such function " + args[0];    
    else if(tip == Error_noImplement)
        return args[0] + "no implement " + args[1];
    else if (tip == Error_noClassMember)
        return args[0] + " no member " + args[1];
    else if(tip == Errro_initVariableFailed)
        return "can't init " + args[0] + " by " + args[1];
    else if (tip == Error_functionParamIn)
        return "function need " + args[0] + " param, but give " + args[1];
    else if (tip == Error_functionParamOut)
        return "function no output ";

    return "error";
}

JZNodeCompiler::JZNodeCompiler()
{    
    m_script = nullptr;
    m_scriptItem = nullptr;
    m_originGraph = nullptr;
    m_statmentList = nullptr;
    m_regCallFunction = nullptr;
    m_builder = nullptr;
    m_env = nullptr;
    m_stackId = Stack_User;

    m_buildGraph = GraphPtr(new Graph());
    m_ignoreError = "#IGNORE_ERROR#";
}

JZNodeCompiler::~JZNodeCompiler()
{
}

void JZNodeCompiler::init(JZScriptItem *scriptFile)
{    
    m_scriptItem = scriptFile;     
    m_env = project()->environment();
    m_script = nullptr;    
    m_originGraph = nullptr;
    m_statmentList = nullptr;
    m_regCallFunction = nullptr;    

    m_nodeGraph.clear();
    m_nodeInfo.clear();
    m_className.clear();    
    m_graphList.clear();        
    resetStack();
}

void JZNodeCompiler::setBuilder(JZNodeBuilder *builder)
{
    m_builder = builder;
}

const JZScriptEnvironment* JZNodeCompiler::env()
{
    return m_env;
}

JZProject *JZNodeCompiler::project()
{
    if (m_builder)
        return m_builder->project();
    else if (m_scriptItem)
        return m_scriptItem->project();
    else
        return nullptr;
}

bool JZNodeCompiler::genGraphs(JZScriptItem *scriptFile, QVector<GraphPtr> &list)
{
    init(scriptFile);
    if(!genGraphs())
        return false;

    list = m_graphList;
    return true;
}

bool JZNodeCompiler::genNodeInputOuput(JZScriptItem *file,JZScriptInOutInfo &result)
{
    QVector<GraphPtr> graph_list;
    if (!genGraphs(file, graph_list))
        return false;

    GraphPtr graph = graph_list[0];

    QStringList params;
    QStringList inList, outList;
    for (int node_idx = 1; node_idx < graph->topolist.size(); node_idx++)
    {
        auto graph_node = graph->topolist[node_idx];
        auto node = graph->topolist[node_idx]->node;
        if (node->type() == Node_param)
        {
            JZNodeParam *param = dynamic_cast<JZNodeParam*>(node);
            if (!params.contains(param->variable()))
            {
                params << param->variable();
                inList << param->variable();
            }
        }
        else if (node->type() == Node_setParam)
        {
            JZNodeSetParam *param = dynamic_cast<JZNodeSetParam*>(node);
            if (!params.contains(param->variable()))
            {
                params << param->variable();
                outList << param->variable();
            }
        }
    }

    result.inList = inList;
    result.outList = outList;    
    return true;
}

void JZNodeCompiler::log(QString error)
{
    if (!m_builder)
        return;

    m_builder->log(LOG_INFO, error);
}

void JZNodeCompiler::logE(QString tips, const QVariantMap &args)
{
    if (!m_builder)
        return;

    QString error = JZNodeUtils::makeLink(tips, m_scriptItem->itemPath(), args);
    m_builder->log(LOG_ERROR, error);
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

    //遍历所有flow节点
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
    QString check_error;
    auto class_file = m_scriptItem->getClassItem();     
    if(check_error.isEmpty())
    {
        QStringList list = m_scriptItem->localVariableList(true);
        for(int i = 0; i < list.size(); i++)
        {
            for(int j = 0; j < list.size(); j++)
            {
                if(i != j && list[i] == list[j])
                {
                    check_error = list[i] + "重定义";
                    goto check_end;
                }
            }

            auto def = m_scriptItem->localVariable(list[i]);
            QString error;
            if(!checkParamDefine(def,error))
            {
                check_error = error;
                goto check_end;
            }
        }
    }

check_end:
    if(!check_error.isEmpty())
    {
        m_checkError = check_error;
    }

    return check_error.isEmpty();
}

bool JZNodeCompiler::build(JZScriptItem *scriptFile,JZNodeScript *result)
{   
    init(scriptFile);

    m_compilerInfo = CompilerResult();
    m_compilerInfo.result = false;
    if (!genGraphs())
    {        
        m_compilerInfo.checkError = m_checkError;
        return false;
    }
    if (!checkGraphs())
    {        
        m_compilerInfo.checkError = m_checkError;
        return false;
    }

    m_script = result;
    m_script->clear();
    m_script->itemPath = scriptFile->itemPath();
    
    JZScriptClassItem *class_file = project()->getItemClass(scriptFile);
    if (class_file)
    {
        m_className = class_file->className();
    }    

    m_originGraph = nullptr;

    bool check_graph = true;

    QList<GraphNode*> all_event_list;    
    for (int graph_idx = 0; graph_idx < m_graphList.size(); graph_idx++)
    {
        QList<GraphNode*> event_list = m_graphList[graph_idx]->eventList();
        all_event_list << event_list;
        if (event_list.size() == 0)
        {
            auto first = m_graphList[graph_idx]->firstNode();

            NodeCompilerInfo info;
            info.node_id = first->node->id();
            info.error = "孤立节点";
            m_nodeInfo[first->node->id()] = info;
            
            check_graph = false;
        }
        else if (event_list.size() == 1)
        {
            m_originGraph = m_graphList[graph_idx].data();
        }
    }

    if (all_event_list.size() > 1)
    {
        check_graph = false;        
        for (int event_idx = 0; event_idx < all_event_list.size(); event_idx++)
        {
            auto node = all_event_list[event_idx]->node;
            
            NodeCompilerInfo info;
            info.node_id = node->id();
            info.error = "只能有一个event节点";
            m_nodeInfo[node->id()] = info;
        }            
    }    

    if (!check_graph)
        m_originGraph = nullptr;
    if (!m_originGraph)
        goto buildEnd;
           
    //多个连通图
    do
    {
        QList<GraphNode*> event_list = m_originGraph->eventList();

        m_checkError.clear();
        resetStack();        

        //每个连通图有多个event节点
        if (!checkFunction())
            break;

        //更新输入输出
        for (int i = 0; i < m_originGraph->topolist.size(); i++)
        {
            QString error;
            auto graph_node = m_originGraph->topolist[i];
            if (!graph_node->node->updateNode(error))
            {
                NodeCompilerInfo info;
                info.node_id = graph_node->node->id();
                info.error = error;
                m_nodeInfo[info.node_id] = info;
            }
        }
        if (isBuildError())
            break;

        //确保每个flowIn 都被连接
        for(int i = 0; i < m_originGraph->topolist.size(); i++)
        {
            auto graph_node = m_originGraph->topolist[i];
            if(graph_node->node->flowInCount() == 1 && 
                !graph_node->paramIn.contains(graph_node->node->flowIn()))
            {
                NodeCompilerInfo info;
                info.node_id = graph_node->node->id();
                info.error = "需要连接输入流程";
                m_nodeInfo[info.node_id] = info;
            }
        }            
        if (isBuildError())
            break;

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
                    if(!m_scriptItem->checkConnectType(from,to,error))
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
            break;

        updateBuildGraph(event_list); //todo 似乎不需要这个了
        Q_ASSERT(m_buildGraph->topolist.size() > 0);

        auto func_start = m_scriptItem->startNode();
            
        QList<JZNodeIRPtr> flow_statment;
        int start_pc = m_script->statmentList.size();
        if (!buildControlFlow(func_start))
        {
            break;
        }

        int end_pc = m_script->statmentList.size();

        JZNodeEvent *node_event = dynamic_cast<JZNodeEvent*>(event_list[0]->node);
        JZFunctionDefine define = node_event->function();
        addFunction(define, start_pc, end_pc);                
    }while(0);
  
buildEnd:
    m_compilerInfo.checkError = m_checkError;
    if(!m_compilerInfo.checkError.isEmpty())
        logE(m_compilerInfo.checkError);

    auto it = m_nodeInfo.begin();
    while (it != m_nodeInfo.end())
    {        
        auto node = m_scriptItem->getNode(it.key());

        auto& nodeInfo = it.value();
        if (!nodeInfo.error.isEmpty())
        {
            QString name = node->name();
            QVariantMap args;
            args["id"] = nodeInfo.node_id;
            logE(nodeInfo.error, args);
            m_compilerInfo.nodeError[node->id()] = nodeInfo.error;
        }
        it++;
    }
    m_compilerInfo.result = !isBuildError();
    return m_compilerInfo.result;
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
    QList<int> ingore = { Type_auto , Type_arg , Type_argPointer , Type_args };

    auto env = project()->environment();
    auto list = node->paramOutList();
    for (int i = 0; i < list.size(); i++)
    {
        int pin_id = list[i];
        auto pin = node->pin(pin_id);
        if (pin->dataType().size() == 1)
        {
            int data_type = env->nameToType(pin->dataType()[0]);
            if(!ingore.contains(data_type))
                setPinType(node->id(), pin_id, data_type);
        }
    }
}

//更新节点输出，推送设置变量
void JZNodeCompiler::updateFlowOut()
{
    auto it = m_nodeInfo.begin();
    while (it != m_nodeInfo.end())
    {
        //可能有推送到多个
        for (int i = 0; i < it->statmentList.size(); i++)
        {
            if (it->statmentList[i]->type != OP_ComilerFlowOut)
                continue;

            JZNodeIRFlowOut* stmt = (JZNodeIRFlowOut*)it->statmentList[i].data();
            auto to_gemo = paramGemo(stmt->toId);

            QList<JZNodeIRPtr> new_list;
            m_statmentList = &new_list;
            addSetVariableConvert(irId(stmt->toId), irId(stmt->fromId));

            m_statmentList = &it->statmentList;
            replaceStatementList(i, new_list);
            i = i + new_list.size() - 1;       //跳过新加的语句            
        }
        m_statmentList = nullptr;

        it++;
    }
}

JZNode* JZNodeCompiler::currentNode()
{
    if (m_compilerNodeStack.size() == 0)
        return nullptr;

    return m_scriptItem->getNode(m_compilerNodeStack.back().nodeInfo->node_id);
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
    Q_ASSERT(type != Type_auto && type != Type_arg && type != Type_argPointer && type != Type_args);
    Q_ASSERT(type != Type_none && m_originGraph->pin(node_id,prop_id));
    auto &info = m_nodeInfo[node_id];
    info.pinType[prop_id] = type;
}

int JZNodeCompiler::pinType(int node_id, int prop_id)
{
    Q_ASSERT(m_nodeInfo.contains(node_id) && m_nodeInfo[node_id].pinType.contains(prop_id));
    auto &info = m_nodeInfo[node_id];
    return info.pinType[prop_id];
}

int JZNodeCompiler::pinType(JZNodeGemo gemo)
{
    return pinType(gemo.nodeId, gemo.pinId);
}

void JZNodeCompiler::setRefType(QString ref, int type)
{
    Q_ASSERT(getVariableInfo(ref) && m_env->nameToType(getVariableInfo(ref)->type) == Type_auto);
    m_refType[ref] = type;
}

int JZNodeCompiler::refType(QString ref)
{
    Q_ASSERT(getVariableInfo(ref));

    if (m_refType.contains(ref))
        return m_refType[ref];
    else
    {
        auto def = getVariableInfo(ref);
        return m_env->nameToType(def->type);
    }
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
        return in_node->pin(in_gemo.pinId)->isConstValue();
    }
    else
    {
        return true;
    }
}

QString JZNodeCompiler::pinLiteral(int nodeId, int pinId)
{    
    if (m_buildGraph->graphNode(nodeId)->paramIn.contains(pinId))
    {
        auto it = m_buildGraph->graphNode(nodeId)->paramIn.find(pinId);
        if(it->size() != 1)
            return QString();

        auto in_gemo = it->at(0);
        return m_scriptItem->getNode(in_gemo.nodeId)->pinValue(in_gemo.pinId);
    }
    else
    {
        return m_buildGraph->node(nodeId)->pinValue(pinId);
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
        
        auto out_list = node->paramOutList();
        for (int i = 0; i < out_list.size(); i++)
        {
            Q_ASSERT(m_nodeInfo[node->id()].pinType.contains(out_list[i]));            
        }        
    }
    else
    {
        Q_ASSERT(!error.isEmpty());
        if(error != m_ignoreError)
            currentNodeInfo()->error = error;
    }

    popCompilerNode();

    return ret;
}

void JZNodeCompiler::pushCompilerNode(int id)
{
    Q_ASSERT(id >= 0);
    auto node = m_scriptItem->getNode(id);
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
        m_statmentList = &currentNodeInfo()->statmentList;
        Q_ASSERT(m_statmentList->size() == 0);
    }
    m_compilerNodeStack.back().start = nextPc();
}

void JZNodeCompiler::popCompilerNode()
{   
    if (currentNode()->isFlowNode())            
        m_statmentList = nullptr;

    m_compilerNodeStack.pop_back();   
    if(currentNode() && currentNode()->isFlowNode())
        m_statmentList = &currentNodeInfo()->statmentList;
}

int JZNodeCompiler::indexOfStatmentList(QList<JZNodeIRPtr>* statments, int ir_type)
{
    for (int i = 0; i < statments->size(); i++)
    {
        if (statments->at(i)->type == ir_type)
            return i;
    }
    return -1;
}

void JZNodeCompiler::initStatmentStack(QList<JZNodeIRPtr>* statments)
{
    m_statmentStak.clear();
    pushStatmentList(statments);
}

void JZNodeCompiler::pushStatmentList(QList<JZNodeIRPtr>* statments)
{
    m_statmentList = statments;
    m_statmentStak.push_back(statments);
}

void JZNodeCompiler::popStatmentList()
{
    m_statmentStak.pop_back();
    if (m_statmentStak.size() == 0)
        m_statmentList = nullptr;
    else
        m_statmentList = m_statmentStak.back();
}

void JZNodeCompiler::connectGraph(Graph *graph,JZNode *node)
{
    auto it = m_nodeGraph.find(node);
    if(it != m_nodeGraph.end())
        return;

    m_nodeGraph[node] = graph;
    auto lines = m_scriptItem->connectList();
    for (int i = 0; i < lines.size(); i++)
    {        
        auto &line = lines[i];
        if(line.from.nodeId == node->id())
            connectGraph(graph,m_scriptItem->getNode(line.to.nodeId));
        if(line.to.nodeId == node->id())
            connectGraph(graph,m_scriptItem->getNode(line.from.nodeId));
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
    auto node_list = m_scriptItem->nodeList();
    for (int i = 0; i < node_list.size(); i++)
    {
        JZNode *node = m_scriptItem->getNode(node_list[i]);        
        Graph *graph = getGraph(node);
        
        GraphNode *graph_node = new GraphNode();        
        graph_node->node = node;
        graph->m_nodes.insert(node->id(), GraphNodePtr(graph_node));
    }

    auto lines = m_scriptItem->connectList();
    for (int i = 0; i < lines.size(); i++)
    {        
        JZNode *node = m_scriptItem->getNode(lines[i].from.nodeId);
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
            m_checkError = graph->error;
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
            m_checkError = graph->error;
            return false;
        }
    }
    return true;
}    

bool JZNodeCompiler::isBuildError()
{
    if (!m_checkError.isEmpty())
        return false;

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

bool JZNodeCompiler::checkBuildStop()
{
    if (!m_builder)
        return false;

    return m_builder->isBuildInterrupt();
}

void JZNodeCompiler::addFunction(const JZFunctionDefine &define, int start_addr, int end_addr)
{        
    JZFunctionDebugInfo func_debug;
    //编译结果    
    auto it = m_nodeInfo.begin();
    while (it != m_nodeInfo.end())
    {
        auto node = m_scriptItem->getNode(it->node_id);

        NodeInfo info;
        info.name = node->name();
        info.id = it->node_id;
        info.type = it->node_type;
        info.isFlow = node->isFlowNode();
        info.pcRanges << it->ranges;

        auto in_list = node->paramInList();
        for (int i = 0; i < in_list.size(); i++)
        {
            if (node->pin(in_list[i])->flag() & Pin_noCompiler)
                continue;

            NodeParamInfo param_info;
            param_info.define.name = node->pin(in_list[i])->name();
            param_info.define.dataType = pinType(node->id(), in_list[i]);
            param_info.id = in_list[i];
            info.paramIn.push_back(param_info);
        }

        auto out_list = node->paramOutList();
        for (int i = 0; i < out_list.size(); i++)
        {
            if (node->pin(out_list[i])->flag() & Pin_noCompiler)
                continue;

            NodeParamInfo param_info;
            param_info.define.name = node->pin(out_list[i])->name();
            param_info.define.dataType = pinType(node->id(), out_list[i]);
            param_info.id = out_list[i];
            info.paramOut.push_back(param_info);
        }
        func_debug.nodeInfo[it.key()] = info;

        it++;
    }

    JZFunction impl;
    impl.define = define;
    impl.addr = start_addr;
    impl.addrEnd = end_addr;
    impl.path = m_script->itemPath;

    m_script->functionList.push_back(impl);
    m_script->functionDebugList.push_back(func_debug);
}

bool JZNodeCompiler::checkPinInType(int node_id, const QList<int> &prop_list, QString &error)
{   
    auto env = project()->environment();
    auto typeListName = [env](QList<int> types)->QString{
        QStringList list;
        for(int i = 0; i < types.size(); i++)
        {
            list << env->typeToName(types[i]);
        }
        return list.join(",");
    };

    GraphNode *graph = m_buildGraph->graphNode(node_id);
    
    error.clear();
    //获得输入类型    
    QMap<int, int> in_type;
    for (int param_idx = 0; param_idx < prop_list.size(); param_idx++)
    {
        auto prop_in_id = prop_list[param_idx];
        if (m_nodeInfo[node_id].pinType.contains(prop_in_id))
            continue;

        auto pin = graph->node->pin(prop_in_id);
        if (pin->flag() & Pin_noCompiler)
            continue;
        
        int prop_idx = graph->node->paramInList().indexOf(prop_in_id);
        QString pin_name = "输入节点" + QString::number(prop_idx) + ", " + graph->node->pinName(prop_in_id);        
        auto pin_type_list = env->nameListToTypeList(pin->dataType());

        int pin_type = Type_none;
        if (graph->paramIn.contains(prop_in_id))  //有输入
        {
            QList<int> from_type_list;
            auto from_list = graph->paramIn[prop_in_id];
            for (int i = 0; i < from_list.size(); i++)
            {   
                auto from_gemo = from_list[i];
                if (!m_nodeInfo[from_gemo.nodeId].pinType.contains(from_gemo.pinId))
                {
                    error = pin_name + "前置依赖未设置";
                    return false;
                }

                from_type_list.push_back(pinType(from_gemo));
            }
            int from_type = env->upType(from_type_list);
            if (from_type != Type_none)
            {
                pin_type = env->matchType({ from_type }, pin_type_list);
                if (pin_type == Type_arg)
                    pin_type = from_type;
                else if (pin_type == Type_auto)
                    pin_type = from_type;
                else if (pin_type == Type_argPointer)
                {
                    if (JZNodeType::isPointer(from_type))
                        pin_type = from_type;
                    else
                        pin_type = JZNodeType::pointerType(from_type);
                }
            }

            if (pin_type == Type_none)
            {
                error = pin_name + "无法确定输入类型,输出" + typeListName(from_type_list) + ",需要" + pin->dataType().join(",");
                return false;
            }
        }
        else
        {
            if (!pin->value().isEmpty()) //默认值
            {         
                if(pin->dataType().size() == 1)
                {
                    pin_type = pin_type_list[0];
                    if(pin_type == Type_arg || pin_type == Type_auto)
                        pin_type = env->stringType(pin->value());
                    else
                    {
                        if(!checkInitValue(pin_type, pin->value()))
                            pin_type = Type_none;
                    }
                }   
                else if(pin->dataType().size() > 1)
                {
                    int pin_value_type = env->stringType(pin->value());
                    pin_type = env->matchType( {pin_value_type} , pin_type_list);
                }
                if (pin_type == Type_none)
                {
                    error = pin_name + "无法将输入" + pin->value() + "转换为" + pin->dataType().join(",");
                    return false;
                }
            }
            else
            {                
                error = pin_name + "未设置";
                return false;                
            }
        }
        in_type[prop_in_id] = pin_type;
    }

    auto it_type = in_type.begin();
    while (it_type != in_type.end())
    {
        m_nodeInfo[node_id].pinType.insert(it_type.key(), it_type.value());
        it_type++;
    }    
    return error.isEmpty();
}

JZNode* JZNodeCompiler::nextFlowNode(JZNode* node, int flow_out)
{
    Q_ASSERT(node->pin(flow_out)->isFlow() || node->pin(flow_out)->isSubFlow());

    QList<int> next_flow = m_scriptItem->getConnectPin(node->id(), flow_out);
    if (next_flow.size() > 0)
    {
        Q_ASSERT(next_flow.size() == 1);
        auto line = m_scriptItem->getConnect(next_flow[0]);
        return m_scriptItem->getNode(line->to.nodeId);
    }
    else
    {
        return nullptr;
    }
}

bool JZNodeCompiler::buildSubControlFlow(JZNode* node, QList<JZNodeIRPtr>& list)
{
    Q_ASSERT(m_scriptItem);

    QList<JZNode*> node_list;
    auto pre_list = m_statmentList;

    //按顺序编译每个节点
    while (node)
    {
        node_list << node;
        compilerNode(node);
        if (node->flowOutCount() != 0)
            node = nextFlowNode(node, node->flowOut());
        else
            node = nullptr;
    }
    if (isBuildError())
        return false;

    updateFlowOut();

    m_statmentList = &list;
    for (int node_idx = 0; node_idx < node_list.size(); node_idx++)
    {
        auto node = node_list[node_idx];
        if (!node->isFlowNode())
            continue;

        auto& node_info = m_nodeInfo[node->id()];
        appendStatementList(node_info.statmentList);
    }

    m_statmentList = pre_list;
    return true;
}

bool JZNodeCompiler::buildControlFlow(JZNode* start_node)
{        
    QList<JZNodeIRPtr> list;
    if (!buildSubControlFlow(start_node, list))
        return false;

    initStatmentStack(&m_script->statmentList);
    appendStatementList(list);    

    if (isBuildError())
        return false;
    if (!isAllFlowReturn(start_node, 0))
        return false;

    //void 类型的末尾添加return
    if (m_scriptItem->function().paramOut.size() == 0
        && m_statmentList->back()->type != OP_return)
    {
        addStatement(JZNodeIRPtr(new JZNodeIR(OP_return)));
    }
    
    while(true)
    {
        int index = indexOfStatmentList(m_statmentList, OP_ComilerAllocAuto);
        if (index == -1)
            break;

        JZNodeIRAutoInit* auto_alloc = dynamic_cast<JZNodeIRAutoInit*>(m_statmentList->at(index).data());
        
        QList<JZNodeIRPtr> tmp_list;
        pushStatmentList(&tmp_list);

        int data_type = refType(auto_alloc->name);
        addAlloc(JZNodeIRAlloc::Stack, auto_alloc->name, data_type);
        addInitVariable(irRef(auto_alloc->name), data_type, "");

        popStatmentList();
        replaceStatementList(index, tmp_list);
    }

    for (int i = 0; i < m_statmentList->size(); i++)
    {
        int op_type = m_statmentList->at(i)->type;
        if (op_type == OP_ComilerBreakContinue)
        {
            JZNodeComilerBreakContinue* complier_jmp = (JZNodeComilerBreakContinue*)(m_statmentList->at(i).data());

            int jmp_pc = -1;
            if (complier_jmp->isBreak())
            {
                JZNode* parent = breakParentNode(complier_jmp->nodeId);
                auto& info = m_nodeInfo[parent->id()];
                jmp_pc = indexOfStatment(info.breakIr.toStrongRef().data());
            }
            else
            {
                JZNode* parent = continueParentNode(complier_jmp->nodeId);
                auto& info = m_nodeInfo[parent->id()];
                jmp_pc = indexOfStatment(info.continueIr.toStrongRef().data());
            }
            Q_ASSERT(jmp_pc >= 0);

            JZNodeIRJmp* jmp = new JZNodeIRJmp(OP_jmp);
            jmp->jmpPc = jmp_pc;
            replaceStatement(i, JZNodeIRPtr(jmp));
        }
    }

    //init function stack
    auto createAlloc = [](int id,int dataType)->JZNodeIRPtr
    {
        Q_ASSERT(id < Reg_Start);

        JZNodeIRAlloc *alloc = new JZNodeIRAlloc();
        alloc->allocType = JZNodeIRAlloc::StackId;
        alloc->dst = irId(id);
        alloc->dataType = dataType;
        return JZNodeIRPtr(alloc);
    };

    for (int pc = 0; pc < m_statmentList->size(); pc++)
    {
        if (m_statmentList->at(pc)->type != OP_ComilerStackInit)
            continue;

        //节点变量
        QList<JZNodeIRPtr> ir_list;        
        for (int node_idx = 0; node_idx < m_buildGraph->topolist.size(); node_idx++)
        {
            auto node = m_buildGraph->topolist[node_idx]->node;        
            auto in_list = node->paramInList();
            for (int j = 0; j < in_list.size(); j++)
            {
                if (node->pin(in_list[j])->flag() & Pin_noCompiler)
                    continue;

                int pin_id = paramId(node->id(),in_list[j]);
                int pin_type = pinType(node->id(),in_list[j]);
                ir_list << createAlloc(pin_id, pin_type);
            }

            auto out_list = node->paramOutList();
            for (int j = 0; j < out_list.size(); j++)
            {
                if (node->pin(out_list[j])->flag() & Pin_noCompiler)
                    continue;
                
                int pin_id = paramId(node->id(),out_list[j]);
                int pin_type = pinType(node->id(),out_list[j]);
                ir_list << createAlloc(pin_id, pin_type);
            }
        }

        //栈变量
        auto it = m_stackType.begin();
        while(it != m_stackType.end())
        {
            ir_list << createAlloc(it.key(),it.value());
            it++;
        }
        replaceStatementList(pc, ir_list);
    }

    for (int pc = 0; pc < m_statmentList->size(); pc++)
        m_statmentList->at(pc)->pc = pc;

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

bool JZNodeCompiler::isAllFlowReturn(JZNode* node, int level)
{
    if (m_scriptItem->function().paramOut.size() == 0)
        return true;
    if (!node)
        return false;

    JZNode* pre_node = node;
    while (node)
    {
        if (node->type() == Node_throw || node->type() == Node_return)
            return true;
        if (node->type() == Node_break || node->type() == Node_continue)
            return false;

        QList<int> sub_list = node->subFlowList();
        if (sub_list.size() != 0)
        {
            int sub_return_size = 0;
            for (int i = 0; i < sub_list.size(); i++)
            {
                JZNode* sub_node = nextFlowNode(node, sub_list[i]);
                if (isAllFlowReturn(sub_node, level + 1))
                    sub_return_size++;
            }

            if (sub_return_size == sub_list.size())
            {
                if (node->type() == Node_if)
                {
                    JZNodeIf* node_if = (JZNodeIf*)node;
                    if (node_if->hasElse())
                        return true;
                }
                else if (node->type() == Node_switch)
                {
                    JZNodeSwitch* node_switch = (JZNodeSwitch*)node;
                    if (node_switch->hasDefault())
                        return true;
                }
                else
                    return true;
            }
        }

        pre_node = node;
        node = nextFlowNode(node, node->flowOut());
    }

    if (level == 0)
    {
        m_nodeInfo[pre_node->id()].error = "需要连接return";
    }
    return false;
}

int JZNodeCompiler::addStatement(JZNodeIRPtr ir)
{   
    Q_ASSERT(ir->pc == -1);
    ir->pc = m_statmentList->size();    
    m_statmentList->push_back(ir);
    return ir->pc;
}

void JZNodeCompiler::adjustStatementPc(int start_idx,int jmp_cond, int adjust)
{
    for (int i = start_idx; i < m_statmentList->size(); i++)
    {
        auto stmt = m_statmentList->at(i);
        if (stmt->type == OP_jmp || stmt->type == OP_je || stmt->type == OP_jne)
        {            
            JZNodeIRJmp *jmp = (JZNodeIRJmp*)stmt.data();
            if (jmp->jmpPc > jmp_cond)
            {
                int new_jmp = jmp->jmpPc + adjust;
                jmp->jmpPc = new_jmp;
            }
        }
        else if (stmt->type == OP_try)
        {
            JZNodeIRTry* ir_try = (JZNodeIRTry*)stmt.data();
            if(ir_try->catchType == JZNodeIRTry::InTry && ir_try->catchPc > jmp_cond)
                ir_try->catchPc += adjust;
        }
    }
}

JZScriptItem *JZNodeCompiler::currentFile()
{
    return m_scriptItem;
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
    return function(m_scriptItem, name);
}

JZNodeIR *JZNodeCompiler::statment(int index)
{
    return m_statmentList->at(index).data();
}

JZNodeIR *JZNodeCompiler::lastStatment()
{
    if (m_statmentList->size() > 0)
        return m_statmentList->back().data();
    else
        return nullptr;
}

int JZNodeCompiler::indexOfStatment(JZNodeIR *ir)
{
    for (int i = 0; i < m_statmentList->size(); i++)
    {
        if (m_statmentList->at(i).data() == ir)
            return i;
    }
    return -1;
}

void JZNodeCompiler::removeStatement(int pc)
{
    Q_ASSERT(!hasStatementDepend(pc));
    m_statmentList->removeAt(pc);
    adjustStatementPc(0, pc, -1);
}

void JZNodeCompiler::replaceStatement(int pc,JZNodeIRPtr ir)
{
    ir->pc = pc;        
    m_statmentList->replace(pc, ir);
}

void JZNodeCompiler::replaceStatementList(int pc, QList<JZNodeIRPtr> ir_list)
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
    adjustStatementPc(0, pc, pc_add); //这里要用pc，是替换前的地址
}

void JZNodeCompiler::appendStatementList(const QList<JZNodeIRPtr>& ir_list)
{
    Q_ASSERT(m_statmentList != &ir_list);

    int start = m_statmentList->size();
    m_statmentList->append(ir_list);
    adjustStatementPc(start, -1, start);
}

void JZNodeCompiler::setBreakContinue(int breakPc, int continuePc)
{
    if(breakPc >= 0)
        currentNodeInfo()->breakIr = m_statmentList->at(breakPc).toWeakRef();
    if(continuePc >= 0)
        currentNodeInfo()->continueIr = m_statmentList->at(continuePc).toWeakRef();
}

JZNode* JZNodeCompiler::breakParentNode(int child_id)
{
    QVector<int> allow_node = { Node_for,Node_while,Node_foreach,Node_switch };
    int parent_id = m_scriptItem->parentNode(child_id);
    while (parent_id != -1)
    {
        auto node = m_scriptItem->getNode(parent_id);
        if (allow_node.contains(node->type()))        
            return node;
        
        parent_id = m_scriptItem->parentNode(node->id());
    }
    return nullptr;
}

JZNode* JZNodeCompiler::continueParentNode(int child_id)
{
    QVector<int> allow_node = { Node_for,Node_while,Node_foreach };
    int parent_id = m_scriptItem->parentNode(child_id);
    while (parent_id != -1)
    {
        auto node = m_scriptItem->getNode(parent_id);
        if (allow_node.contains(node->type()))
            return node;

        parent_id = m_scriptItem->parentNode(node->id());
    }
    return nullptr;
}

void JZNodeCompiler::addConstructor(SignalConnectInfo info)
{
    for (int i = 0; i < info.irList.size(); i++)
    {
        Q_ASSERT(!info.irList[i].isStack());
    }
    m_builder->addClassConstructor(m_className, info);
}

JZNodeIRJmp* JZNodeCompiler::addJmp(JZNodeIRType type)
{
    JZNodeIRJmp* jmp = new JZNodeIRJmp(type);
    addStatement(JZNodeIRPtr(jmp));
    return jmp;
}

int JZNodeCompiler::addContinue(int node_id)
{
    return addStatement(JZNodeIRPtr(new JZNodeComilerBreakContinue(node_id, JZNodeComilerBreakContinue::Jmp_continue)));
}

int JZNodeCompiler::addBreak(int node_id)
{
    return addStatement(JZNodeIRPtr(new JZNodeComilerBreakContinue(node_id, JZNodeComilerBreakContinue::Jmp_break)));
}

void JZNodeCompiler::addCall(const QString &function_name, const QList<JZNodeIRParam> &paramIn,const QList<JZNodeIRParam> &paramOut)
{    
    auto func = function(function_name);
    addCall(func, paramIn, paramOut); 
}

void JZNodeCompiler::addCall(const JZFunctionDefine *func, const QList<JZNodeIRParam> &paramIn, const QList<JZNodeIRParam> &paramOut)
{
    dealAddCall(false,func,paramIn, paramOut); 
}

void JZNodeCompiler::addCallVirtual(const QString & function_name, const QList<JZNodeIRParam> &paramIn, const QList<JZNodeIRParam> &paramOut)
{
    auto func = function(function_name);
    dealAddCall(true, func,paramIn,paramOut);
}

void JZNodeCompiler::addCallConvert(const QString &function_name, const QList<JZNodeIRParam> &paramIn, const QList<JZNodeIRParam> &paramOut)
{
    auto func = function(function_name);
    addCallConvert(func, paramIn, paramOut); 
}

void JZNodeCompiler::addCallConvert(const JZFunctionDefine *func, const QList<JZNodeIRParam> &org_paramIn, const QList<JZNodeIRParam> &org_paramOut)
{
    Q_ASSERT(func && func->paramIn.size() == org_paramIn.size() && func->paramOut.size() >= org_paramOut.size());

    auto env = project()->environment();
    //分配函数输入
    QList<JZNodeIRParam> param_in;
    for(int i = 0; i < org_paramIn.size(); i++)
    {
        int func_param_type = env->nameToType(func->paramIn[i].type);
        int param_type = irParamType(org_paramIn[i]);
        if(m_env->isSameType(param_type,func_param_type))
        {
            param_in << org_paramIn[i];
        }
        else
        {
            int id = allocStack(func_param_type);
            addConvert(irId(id),func_param_type,org_paramIn[i]);
            param_in << irId(id);
        }
    }   
    
    //分配函数输出
    QList<JZNodeIRParam> param_out;
    for(int i = 0; i < org_paramOut.size(); i++)
    {
        int func_param_type = env->nameToType(func->paramOut[i].type);
        int param_type = irParamType(org_paramOut[i]);
        if(m_env->isSameType(func_param_type, param_type))
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
    //转换函数输出
    for(int i = 0; i < org_paramOut.size(); i++)
    {   
        int func_param_type = env->nameToType(func->paramOut[i].type);
        int param_type = irParamType(org_paramOut[i]);
        if(!m_env->isSameType(func_param_type, param_type))
            addConvert(org_paramOut[i],param_type,param_out[i]);
    }
}

void JZNodeCompiler::dealAddCall(bool isVirtual,const JZFunctionDefine *func, const QList<JZNodeIRParam> &paramIn, const QList<JZNodeIRParam> &paramOut)
{
    Q_ASSERT(func && (func->isVariadicFunction() || func->paramIn.size() == paramIn.size()) && func->paramOut.size() >= paramOut.size());

    setRegCallFunction(func);
    for (int i = 0; i < paramIn.size(); i++)
    {
        addSetVariable(irId(Reg_CallIn + i), paramIn[i]);
        m_regCallInput.push_back(paramIn[i]);
    }
    JZNodeIRCall *call = new JZNodeIRCall();
    call->function = func->fullName();
    call->inCount = paramIn.size();
    call->isVirtual = isVirtual;
    addStatement(JZNodeIRPtr(call));

    for(int i = 0; i < paramOut.size(); i++)
        addSetVariable(paramOut[i],irId(Reg_CallOut + i));

    setRegCallFunction(nullptr);
    m_regCallInput.clear();
    addStatement(JZNodeIRPtr(new JZNodeIR(OP_clearReg)));    
}

bool JZNodeCompiler::hasStatementDepend(int pc)
{
    for (int i = 0; i < m_statmentList->size(); i++)
    {
        auto stmt = m_statmentList->at(i);
        if (stmt->type == OP_jmp || stmt->type == OP_je || stmt->type == OP_jne)
        {
            JZNodeIRJmp *jmp = (JZNodeIRJmp*)stmt.data();
            if (jmp->jmpPc == pc)
                return true;
        }
    }
    return false;
}

void JZNodeCompiler::resetStack()
{
    m_stackId = Stack_User;
    m_nodeInfo.clear();
    m_stackType.clear();
    m_statmentList = nullptr;
    m_regCallFunction = nullptr;
}

int JZNodeCompiler::allocStack(QString type)
{
    return allocStack(m_env->nameToType(type));
}

int JZNodeCompiler::allocStack(int dataType)
{
    Q_ASSERT(dataType != Type_none);

    int id = m_stackId++;    
    m_stackType[id] = dataType;
    return id;        
}

void JZNodeCompiler::setStackId(int id)
{
    m_stackId = id;
}

int JZNodeCompiler::stackId()
{
    return m_stackId;
}

int JZNodeCompiler::stackType(int id)
{
    int type = Type_none;
    if (id < Stack_User)
        type = pinType(paramGemo(id));
    else
        type = m_stackType[id];

    return type;
}

JZNodeIRParam JZNodeCompiler::paramRef(QString name)
{
    auto coor = variableCoor(m_scriptItem,name);
    if (coor == Variable_member && !name.startsWith("this."))
        name = "this." + name;
    return irRef(name);
}

void JZNodeCompiler::addFunctionAlloc(const JZFunctionDefine &define)
{
    auto env = project()->environment();
    int start_pc = nextPc();
    setRegCallFunction(&define);
    for (int i = 0; i < define.paramIn.size(); i++)
    {
        if (i != 0 || !define.isMemberFunction())
        {
            addAlloc(JZNodeIRAlloc::Stack,define.paramIn[i].name, env->nameToType(define.paramIn[i].type));
            addSetVariable(irRef(define.paramIn[i].name),irId(Reg_CallIn + i));
        }
    }
    addStatement(JZNodeIRPtr(new JZNodeIR(OP_clearReg)));
    setRegCallFunction(nullptr);

    auto list = m_scriptItem->localVariableList(false);
    for (int i = 0; i < list.size(); i++)
    {
        auto param = m_scriptItem->localVariable(list[i]);
        if (param->type == "auto")
        {
            addAllocAuto(param->name);
            continue;
        }
        
        int data_type = env->nameToType(param->type);
        addAlloc(JZNodeIRAlloc::Stack, param->name, data_type);
        if(!param->value.isEmpty())
            addInitVariable(irRef(param->name), data_type, param->value);
        else
        {
            if(JZNodeType::isBaseOrEnum(data_type))
                addInitVariable(irRef(param->name), data_type, param->value);
            else if(m_env->meta(param->type)->isValueType())
                addInitVariable(irRef(param->name), data_type, param->value);
        }
    }
    addStatement(JZNodeIRPtr(new JZNodeIRStackInit()));
}

const JZParamDefine *JZNodeCompiler::getVariableInfo(JZScriptItem *file,const QString &name)
{
    if (name.isEmpty())
        return nullptr;

    auto project = file->project();    
    auto obj_inst = project->environment()->objectManager();
    JZScriptClassItem *class_item = file->getClassItem();

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
        
    const JZParamDefine *def = nullptr;
    if (base_name == "this")
        def = class_item? class_item->memberThis() : nullptr;
    else
        def = file->localVariable(base_name);
    if (!def)
        def = project->globalVariable(base_name);
    if (!def)
        return nullptr;

    if (param_name.isEmpty())
        return def;
    else
    {
        auto obj_def = obj_inst->meta(JZNodeType::baseType(def->type));
        if (!obj_def)
            return nullptr;

        return obj_def->param(param_name);
    }
}

const JZFunctionDefine* JZNodeCompiler::function(JZScriptItem* file, const QString& name)
{
    const JZFunctionDefine* func = file->project()->function(name);
    if (func)
        return func;

    auto env = file->project()->environment();
    return env->functionManager()->function(name);
}

void JZNodeCompiler::setRegCallFunction(const JZFunctionDefine *func)
{
    Q_ASSERT((m_regCallFunction == nullptr && func) || (m_regCallFunction && func == nullptr));
    m_regCallFunction = func;
}

bool JZNodeCompiler::checkParamDefine(const JZParamDefine *def, QString &error)
{
    auto env = project()->environment();
    int data_type = env->nameToType(def->type);
    if (def->name.isEmpty())
    {
        error = errorString(Error_varNameEmpty, {});
        return false;
    }
    if(!JZRegExpHelp::isIdentify(def->name))
    {
        error = errorString(Error_varNameInvaild, { def->name });
        return false;
    }
    if (data_type == Type_none)
    {
        error = errorString(Error_noType, { def->type});
        return false;
    }
    if(!checkInitValue(data_type,def->value))
    {
        error = errorString(Errro_initVariableFailed, { def->name, def->value });
        return false;
    }

    return true;
}

const JZParamDefine *JZNodeCompiler::getVariableInfo(const QString &name)
{
    return getVariableInfo(m_scriptItem, name);
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

    return true;
}

bool JZNodeCompiler::checkVariableType(const QString& name, QString data_type, QString& error)
{
    int data_type_id = m_env->nameToType(data_type);
    return checkVariableType(name, data_type_id, error);
}

bool JZNodeCompiler::checkVariableType(const QString &name, int data_type, QString &error)
{
    auto env = project()->environment();
    if (!checkVariableExist(name, error))
        return false;

    auto def = getVariableInfo(name);
    if (!env->canConvert(env->nameToType(def->type),data_type))
    {
        error = name + "不是" + env->typeToName(data_type);
        return false;
    }

    return true;    
}

bool JZNodeCompiler::checkInitValue(int type,const QString & text)
{
    if (text.isEmpty())
        return true;

    if (type == Type_any)
    {
        return checkInitValue(m_env->stringType(text), text);
    }
    else if (type == Type_string)
    {
        return true;
    }
    else if (type == Type_bool)
    {
        return (text == "false" || text == "true");
    }
    else if (type == Type_function)
    {
        return true;
    }
    else if (type == Type_nullptr)
    {
        return text == "null";
    }
    else if (type >= Type_enum && type < Type_class)
    {
        auto meta = m_env->objectManager()->enumMeta(type);
        return meta->hasKey(text);
    }
    else if (type >= Type_class)
    {
        if (text.isEmpty() || text == "null")
            return true;
        if (text.startsWith("{") || text.endsWith("}"))
        {
            auto obj_def = m_env->meta(type);
            if (obj_def->function("__fromString__"))
                return true;
            else
                return false;
        }
        return false;
    }
    else if (JZNodeType::isNumber(type))
    {
        QVariant v = m_env->tryInitValue(type, text);
        return v.isValid();
    }

    return false;
}

bool JZNodeCompiler::addDataInput(int nodeId, const QList<int>& prop_list, QString &error)
{    
    if (!checkPinInType(nodeId, prop_list, error))
        return false;
    
    auto func_inst = m_env->functionManager();
    GraphNode* in_node = m_buildGraph->graphNode(nodeId);
    for (int prop_idx = 0; prop_idx < prop_list.size(); prop_idx++)
    {
        int in_prop = prop_list[prop_idx];
        auto pin = in_node->node->pin(in_prop);
        if (pin->flag() & Pin_noCompiler)
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
                func = func_inst->function(func_node->function());
                is_member_function = func->isMemberFunction();
            }
            
            int to_id = paramId(in_node->node->id(), in_prop); 
            if (pin->value().isEmpty())
            {
                Q_ASSERT(false);
                return false;                
            }            
            else
            {                
                int match_type = pinType(nodeId,in_prop);
                addInitVariable(irId(to_id),match_type,pin->value());                
            }
        }
    }

    Q_ASSERT(m_compilerNodeStack.back().nodeInfo->node_id == nodeId);
    if(m_compilerNodeStack.back().debugStart == -1 && m_nodeInfo[nodeId].autoAddDebugStart)
        addNodeEnter(nodeId);

    return true;
}

bool JZNodeCompiler::addDataInput(int nodeId,QString &error)
{
    QList<int> prop_list = m_buildGraph->node(nodeId)->paramInList();
    return addDataInput(nodeId, prop_list, error);
}

bool JZNodeCompiler::addFlowInput(int nodeId, const QList<int>& prop_list, QString &error)
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
                //初始节点额外根据prop_list筛选
                if (in_node->node->id() == nodeId && !prop_list.contains(it.key())) 
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
                        if (!node_event && m_scriptItem->getConnectPin(n->id(), n->flowIn()).size() == 0)
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
    {
        error = m_ignoreError;
        return false;
    }
    if (!addDataInput(nodeId, prop_list, error))
        return false;

    return true;
}

bool JZNodeCompiler::addFlowInput(int nodeId,QString &error)
{
    QList<int> prop_list = m_buildGraph->node(nodeId)->paramInList();
    return addFlowInput(nodeId, prop_list, error);
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

void JZNodeCompiler::addAlloc(int allocType, QString name, QString dataType)
{
    addAlloc(allocType, name, m_env->nameToType(dataType));
}

void JZNodeCompiler::addAlloc(int allocType, QString name, int dataType)
{
    Q_ASSERT(dataType != Type_none);
    
    JZNodeIRAlloc *alloc = new JZNodeIRAlloc();
    alloc->allocType = allocType;
    alloc->dst = irRef(name);
    alloc->dataType = dataType;
    addStatement(JZNodeIRPtr(alloc));
}

void JZNodeCompiler::addAllocAuto(const QString& name)
{
    JZNodeIRAutoInit *alloc = new JZNodeIRAutoInit();
    alloc->name = name;
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

int JZNodeCompiler::addNodeEnter(int id)
{    
    JZNodeIRNodeEnter *node_ir = new JZNodeIRNodeEnter();
    node_ir->id = id;    
    m_compilerNodeStack.back().debugStart = addStatement(JZNodeIRPtr(node_ir));
    return node_ir->pc;    
}

void JZNodeCompiler::setAutoaddNodeEnter(int m_id,bool flag)
{
    m_nodeInfo[m_id].autoAddDebugStart = flag;
}

int JZNodeCompiler::addExpr(const JZNodeIRParam &dst,const JZNodeIRParam &p1,const JZNodeIRParam &p2, JZNodeIRType op)
{
    Q_ASSERT(irParamTypeMatch(p1,p2,false));
    Q_ASSERT(JZNodeType::calcExprType(irParamType(p1),irParamType(p2),op) == irParamType(dst));

    JZNodeIRExpr *expr = new JZNodeIRExpr(op);
    expr->src1 = p1;
    expr->src2 = p2;
    expr->dst = dst;
    return addStatement(JZNodeIRPtr(expr));
}

int JZNodeCompiler::addSingleExpr(const JZNodeIRParam &dst, const JZNodeIRParam &p1, JZNodeIRType op)
{
    JZNodeIRExpr *expr = new JZNodeIRExpr(op);
    expr->src1 = p1;
    expr->dst = dst;
    return addStatement(JZNodeIRPtr(expr));
}

void JZNodeCompiler::addExprConvert(const JZNodeIRParam &dst, const JZNodeIRParam &p1, const JZNodeIRParam &p2, JZNodeIRType op)
{   
    auto env = project()->environment();
    int t1 = irParamType(p1);
    int t2 = irParamType(p2);
    int tDst = irParamType(dst);
    int upType = env->upType(t1,t2);
    int dstType = JZNodeType::calcExprType(upType,upType,op);

    JZNodeIRParam tmp_dst = dst;
    JZNodeIRParam tmp_p1 = p1;
    JZNodeIRParam tmp_p2 = p2;
    if(t1 != upType)
    {
        int id = allocStack(upType);
        addConvert(irId(id),upType,p1);
        tmp_p1 = irId(id);
    }
    if(t2 != upType)
    {
        int id = allocStack(upType);
        addConvert(irId(id),upType,p2);
        tmp_p2 = irId(id);
    }
    if(tDst != dstType)
    {
        int id = allocStack(dstType);
        tmp_dst = irId(id);
    }

    addExpr(tmp_dst,tmp_p1,tmp_p2,op);
    if(tDst != dstType)
        addConvert(dst,tDst,tmp_dst);
}

int JZNodeCompiler::addCompare(const JZNodeIRParam &p1,const JZNodeIRParam &p2, JZNodeIRType op)
{
    Q_ASSERT(irParamTypeMatch(p1,p2,false));
    return addExpr(irId(Reg_Cmp),p1,p2,op);    
}

void JZNodeCompiler::addCompareConvert(const JZNodeIRParam &p1, const JZNodeIRParam &p2, JZNodeIRType op)
{
    addCompare(p1,p2,op);
}

int JZNodeCompiler::irParamType(const JZNodeIRParam &param)
{
    auto env = project()->environment();
    int type = Type_none;    
    if (param.isReg())
    {
        int id = param.id();
        if (id == Reg_Cmp)
            type = Type_bool;
        else
        {
            auto func = m_regCallFunction;
            Q_ASSERT(m_regCallFunction);
            if (id < Reg_CallOut) //in
            {
                if (func->isVariadicFunction() && (id - Reg_CallIn >= func->paramIn.size() - 1))
                    type = Type_arg;
                else
                    type = env->nameToType(func->paramIn[id - Reg_CallIn].type);
            }
            else
            {
                if (func->name == "createObject")
                {
                    QString obj_name = m_regCallInput[0].literal().toString();
                    type = env->nameToType(obj_name);
                }
                else
                {
                    type = env->nameToType(func->paramOut[id - Reg_CallOut].type);
                }
            }
        }
    }
    else if(param.isStack())
    {
        int id = param.id();        
        if (id < Stack_User)
            type = pinType(paramGemo(param.id()));                            
        else
            type = m_stackType[param.id()];        
    }
    else if(param.isLiteral())
        type = JZNodeType::variantType(param.literal());
    else if(param.isRef())
    {
        type = refType(param.ref());
    }
    else if(param.isThis())
        type = JZNodeType::pointerType(env->nameToType(m_className));
    
    Q_ASSERT(type != Type_none);
    return type;
}

bool JZNodeCompiler::irParamTypeMatch(const JZNodeIRParam &p1,const JZNodeIRParam &p2,bool isSet)
{
    auto env = project()->environment();
    int t1 = irParamType(p1);
    int t2 = irParamType(p2);
    if(isSet)
        return env->isSameType(t1,t2);
    else  //compare
        return env->isSameType(t1,t2) || env->isSameType(t2,t1);
}

void JZNodeCompiler::addInitVariable(const JZNodeIRParam &dst, int dataType, const QString &value)
{
    auto env = project()->environment();
    auto obj_inst = env->objectManager();
    if(dataType < Type_class)
        addSetVariable(dst,irLiteral(env->initValue(dataType,value)));
    else
    {   
        if (value.isEmpty() || value == "{}")
        {
            QList<JZNodeIRParam> in, out;
            in << irLiteral(env->typeToName(dataType));
            out << dst;
            addCall("createObject", in, out);
        }
        else if(value.startsWith("{") && value.endsWith("}"))
        {
            QList<JZNodeIRParam> in,out;
            in << irLiteral(value);
            out << dst;

            auto meta = obj_inst->meta(dataType);
            auto func = meta->function("__fromString__");
            addCall(func,in,out);
        }
        else
        {
            Q_ASSERT(0);
        }
    }
}

void JZNodeCompiler::addSetVariable(const JZNodeIRParam &dst, const JZNodeIRParam &src)
{    
    Q_ASSERT_X(irParamTypeMatch(src,dst,true),"",qUtf8Printable("set " + m_env->typeToName(irParamType(src))
        + " to " + m_env->typeToName(irParamType(dst))));

    auto obj_inst = m_env->objectManager();
    bool clone = false;
    int dst_type = irParamType(dst);
    if (dst_type == Type_auto)
    {
        auto gemo = paramGemo(dst.id());
        setPinType(gemo.nodeId,gemo.pinId,irParamType(src));
    }
    if(!JZNodeType::isPointer(dst_type) && dst_type >= Type_class)
    {
        auto meta = obj_inst->meta(dst_type);
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
        addConvert(dst,to_type,src);
    }
}

void JZNodeCompiler::addSetJson(const JZNodeIRParam& obj,const QString &name, const JZNodeIRParam& value)
{
    Q_ASSERT(irParamType(obj) == Type_jsonObject);

    QList<JZNodeIRParam> in, out;
    in << obj << irLiteral(name) << value;    
    addCallConvert("QJsonObject::set", in, out);
}

void JZNodeCompiler::addGetJson(const JZNodeIRParam& dst, const QString &name, const JZNodeIRParam &obj)
{
    Q_ASSERT(irParamType(obj) == Type_jsonObject);

    QList<JZNodeIRParam> in, out;
    in << obj << irLiteral(name);
    out << dst;
    addCallConvert("QJsonObject::set", in, out);
}

void JZNodeCompiler::addSetBuffer(const JZNodeIRParam &id, const QByteArray &buffer)
{
    Q_ASSERT(irParamType(id) == Type_byteArray);

    JZNodeIRBuffer *op = new JZNodeIRBuffer();
    op->id = id;
    op->buffer = buffer;
    addStatement(JZNodeIRPtr(op));
}

void JZNodeCompiler::addConvert(const JZNodeIRParam &dst, int dst_type,const JZNodeIRParam &src)
{
    Q_ASSERT_X(m_env->canConvertExplicitly(irParamType(src),dst_type),"convert failed",qUtf8Printable("convert " + m_env->typeToName(irParamType(src))
        + " to " + m_env->typeToName(irParamType(dst))));
    Q_ASSERT(project()->environment()->isSameType(dst_type,irParamType(dst)));

    JZNodeIRConvert *op = new JZNodeIRConvert();    
    op->dst = dst;
    op->src = src;
    op->dstType = dst_type;
    addStatement(JZNodeIRPtr(op));
}