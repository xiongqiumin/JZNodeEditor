#include "JZNodeProgram.h"
#include "JZNodeCompiler.h"
#include <QFile>
#include <QDebug>

//NodeRange
NodeRange::NodeRange()
{
    start = -1;
    debugStart = -1;
    end = -1;
}

QDataStream &operator<<(QDataStream &s, const NodeRange &param)
{
    s << param.start;
    s << param.debugStart;
    s << param.end;
    return s;
}
QDataStream &operator>>(QDataStream &s, NodeRange &param)
{
    s >> param.start;
    s >> param.debugStart;
    s >> param.end;
    return s;
}

//NodeParamInfo
QDataStream &operator<<(QDataStream &s, const NodeParamInfo &param)
{
    s << param.define;
    s << param.id;
    return s;
}

QDataStream &operator>>(QDataStream &s, NodeParamInfo &param)
{
    s >> param.define;
    s >> param.id;
    return s;
}

//NodeInfo
NodeInfo::NodeInfo()
{
    node_id = -1;
    node_type = Node_none;
    isFlow = false;
}

QDataStream &operator<<(QDataStream &s, const NodeInfo &param)
{
    s << param.node_id;
    s << param.node_type;
    s << param.isFlow;
    s << param.paramIn;    
    s << param.paramOut;
    s << param.pcRanges;
    return s;
}

QDataStream &operator>>(QDataStream &s, NodeInfo &param)
{
    s >> param.node_id;
    s >> param.node_type;
    s >> param.isFlow;
    s >> param.paramIn;    
    s >> param.paramOut;    
    s >> param.pcRanges;
    return s;
}

//NodeWatch
QDataStream &operator<<(QDataStream &s, const NodeWatch &param)
{
    s << param.source << param.traget;
    return s;
}

QDataStream &operator >> (QDataStream &s, NodeWatch &param)
{
    s >> param.source >> param.traget;
    return s;
}

//JZNodeScript
JZNodeScript::JZNodeScript()
{

}

void JZNodeScript::clear()
{
    file.clear();
    className.clear();
    nodeInfo.clear();
    statmentList.clear();
    functionList.clear();
    watchList.clear();
}

JZFunction *JZNodeScript::function(QString name)
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
    s >> watchList;
}

//JZNodeProgram
JZNodeProgram::JZNodeProgram()
{    
}

JZNodeProgram::~JZNodeProgram()
{
}

bool JZNodeProgram::isNull()
{
    return m_filePath.isEmpty();
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

QString JZNodeProgram::applicationFilePath()
{
    return m_filePath;
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
    s >> m_binds;

    m_filePath = filepath;
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
    s << m_binds;
    
    m_filePath = filepath;
    return true;
}

JZNodeScript *JZNodeProgram::script(QString path)
{
    return m_scripts.value(path, JZNodeScriptPtr()).data();
}

QStringList JZNodeProgram::functionList()
{
    QStringList list;

    auto it = m_scripts.begin();
    while (it != m_scripts.end())
    {
        auto &sub_list = it.value()->functionList;
        for (int i = 0; i < sub_list.size(); i++)
            list << sub_list[i].fullName();

        it++;
    }
    return list;
}

JZFunction *JZNodeProgram::function(QString name)
{    
    auto it = m_scripts.begin();
    while (it != m_scripts.end())
    {
        JZFunction *def = it->data()->function(name);
        if (def)
            return def;

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

QMap<QString, JZNodeParamBind> JZNodeProgram::bindInfo(QString className)
{
    return m_binds.value(className);
}

QList<JZNodeObjectDefine> JZNodeProgram::objectDefines()
{
    return m_objectDefines;
}

QString JZNodeProgram::toString(JZNodeIRParam param)
{
    if (param.type == JZNodeIRParam::Literal)
    {
        if (param.value.type() == QVariant::String)
            return "\"" + param.value.toString() + "\"";
        else if (JZNodeType::isNullptr(param.value))
        {
            auto v = toJZNullptr(param.value);
            return "nullptr(" + JZNodeType::typeToName(v->type) +")";
        }
        else
            return "$" + JZNodeType::toString(param.value);
    }
    else if(param.type == JZNodeIRParam::Reference)
        return param.ref();
    else if(param.type == JZNodeIRParam::This)
        return "this";
    else
        return JZNodeCompiler::paramName(param.id());
}

QString JZNodeProgram::irToString(JZNodeIR *op)
{    
    QString line = QString::asprintf("%04d ", op->pc);
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
        if (ir_alloc->allocType == JZNodeIRAlloc::Heap || ir_alloc->allocType == JZNodeIRAlloc::Stack)
            line += alloc + " " + ir_alloc->name + " = " + toString(ir_alloc->value);
        else
            line += alloc + " " + JZNodeCompiler::paramName(ir_alloc->id) + " = " + toString(ir_alloc->value);
        break;
    }
    case OP_set:
    {
        JZNodeIRSet *ir_set = (JZNodeIRSet*)op;
        line += "SET " + toString(ir_set->dst) + " = " + toString(ir_set->src);
        break;
    }
    case OP_call:
    {
        JZNodeIRCall *ir_call = (JZNodeIRCall *)op;
        line += "CALL " + ir_call->function;
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
        if (op->type == OP_jmp)
            line += "JMP " + QString::number(ir_jmp->jmpPc);
        else if (op->type == OP_je)
            line += "JE " + QString::number(ir_jmp->jmpPc);
        else
            line += "JNE " + QString::number(ir_jmp->jmpPc);
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

    if (!op->memo.isEmpty())
    {
        line = line.leftJustified(12);
        line += " //" + op->memo;
    }
    return line;
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
            content += irToString(opList[i].data()) + "\n";
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
