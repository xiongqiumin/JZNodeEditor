#include "JZNodeDebugPacket.h"
#include <QDataStream>
#include "JZNodeFunctionManager.h"

//JZNodeDebugPacket
JZNodeDebugPacket::JZNodeDebugPacket()
{
    cmd = Cmd_none;
}

JZNodeDebugPacket::~JZNodeDebugPacket()
{
}

int JZNodeDebugPacket::type() const
{
    return NetPack_debugPacket;
}

void JZNodeDebugPacket::saveToStream(QDataStream &s) const
{
    s << cmd << params;        
}

void JZNodeDebugPacket::loadFromStream(QDataStream &s)
{
    s >> cmd >> params;
}

//JZNodeParamCoor
JZNodeParamCoor::JZNodeParamCoor()
{
    type = 0;
    id = -1;
    stack = -1;
}

QDataStream &operator<<(QDataStream &s, const JZNodeParamCoor &param)
{
    s << param.type;
    s << param.stack;
    s << param.name;
    s << param.id;
    return s;
}

QDataStream &operator >> (QDataStream &s, JZNodeParamCoor &param)
{
    s >> param.type;
    s >> param.stack;
    s >> param.name;
    s >> param.id;
    return s;
}

//JZNodeDebugParamValue
QDataStream &operator<<(QDataStream &s, const JZNodeDebugParamValue &param)
{
    s << param.type;
    s << param.value;
    s << param.params;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeDebugParamValue &param)
{
    s >> param.type;
    s >> param.value;
    s >> param.params;
    return s;
}

//JZNodeDebugParamInfo
QDataStream &operator<<(QDataStream &s, const JZNodeDebugParamInfo &param)
{
    s << param.coors;
    s << param.values;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeDebugParamInfo &param)
{
    s >> param.coors;
    s >> param.values;
    return s;
}

//JZNodeScriptInfo
JZNodeScriptInfo::JZNodeScriptInfo()
{
}

const FunctionDefine *JZNodeScriptInfo::function(QString name) const
{
    for (int i = 0; i < functionList.size(); i++)
    {
        if (functionList[i].fullName() == name)
            return &functionList[i];
    }
    return nullptr;
}

QDataStream &operator<<(QDataStream &s, const JZNodeScriptInfo &param)
{
    s << param.file;
    s << param.className;
    s << param.nodeInfo;
    s << param.functionList;
    s << param.runtimeInfo;
    return s;
}

QDataStream &operator >> (QDataStream &s, JZNodeScriptInfo &param)
{    
    s >> param.file;
    s >> param.className;
    s >> param.nodeInfo;
    s >> param.functionList;
    s >> param.runtimeInfo;
    return s;
}

//JZNodeProgramInfo
JZNodeProgramInfo::JZNodeProgramInfo()
{

}

const JZNodeObjectDefine *JZNodeProgramInfo::meta(QString name) const
{
    for (int i = 0; i < objectDefines.size(); i++)
    {
        if (objectDefines[i].className == name)
            return &objectDefines[i];
    }

    return JZNodeObjectManager::instance()->meta(name);
}

const FunctionDefine *JZNodeProgramInfo::function(QString name) const
{
    auto it = scripts.begin();
    while(it != scripts.end())
    {
        if (auto func = it->function(name))
            return func;
        it++;
    }

    for (int i = 0; i < globalFunstions.size(); i++)
    {
        if (globalFunstions[i].fullName() == name)
            return &globalFunstions[i];
    }
    return JZNodeFunctionManager::instance()->function(name);
}

QDataStream &operator<<(QDataStream &s, const JZNodeProgramInfo &param)
{ 
    s << param.scripts;
    s << param.globalFunstions;
    s << param.globalVariables;
    s << param.objectDefines;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeProgramInfo &param)
{    
    s >> param.scripts;
    s >> param.globalFunstions;
    s >> param.globalVariables;
    s >> param.objectDefines;
    return s;
}


//DebugInfo
JZNodeDebugInfo::JZNodeDebugInfo()
{
}

QDataStream &operator<<(QDataStream &s, const JZNodeDebugInfo &param)
{
    s << param.breakPoints;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeDebugInfo &param)
{
    s >> param.breakPoints;
    return s;
}
