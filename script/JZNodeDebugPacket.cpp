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
    type = Name;
    id = -1;    
}

bool JZNodeParamCoor::isNodeId() const
{
    return (type == Id && id < Stack_User);
}

QDataStream &operator<<(QDataStream &s, const JZNodeParamCoor &param)
{
    s << param.type;    
    s << param.name;
    s << param.id;
    return s;
}

QDataStream &operator >> (QDataStream &s, JZNodeParamCoor &param)
{
    s >> param.type;    
    s >> param.name;
    s >> param.id;
    return s;
}

//JZNodeDebugParamValue
JZNodeDebugParamValue::JZNodeDebugParamValue()
{
    type = Type_none;
    ptrValue = nullptr;
}

QDataStream &operator<<(QDataStream &s, const JZNodeDebugParamValue &param)
{
    s << param.type;
    s << param.value;
    s << param.binValue;
    s << param.params;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeDebugParamValue &param)
{
    s >> param.type;
    s >> param.value;
    s >> param.binValue;
    s >> param.params;
    return s;
}

//JZNodeGetDebugParam
JZNodeGetDebugParam::JZNodeGetDebugParam()
{
    stack = -1;
}

QDataStream &operator<<(QDataStream &s, const JZNodeGetDebugParam &param)
{
    s << param.stack;
    s << param.coors;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeGetDebugParam &param)
{
    s >> param.stack;
    s >> param.coors;
    return s;
}

//JZNodeGetDebugParam
JZNodeGetDebugParamResp::JZNodeGetDebugParamResp()
{
    stack = -1;
}

QDataStream &operator<<(QDataStream &s, const JZNodeGetDebugParamResp &param)
{
    s << param.stack;
    s << param.coors;
    s << param.values;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeGetDebugParamResp &param)
{
    s >> param.stack;
    s >> param.coors;
    s >> param.values;
    return s;
}

//JZNodeSetDebugParam
JZNodeSetDebugParam::JZNodeSetDebugParam()
{
    stack = -1;
}

QDataStream &operator<<(QDataStream &s, const JZNodeSetDebugParam &param)
{
    s << param.stack;
    s << param.coor;
    s << param.value;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeSetDebugParam &param)
{
    s >> param.stack;
    s >> param.coor;
    s >> param.value;
    return s;
}

//JZNodeSetDebugParamResp
JZNodeSetDebugParamResp::JZNodeSetDebugParamResp()
{
    stack = -1;
}

QDataStream &operator<<(QDataStream &s, const JZNodeSetDebugParamResp &param)
{
    s << param.stack;
    s << param.coor;
    s << param.value;
    return s;
}
QDataStream &operator>>(QDataStream &s, JZNodeSetDebugParamResp &param)
{
    s >> param.stack;
    s >> param.coor;
    s >> param.value;
    return s;
}

//JZNodeScriptInfo
JZNodeScriptInfo::JZNodeScriptInfo()
{
}

const JZFunctionDefine *JZNodeScriptInfo::function(QString name) const
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
QDataStream &operator<<(QDataStream &s, const JZNodeProgramInfo &param)
{
    s << param.appPath;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeProgramInfo &param)
{
    s >> param.appPath;
    return s;
}

//JZNodeDebugInfo
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

//JZNodeValueChanged
JZNodeRuntimeWatch::JZNodeRuntimeWatch()
{        
}

QDataStream &operator<<(QDataStream &s, const JZNodeRuntimeWatch &param)
{
    s << param.runtimInfo << param.values;
    return s;
}

QDataStream &operator >> (QDataStream &s, JZNodeRuntimeWatch &param)
{
    s >> param.runtimInfo >> param.values;
    return s;
}