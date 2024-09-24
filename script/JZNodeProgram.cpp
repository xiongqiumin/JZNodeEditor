#include <QFile>
#include <QDebug>
#include "JZNodeProgram.h"
#include "JZNodeCompiler.h"
#include "JZNodeFunctionManager.h"
#include "JZContainer.h"
#include "JZModule.h"

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
    s << param.moduleList;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeTypeMeta &param)
{
    s >> param.functionList;
    s >> param.objectList;
    s >> param.cobjectList;
    s >> param.moduleList;
    return s;
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
    if(magic != NodeIRMagic())
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

    m_filePath = filepath;
    return true;
}

JZNodeScript *JZNodeProgram::script(QString path)
{
    return m_scripts.value(path, JZNodeScriptPtr()).data();
}

const JZFunctionDebugInfo *JZNodeProgram::debugInfo(QString name)
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

const JZNodeTypeMeta &JZNodeProgram::typeMeta() const
{
    return m_typeMeta;
}