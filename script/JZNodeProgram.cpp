#include <QFile>
#include <QDebug>
#include "JZNodeProgram.h"
#include "JZNodeCompiler.h"
#include "JZNodeFunctionManager.h"
#include "JZContainer.h"
#include "JZModule.h"
#include "JZNodeDefine.h"

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

//JZFunctionDebugInfo
QDataStream &operator<<(QDataStream &s, const JZFunctionDebugInfo &param)
{
    s << param.localVariables;
    s << param.nodeInfo;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZFunctionDebugInfo &param)
{
    s >> param.localVariables;
    s >> param.nodeInfo;
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
    statmentList.clear();
    functionList.clear();    
    functionDebugList.clear();
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

JZFunctionDebugInfo *JZNodeScript::functionDebug(QString name)
{
    for (int i = 0; i < functionList.size(); i++)
    {
        if (functionList[i].fullName() == name)
            return &functionDebugList[i];
    }
    return nullptr;
}

JZNodeScript *JZNodeScript::clone()
{
    QByteArray buffer;
    QDataStream out(&buffer, QIODevice::WriteOnly);
    this->saveToStream(out);

    QDataStream in(&buffer, QIODevice::ReadOnly);
    JZNodeScript *script = new JZNodeScript();
    script->loadFromStream(in);
    return script;
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
    s << functionDebugList;
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
    s >> functionDebugList;
}

//ScriptDepend
ScriptDepend::FunctionHook::FunctionHook()
{
    enable = false;
    nodeId = -1;
    pc = -1;
}

void ScriptDepend::clear()
{
    function = JZFunctionDefine();
    member.clear();
    global.clear();
    hook.clear();
}

JZParamDefine *ScriptDepend::getGlobal(QString name)
{        
    for (int i = 0; i < global.size(); i++)
    {
        if (global[i].name == name)
            return &global[i];
    }
    return nullptr;
}

JZParamDefine *ScriptDepend::getMember(QString name)
{
    if (name.startsWith("this."))
        name = name.mid(5);

    for (int i = 0; i < member.size(); i++)
    {
        if (member[i].name == name)
            return &member[i];
    }
    return nullptr;
}

ScriptDepend::FunctionHook *ScriptDepend::getHook(int node_id)
{
    for(int i = 0; i < hook.size(); i++)
    {
        if(hook[i].nodeId == node_id)
            return &hook[i];
    }

    return nullptr;
}

//JZNodeTypeMeta
void JZNodeTypeMeta::clear()
{
    functionList.clear();
    objectList.clear();       
    cobjectList.clear();
}

const JZFunctionDefine *JZNodeTypeMeta::function(QString name) const
{
    if (name.contains("."))
    {
        int index = name.indexOf(".");
        QString class_name = name.left(index);
        QString func_name = name.mid(index + 1);
        auto obj = object(class_name);
        if (!obj)
            return nullptr;

        return obj->function(func_name);
    }
    else
    {
        for (int i = 0; i < functionList.size(); i++)
        {
            if (functionList[i].name == name)
                return &functionList[i];
        }
        return nullptr;
    }
}

const JZNodeObjectDefine *JZNodeTypeMeta::object(QString name) const
{
    for(int i = 0; i < objectList.size(); i++)
    {
        if(objectList[i].className == name)
            return &objectList[i];
    }
    return nullptr;
}

QDataStream &operator<<(QDataStream &s, const JZNodeTypeMeta &param)
{
    s << param.functionList;
    s << param.objectList;
    s << param.cobjectList;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeTypeMeta &param)
{
    s >> param.functionList;
    s >> param.objectList;
    s >> param.cobjectList;
    return s;
}

void JZNodeRegistType(const JZNodeTypeMeta &type_info)
{
    JZNodeFunctionManager::instance()->clearUserReigst();
    JZNodeObjectManager::instance()->clearUserReigst();
    
    auto &module_list = type_info.moduleList;
    auto &define_list = type_info.objectList;
    auto &cobj_list = type_info.cobjectList;
    auto &function_list = type_info.functionList;
    QList<int> cobj_id;

    for(int i = 0; i < module_list.size(); i++)
        JZModuleManager::instance()->loadModule(module_list[i]);

    //delcare
    for(int i = 0; i < define_list.size(); i++)
        JZNodeObjectManager::instance()->delcare(define_list[i].className,define_list[i].id);
    for (int i = 0; i < cobj_list.size(); i++)
    {
        int id = JZNodeObjectManager::instance()->delcare(cobj_list[i].className, cobj_list[i].id);
        cobj_id << id;
    }

    //regist
    for(int i = 0; i < define_list.size(); i++)
        JZNodeObjectManager::instance()->regist(define_list[i]);
    for(int i = 0; i < cobj_list.size(); i++)
        registContainer(cobj_list[i].className, cobj_id[i]);    

    for (int i = 0; i < function_list.size(); i++)
        JZNodeFunctionManager::instance()->registFunction(function_list[i]);
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
    m_typeMeta.clear();        
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
    s >> m_typeMeta;
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
    s << m_typeMeta;
    s << m_binds;
    
    m_filePath = filepath;
    return true;
}

JZNodeScript *JZNodeProgram::script(QString path)
{
    return m_scripts.value(path, JZNodeScriptPtr()).data();
}

const JZFunctionDefine *JZNodeProgram::function(QString name)
{
    auto func = m_typeMeta.function(name);
    if(func)
        return func;

    return JZNodeFunctionManager::instance()->function(name);
}

void JZNodeProgram::registType()
{
    JZNodeRegistType(m_typeMeta);

    auto it = m_scripts.begin();
    while (it != m_scripts.end())
    {
        auto &sub_list = it.value()->functionList;
        for (int i = 0; i < sub_list.size(); i++)
            JZNodeFunctionManager::instance()->registFunctionImpl(sub_list[i]);    

        it++;
    }
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

const JZNodeTypeMeta &JZNodeProgram::typeMeta() const
{
    return m_typeMeta;
}

QString JZNodeProgram::toString(JZNodeIRParam param)
{
    if (param.type == JZNodeIRParam::Literal)
    {
        auto var_type = JZNodeType::variantType(param.value);
        if (var_type == Type_string)
            return "\"" + param.value.toString() + "\"";
        else if (var_type == Type_class)
        {
            return JZNodeType::typeToName(var_type) + "()";
        }
        else 
            return JZNodeType::debugString(param.value) + ":" + JZNodeType::typeToName(var_type);
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
        alloc += " " + JZNodeType::typeToName(ir_alloc->dataType);
        if (ir_alloc->allocType == JZNodeIRAlloc::Heap || ir_alloc->allocType == JZNodeIRAlloc::Stack)
            line += alloc + " " + ir_alloc->name;
        else
            line += alloc + " " + JZNodeCompiler::paramName(ir_alloc->id);
        break;
    }
    case OP_clearReg:
        line += "CLEAR REG";
        break;
    case OP_set:
    {
        JZNodeIRSet *ir_set = (JZNodeIRSet*)op;
        line += "SET " + toString(ir_set->dst) + " = " + toString(ir_set->src);
        break;
    }
    case OP_clone:
    {
        JZNodeIRClone *ir_set = (JZNodeIRClone*)op;
        line += "CLONE " + toString(ir_set->dst) + " = " + toString(ir_set->src);
        break;
    }
    case OP_watch:
    {
        JZNodeIRWatch *ir_watch = (JZNodeIRWatch*)op;
        line += "WATCH " + toString(ir_watch->traget) + " = " + toString(ir_watch->source);
        break;
    }
    case OP_convert:
    {
        JZNodeIRConvert *ir_cvt = (JZNodeIRConvert*)op;
        line += "CONVERT " + JZNodeType::typeToName(ir_cvt->dstType) + " " + 
            toString(ir_cvt->dst) + " = " + toString(ir_cvt->src);
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
        for(int func_idx = 0; func_idx < script->functionList.size(); func_idx++)
        {
            auto &func = script->functionList[func_idx];
            
            QString line = "function " + func.fullName() + ":\n";
            for(int i = func.addr; i < func.addrEnd; i++)
            {
                //deal op            
                content += irToString(opList[i].data()) + "\n";
            }
            content += "\n";
        }

        it++;
    }    
    return content;
}
