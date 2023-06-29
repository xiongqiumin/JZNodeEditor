#include "JZNodeProgram.h"
#include "JZNodeCompiler.h"
#include <QFile>

//TopoTool
TopoTool::TopoTool()
{

}

void TopoTool::addDepend(int id,QStringList in,QStringList out)
{
    TopoNode node;
    node.id = id;
    node.in = in;
    node.out = out;
    m_list.push_back(node);
}

bool TopoTool::toposort(QList<int> &result)
{
    QMap<QString,QList<int>> inList;
    for(int i = 0; i < m_list.size(); i++)
    {
        auto &node = m_list[i];
        for(int in_idx = 0; in_idx < node.in.size(); in_idx++)
            inList[node.in[in_idx]] << i;
    }

    QMap<int, int> nodeMap;
    for(int i = 0; i < m_list.size(); i++)
        nodeMap[i] = 0;

    for(int i = 0; i < m_list.size(); i++)
    {
        auto &node = m_list[i];
        node.next.clear();

        for(int out_idx = 0; out_idx < node.out.size(); out_idx++)
        {
            auto it = inList.find(node.out[out_idx]);
            if(it == inList.end())
                continue;

            auto &next = it.value();
            for (int j = 0; j < next.size(); j++)
                nodeMap[next[j]]++;

            node.next << next;
        }
    }

    while (true)
    {
        QList<TopoNode*> tmp;
        auto it = nodeMap.begin();
        while (it != nodeMap.end())
        {
            if (it.value() == 0)
            {
                auto &cur_node = m_list[it.key()];
                auto &next = cur_node.next;
                for (int i = 0; i < next.size(); i++)
                    nodeMap[next[i]]--;

                tmp.push_back(&cur_node);
                it = nodeMap.erase(it);
            }
            else
                it++;
        }
        if (tmp.size() == 0)
        {
            if (nodeMap.size() != 0)
            {
                return false;
            }
            break;
        }
        std::sort(tmp.begin(), tmp.end(), [](const TopoNode *n1, const TopoNode *n2)
                  { return n1->id < n2->id; });
        for(int i = 0; i < tmp.size(); i++)
            result.push_back(tmp[i]->id);
    }

    return true;
}

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
    if(it != m_nodes.end())
        return it.value().data();        
    else
        return nullptr;
}
    
JZNode *Graph::node(int id)
{
    auto node = graphNode(id);
    if(node)
        return node->node;
    else
        return nullptr;
}

JZNodePin *Graph::pin(JZNodeGemo gemo)
{
    return pin(gemo.nodeId,gemo.propId);
}

JZNodePin *Graph::pin(int nodeId,int pinId)
{
    auto n = node(nodeId);
    if(n)
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

//JZEventHandle
JZEventHandle::JZEventHandle()
{    
}

QDataStream &operator<<(QDataStream &s, const JZEventHandle &param)
{
    s << param.type;
    s << param.sender;
    s << param.function;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZEventHandle &param)
{
    s >> param.type;
    s >> param.sender;
    s >> param.function;
    return s;
}

//JZParamChangeHandle
JZParamChangeHandle::JZParamChangeHandle()
{

}

QDataStream &operator<<(QDataStream &s, const JZParamChangeHandle &param)
{
    s << param.paramName;
    s << param.function;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZParamChangeHandle &param)
{
    s >> param.paramName;
    s >> param.function;
    return s;
}


//NodeInfo
NodeInfo::NodeInfo()
{
    node_id = -1;
    node_type = Node_none;
}

QDataStream &operator<<(QDataStream &s, const NodeInfo &param)
{
    s << param.node_id;
    s << param.node_type;
    return s;
}

QDataStream &operator>>(QDataStream &s, NodeInfo &param)
{
    s >> param.node_id;
    s >> param.node_type;
    return s;
}

//JZNodeScript
JZNodeScript::JZNodeScript()
{

}

void JZNodeScript::clear()
{
    file.clear();
    graphs.clear();
    events.clear();
    statmentList.clear();
    functionList.clear();
    nodeInfo.clear();
}

FunctionDefine *JZNodeScript::function(QString name)
{
    for(int i = 0; i < functionList.size(); i++)
    {
        if(functionList[i].name == name)
            return &functionList[i];
    }
    return nullptr;
}

void JZNodeScript::saveToStream(QDataStream &s)
{
    s << file;    
    s << className;
    s << statmentList.size();
    for(int i = 0; i < statmentList.size(); i++)
    {
        s << statmentList[i]->type;
        statmentList[i]->saveToStream(s);
    }    
    s << functionList;
    s << nodeInfo;
    s << localVariable;    
    s << events;
    s << paramChanges;
    s << watchList;    
}

void JZNodeScript::loadFromStream(QDataStream &s)
{
    s >> file;
    s >> className;
    int stmt_size = 0;
    s >> stmt_size;
    for(int i = 0; i < stmt_size; i++)
    {
        int type;
        s >> type;
        JZNodeIR *ir = createNodeIR(type);
        ir->loadFromStream(s);
        statmentList.push_back(JZNodeIRPtr(ir));
    }
    s >> functionList;
    s >> nodeInfo;
    s >> localVariable;    
    s >> events;
    s >> paramChanges;
    s >> watchList;    
}


//JZNodeProgram
JZNodeProgram::JZNodeProgram()
{
    m_opNames = QStringList{"+","-","*","/","%","==","!=",">",">=","<","<=","&&","||","|","&","^"};
}

JZNodeProgram::~JZNodeProgram()
{
}

void JZNodeProgram::clear()
{
    m_scripts.clear();    
    m_variables.clear();
    m_functionDefines.clear();
    m_objectDefines.clear();    
}

bool JZNodeProgram::load(QString filepath)
{   
    QFile file(filepath);
    if(!file.open(QFile::ReadOnly))
        return false;

    QByteArray magic;
    QDataStream s(&file);
    int script_size;
    s >> magic;
    if(magic != NodeMagic())
        return false;
        
    s >> script_size;
    for(int i = 0; i < script_size; i++)
    {
        QString path;
        s >> path;
        JZNodeScript *script = new JZNodeScript();
        script->loadFromStream(s);
        m_scripts[path] = JZNodeScriptPtr(script);       
    }
    s >> m_variables;
    s >> m_functionDefines;
    s >> m_objectDefines;    
    return true;
}
    
bool JZNodeProgram::save(QString filepath)
{
    QFile file(filepath);
    if(!file.open(QFile::WriteOnly | QFile::Truncate))
        return false;

    QDataStream s(&file);    
    s << NodeMagic();
    s << m_scripts.size();
    auto it = m_scripts.begin();
    while (it != m_scripts.end())
    {
        s << it.key();
        it.value()->saveToStream(s);
        it++;
    }        
    s << m_variables;
    s << m_functionDefines;
    s << m_objectDefines;    
    return true;
}

FunctionDefine *JZNodeProgram::function(QString name)
{    
    for(int i = 0; i < m_functionDefines.size(); i++)
    {
        if(m_functionDefines[i].name == name)
            return &m_functionDefines[i];
    }
    return nullptr;
}

JZNodeScript *JZNodeProgram::script(QString name)
{
    if(!m_scripts.contains(name))
        return nullptr;

    return m_scripts[name].data();
}

QList<JZNodeScript*> JZNodeProgram::scriptList()
{
    QList<JZNodeScript*> list;
    auto it = m_scripts.begin();
    while(it != m_scripts.end())
    {
        if(it.value()->className.isEmpty())
        {
            list << it.value().data();
        }
        it++;
    }
    return list;
}

JZNodeScript *JZNodeProgram::objectScript(QString name)
{
    QString path = "./" + name + "/事件";
    return script(path);
}

QList<JZEventHandle*> JZNodeProgram::eventList() const
{
    QList<JZEventHandle*> result;
    auto it = m_scripts.begin();
    while(it != m_scripts.end())
    {
        if(it.value()->className.isEmpty())
        {
            QList<JZEventHandle> &handle_list = it.value()->events;
            for(int i = 0; i < handle_list.size(); i++)
                result.push_back(&handle_list[i]);
        }
        it++;
    }
    return result;
}

QList<JZNodeObjectDefine> JZNodeProgram::objectDefines()
{
    return m_objectDefines;
}

QMap<QString,JZParamDefine> JZNodeProgram::variables()
{
    return m_variables;
}

QString JZNodeProgram::paramName(JZNodeIRParam param)
{
    if(param.type == JZNodeIRParam::Literal)
        return "$" + param.value.toString();
    else if(param.type == JZNodeIRParam::Reference)
        return param.ref();
    else if(param.type == JZNodeIRParam::This)
        return "this";
    else
        return JZNodeCompiler::paramName(param.id());
}

QString JZNodeProgram::dump()
{    
    QString content;    
    auto it = m_scripts.begin();
    while(it != m_scripts.end())
    {
        JZNodeScript *script = it.value().data();
        content += "// Script " + it.key() + "\n";
        auto &opList = script->statmentList;
        for(int i = 0; i < opList.size(); i++)
        {
            //deal op
            JZNodeIR *op = opList[i].data();
            QString line = QString::asprintf("%04d ",i);
            switch (op->type)
            {
                case OP_nodeId:
                {
                    JZNodeIRNodeId *ir_node = (JZNodeIRNodeId*)op;
                    line += "Node" + QString::number(ir_node->id);
                    break;
                }
                case OP_nop:
                {
                    line += "NOP";
                    break;
                }
                case OP_set:
                {
                    JZNodeIRSet *ir_set = (JZNodeIRSet*)op;
                    line += "SET " + paramName(ir_set->dst) + " = " + paramName(ir_set->src);
                    break;
                }
                case OP_get:
                {
                    Q_ASSERT(0);
                    break;
                }
                case OP_call:
                {
                    JZNodeIRCall *ir_call = (JZNodeIRCall *)op;
                    line += "CALL " + paramName(ir_call->function);
                    break;
                }
                case OP_return:
                    line += "RETURN";
                    break;
                case OP_exit:
                    line += "EXIT";
                    break;
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
                    JZNodeIRExpr *ir_expr = (JZNodeIRExpr *)op;
                    QString c = paramName(ir_expr->dst);
                    QString a = paramName(ir_expr->src1);
                    QString b = paramName(ir_expr->src2);
                    line += c + " = " + a + " " + m_opNames[op->type - OP_add] + " " + b;
                    break;
                }
                case OP_jmp:
                case OP_jne:
                case OP_je:
                {
                    JZNodeIRJmp *ir_jmp = (JZNodeIRJmp *)op;
                    if(op->type == OP_jmp)
                        line += "JMP " + QString::number(ir_jmp->jmpPc);
                    else if(op->type == OP_je)
                        line += "JE " + QString::number(ir_jmp->jmpPc);
                    else
                        line += "JNE " + QString::number(ir_jmp->jmpPc);
                    break;
                }
                default:
                    Q_ASSERT(0);
                    break;
            }

            if(!op->memo.isEmpty())
            {
                line = line.leftJustified(12);        
                line += " //" + op->memo;
            }
            content += line + "\n";
        }

        for(int i = 0; i < script->events.size(); i++)
        {
            if(i == 0)
                content += "event handles:\n";

            auto &event = script->events[i];
            QString line = event.function.name + " addr: " + QString::number(event.function.addr);
            content += line + "\n";
        }

        it++;
    }    
    return content;
}
