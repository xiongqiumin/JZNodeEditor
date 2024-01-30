#include "JZNodeProgram.h"
#include "JZNodeCompiler.h"
#include <QFile>

//NodeRange
NodeRange::NodeRange()
{
    start = -1;
    end = -1;
}

QDataStream &operator<<(QDataStream &s, const NodeRange &param)
{
    s << param.start;
    s << param.end;
    return s;
}
QDataStream &operator>>(QDataStream &s, NodeRange &param)
{
    s >> param.start;
    s >> param.end;
    return s;
}

//NodeInfo
NodeInfo::NodeInfo()
{
    node_id = -1;
    node_type = Node_none;
    isFlow = false;
}

int NodeInfo::indexOfRange(int pc)
{
    for (int i = 0; i < pcRanges.size(); i++)
    {
        if (pc >= pcRanges[i].start && pc < pcRanges[i].end)
            return i;
    }
    return -1;
}

QDataStream &operator<<(QDataStream &s, const NodeInfo &param)
{
    s << param.node_id;
    s << param.node_type;
    s << param.isFlow;
    s << param.paramIn;
    s << param.paramInId;
    s << param.paramOut;
    s << param.paramOutId;
    s << param.pcRanges;
    return s;
}

QDataStream &operator>>(QDataStream &s, NodeInfo &param)
{
    s >> param.node_id;
    s >> param.node_type;
    s >> param.isFlow;
    s >> param.paramIn;
    s >> param.paramInId;
    s >> param.paramOut;
    s >> param.paramOutId;
    s >> param.pcRanges;
    return s;
}

//JZFunction
QDataStream &operator<<(QDataStream &s, const JZFunction &param)
{
    s << param.file << param.localVariables;
    return s;
}

QDataStream &operator >> (QDataStream &s, JZFunction &param)
{
    s >> param.file >> param.localVariables;
    return s;
}

//JZNodeScript
JZNodeScript::JZNodeScript()
{

}

void JZNodeScript::clear()
{
    file.clear();        
    statmentList.clear();
    functionList.clear();
    nodeInfo.clear();
    runtimeInfo.clear();
}

FunctionDefine *JZNodeScript::function(QString name)
{
    for(int i = 0; i < functionList.size(); i++)
    {
        if(functionList[i].fullName() == name)
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
    s << runtimeInfo;
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
    s >> runtimeInfo;
}

//JZNodeProgram
JZNodeProgram::JZNodeProgram()
{    
}

JZNodeProgram::~JZNodeProgram()
{
}

void JZNodeProgram::clear()
{
    m_scripts.clear();    
    m_variables.clear();    
    m_objectDefines.clear();        
}

QString JZNodeProgram::error()
{
    return m_error;
}

bool JZNodeProgram::load(QString filepath)
{   
    QFile file(filepath);
    if (!file.open(QFile::ReadOnly))
    {
        m_error = "open file failed";
        return false;
    }

    QByteArray magic;
    QDataStream s(&file);
    int script_size;
    s >> magic;
    if(magic != NodeMagic())
    {
        m_error = "version not support";
        return false;
    }
        
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
    s << m_objectDefines;      
    return true;
}

JZNodeScript *JZNodeProgram::script(QString path)
{
    return m_scripts.value(path, JZNodeScriptPtr()).data();
}

FunctionDefine *JZNodeProgram::function(QString name)
{    
    auto it = m_scripts.begin();
    while (it != m_scripts.end())
    {
        FunctionDefine *def = it->data()->function(name);
        if (def)
            return def;

        it++;
    }
    return nullptr;
}

JZFunction *JZNodeProgram::runtimeInfo(QString function)
{
    auto it = m_scripts.begin();
    while (it != m_scripts.end())
    {
        auto run_it = it->data()->runtimeInfo.find(function);
        if (run_it != it->data()->runtimeInfo.end())
            return &run_it.value();

        it++;
    }
    return nullptr;
}

QList<JZNodeScript*> JZNodeProgram::scriptList()
{
    QList<JZNodeScript*> list;
    auto it = m_scripts.begin();
    while(it != m_scripts.end())
    {
        list << it.value().data();        
        it++;
    }
    return list;
}

JZNodeScript *JZNodeProgram::objectScript(QString name)
{
    QString path = "./" + name + "/事件";
    return script(path);
}

QList<FunctionDefine> JZNodeProgram::functionDefines()
{
    QList<FunctionDefine> result;
    auto it = m_scripts.begin();
    while (it != m_scripts.end())
    {
        auto list = it->data()->functionList;        
        it++;
    }
    return result;
}

QList<JZNodeObjectDefine> JZNodeProgram::objectDefines()
{
    return m_objectDefines;
}

QMap<QString,JZParamDefine> JZNodeProgram::globalVariables()
{
    return m_variables;
}

QString JZNodeProgram::toString(JZNodeIRParam param)
{
    if (param.type == JZNodeIRParam::Literal)
    {
        if(param.value.type() == QVariant::String)
            return "\"" + param.value.toString() + "\"";
        else
            return "$" + param.value.toString();
    }
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
                case OP_alloc:
                {
                    JZNodeIRAlloc *ir_alloc = (JZNodeIRAlloc*)op;
                    QString alloc = (ir_alloc->allocType == JZNodeIRAlloc::Heap) ? "Global" : "Local";
                    line += alloc + " " + ir_alloc->name + " = " + toString(ir_alloc->value);
                    break;
                }
                case OP_set:
                {
                    JZNodeIRSet *ir_set = (JZNodeIRSet*)op;
                    line += "SET " + toString(ir_set->dst) + " = " + toString(ir_set->src);
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
                    line += "CALL " + toString(ir_call->function);
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
                    QString c = toString(ir_expr->dst);
                    QString a = toString(ir_expr->src1);
                    QString b = toString(ir_expr->src2);
                    line += c + " = " + a + " " + JZNodeType::opName(op->type) + " " + b;
                    break;
                }
                case OP_not:
                {
                    JZNodeIRExpr *ir_expr = (JZNodeIRExpr *)op;
                    QString c = toString(ir_expr->dst);
                    QString a = toString(ir_expr->src1);                    
                    line += c + " = " + JZNodeType::opName(op->type) + " " + a;
                    break;
                }
                case OP_jmp:
                case OP_jne:
                case OP_je:
                {
                    JZNodeIRJmp *ir_jmp = (JZNodeIRJmp *)op;
                    if(op->type == OP_jmp)
                        line += "JMP " + toString(ir_jmp->jmpPc);
                    else if(op->type == OP_je)
                        line += "JE " + toString(ir_jmp->jmpPc);
                    else
                        line += "JNE " + toString(ir_jmp->jmpPc);
                    break;
                }
                case OP_assert:
                {
                    JZNodeIRAssert *ir_assert = (JZNodeIRAssert *)op;
                    line += "ASSERT " + toString(ir_assert->tips);
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

        for(int i = 0; i < script->functionList.size(); i++)
        {
            if(i == 0)
                content += "functions:\n";

            auto &func = script->functionList[i];
            QString line = func.name + " addr: " + QString::number(func.addr);
            content += line + "\n";
        }

        it++;
    }    
    return content;
}
