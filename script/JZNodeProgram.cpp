#include <QFile>
#include <QDebug>
#include "JZNodeProgram.h"
#include "JZNodeCompiler.h"
#include "JZNodeFunctionManager.h"
#include "JZContainer.h"
#include "JZModule.h"
#include "JZRegExpHelp.h"

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
    id = -1;
    type = Node_none;
    isFlow = false;
}

NodeParamInfo *NodeInfo::param(int id)
{
    for (int i = 0; i < paramIn.size(); i++)
    {
        if (paramIn[i].id == id)
            return &paramIn[i];
    }
    for (int i = 0; i < paramOut.size(); i++)
    {
        if (paramOut[i].id == id)
            return &paramOut[i];
    }
    return nullptr;
}

QDataStream &operator<<(QDataStream &s, const NodeInfo &param)
{
    s << param.name;
    s << param.id;
    s << param.type;
    s << param.isFlow;
    s << param.paramIn;    
    s << param.paramOut;
    s << param.pcRanges;
    return s;
}

QDataStream &operator>>(QDataStream &s, NodeInfo &param)
{
    s >> param.name;
    s >> param.id;
    s >> param.type;
    s >> param.isFlow;
    s >> param.paramIn;    
    s >> param.paramOut;    
    s >> param.pcRanges;
    return s;
}

//JZFunctionDebugInfo
const JZParamDefine *JZFunctionDebugInfo::localParam(QString name) const
{
    for(int i = 0; i < localVariables.size(); i++)
    {
        if(localVariables[i].name == name)
            return &localVariables[i];
    }
    return nullptr;
}

const JZParam *JZFunctionDebugInfo::nodeParam(int id) const
{
    auto gemo = JZNodeCompiler::paramGemo(id);
    auto it = nodeInfo.find(gemo.nodeId);
    if(it == nodeInfo.end())
        return nullptr;

    auto &info = it.value();
    for(int i = 0; i < info.paramIn.size(); i++)
    {
        if(info.paramIn[i].id == gemo.pinId)
            return &info.paramIn[i].define;
    }
    for(int i = 0; i < info.paramOut.size(); i++)
    {
        if(info.paramOut[i].id == gemo.pinId)
            return &info.paramOut[i].define;
    }

    return nullptr;
}   

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
    itemPath.clear();
    className.clear();    
    statmentList.clear();
    functionList.clear();    
    functionDebugList.clear();
}

const JZFunction *JZNodeScript::function(QString name) const
{
    for(int i = 0; i < functionList.size(); i++)
    {
        if(functionList[i].fullName() == name)
            return &functionList[i];
    }        
    return nullptr;
}

const JZFunctionDebugInfo *JZNodeScript::functionDebug(QString name) const
{
    for (int i = 0; i < functionList.size(); i++)
    {
        if (functionList[i].fullName() == name)
            return &functionDebugList[i];
    }
    return nullptr;
}

void JZNodeScript::copyTo(JZNodeScript *other) const
{
    QByteArray buffer;
    QDataStream out(&buffer, QIODevice::WriteOnly);
    this->saveToStream(out);

    QDataStream in(&buffer, QIODevice::ReadOnly);    
    other->loadFromStream(in);    
}

void JZNodeScript::saveToStream(QDataStream &s) const
{
    s << itemPath;    
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
    s >> itemPath;
    s >> className;
    int stmt_size = 0;
    s >> stmt_size;
    for(int i = 0; i < stmt_size; i++)
    {
        JZNodeIRType type;
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
    QString className,memberName;
    JZRegExpHelp::splitDefine(name,memberName,memberName);

    if (!className.isEmpty())
    {        
        auto obj = object(className);
        if (!obj)
            return nullptr;

        return obj->function(memberName);
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

//JZNodeProgram
JZNodeProgram::JZNodeProgram()
{    
}

JZNodeProgram::~JZNodeProgram()
{
}

bool JZNodeProgram::isNull() const
{
    return m_filePath.isEmpty();
}

void JZNodeProgram::clear()
{
    m_scripts.clear();    
    m_variables.clear();    
    m_typeMeta.clear();        
}


QString JZNodeProgram::applicationFilePath() const
{
    return m_filePath;
}

void JZNodeProgram::saveToStream(QDataStream &s) const
{
    int script_size = m_scripts.size();
    s << script_size;
    auto it = m_scripts.begin();
    while (it != m_scripts.end())
    {
        s << it.key();
        it.value()->saveToStream(s);
        it++;
    }
    s << m_variables;
    s << m_typeMeta;
}

void JZNodeProgram::loadFromStream(QDataStream &s)
{
    int script_size = 0;
    s >> script_size;
    for (int i = 0; i < script_size; i++)
    {
        QString path;
        s >> path;
        JZNodeScript *script = new JZNodeScript();
        script->loadFromStream(s);
        m_scripts[path] = JZNodeScriptPtr(script);
    }
    s >> m_variables;
    s >> m_typeMeta;
}

bool JZNodeProgram::load(QString filepath,QString &error)
{   
    QFile file(filepath);
    if (!file.open(QFile::ReadOnly))
    {
        error = "open file failed";
        return false;
    }

    QByteArray magic;
    QDataStream s(&file);    
    s >> magic;
    if(magic != NodeIRMagic())
    {
        error = "version not support";
        return false;
    }
        
    loadFromStream(s);

    m_filePath = filepath;
    return true;
}
    
bool JZNodeProgram::save(QString filepath)
{
    QFile file(filepath);
    if(!file.open(QFile::WriteOnly | QFile::Truncate))
        return false;

    QDataStream s(&file);    
    s << NodeIRMagic();
    saveToStream(s);

    m_filePath = filepath;
    return true;
}

void JZNodeProgram::copyTo(JZNodeProgram *other) const
{
    QByteArray buffer;
    QDataStream out(&buffer,QIODevice::WriteOnly);
    this->saveToStream(out);

    QDataStream in(&buffer, QIODevice::ReadOnly);
    other->loadFromStream(in);
}

const JZFunction* JZNodeProgram::function(QString name) const
{
    auto it = m_scripts.begin();
    while (it != m_scripts.end())
    {
        JZNodeScript* s = it->data();
        const JZFunction *func = s->function(name);
        if (func)
            return func;

        it++;
    }
    return nullptr;
}

void JZNodeProgram::addScript(QString path, JZNodeScriptPtr script)
{
    m_scripts[path] = script;
}

const JZNodeScript *JZNodeProgram::script(QString path) const
{
    return m_scripts.value(path, JZNodeScriptPtr()).data();
}

const JZFunctionDebugInfo *JZNodeProgram::debugInfo(QString name) const
{
    auto it = m_scripts.begin();
    while(it != m_scripts.end())
    {
        JZNodeScript *s = it->data();
        auto debug = s->functionDebug(name);
        if (debug)
            return debug;

        it++;
    }
    return nullptr;
}    

QList<JZNodeScript*> JZNodeProgram::scriptList() const
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

const JZNodeTypeMeta &JZNodeProgram::typeMeta() const
{
    return m_typeMeta;
}

void JZNodeProgram::initEnv(JZScriptEnvironment *env) const
{
    env->registType(this->typeMeta());
    auto script_list = this->scriptList();
    for (int i = 0; i < script_list.size(); i++)
    {
        auto& func_list = script_list[i]->functionList;
        for (int func_idx = 0; func_idx < func_list.size(); func_idx++)
            env->functionManager()->registFunctionImpl(func_list[func_idx]);
    }
}