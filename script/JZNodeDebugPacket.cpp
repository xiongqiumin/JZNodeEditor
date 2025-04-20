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
    s << cmd << buffer;
}

void JZNodeDebugPacket::loadFromStream(QDataStream &s)
{
    s >> cmd >> buffer;
}

//JZNodeDebugParamValue
JZNodeDebugParamValue::JZNodeDebugParamValue()
{
    type = Type_none;    
}

QDataStream &operator<<(QDataStream &s, const JZNodeDebugParamValue &param)
{
    s << param.type;
    s << param.value;
    s << param.subParamNames;
    s << param.subParamValues;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeDebugParamValue &param)
{
    s >> param.type;
    s >> param.value;
    s >> param.subParamNames;
    s >> param.subParamValues;    
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
}

QDataStream &operator<<(QDataStream &s, const JZNodeGetDebugParamResp &param)
{
    s << param.req;
    s << param.values;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeGetDebugParamResp &param)
{
    s >> param.req;
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
}

QDataStream &operator<<(QDataStream &s, const JZNodeSetDebugParamResp &param)
{
    return s;
}
QDataStream &operator>>(QDataStream &s, JZNodeSetDebugParamResp &param)
{
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

QDataStream &operator<<(QDataStream &s, const JZNodeDebugInfo&param)
{
    s << param.breakPoints;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeDebugInfo&param)
{
    s >> param.breakPoints;
    return s;
}

//JZNodeRuntimeWatch
JZNodeRuntimeWatch::JZNodeRuntimeWatch()
{
}

QDataStream &operator<<(QDataStream &s, const JZNodeRuntimeWatch &param)
{
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeRuntimeWatch &param)
{
    return s;
}

//JZNodeRuntimeWatchResult
JZNodeRuntimeWatchResult::JZNodeRuntimeWatchResult()
{        
}

QDataStream &operator<<(QDataStream &s, const JZNodeRuntimeWatchResult &param)
{
    s << param.runtimInfo << param.values;
    return s;
}

QDataStream &operator >> (QDataStream &s, JZNodeRuntimeWatchResult &param)
{
    s >> param.runtimInfo >> param.values;
    return s;
}