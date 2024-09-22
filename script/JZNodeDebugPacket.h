#ifndef JZNODE_DEBUG_PACKET_H_
#define JZNODE_DEBUG_PACKET_H_

#include "JZNetPack.h"
#include "JZProject.h"
#include "JZNodeProgram.h"
#include "JZNodeEngine.h"

const int NetPack_debugPacket = NetPack_user;

enum{
    Cmd_none,
    Cmd_init,
    Cmd_addBreakPoint,
    Cmd_removeBreakPoint,       
    Cmd_clearBreakPoint,    
    Cmd_detach,
    Cmd_pause,
    Cmd_resume,    
    Cmd_stop,
    Cmd_stepIn,
    Cmd_stepOver,
    Cmd_stepOut,     
    Cmd_runtimeStatus,
    Cmd_runtimeInfo,
    Cmd_runtimeError,
    Cmd_getVariable,
    Cmd_setVariable,
    Cmd_nodePropChanged,
    Cmd_log,
};


class JZNodeDebugPacket : public JZNetPack 
{
public:
    JZNodeDebugPacket();
    ~JZNodeDebugPacket();

    virtual int type() const;
	virtual void saveToStream(QDataStream &s) const;
	virtual void loadFromStream(QDataStream &s);    

    int cmd;
    QVariantList params;
};

//JZNodeDebugParamValue
class JZNodeDebugParamValue
{
public:
    JZNodeDebugParamValue();

    int type;
    QString value;
    QByteArray binValue;    
    QMap<QString, JZNodeDebugParamValue> params;
};
QDataStream &operator<<(QDataStream &s, const JZNodeDebugParamValue &param);
QDataStream &operator>>(QDataStream &s, JZNodeDebugParamValue &param);

//JZNodeGetDebugParam
class JZNodeGetDebugParam
{
public:
    JZNodeGetDebugParam();

    int stack;
    QList<JZNodeIRParam> coors;
};
QDataStream &operator<<(QDataStream &s, const JZNodeGetDebugParam &param);
QDataStream &operator>>(QDataStream &s, JZNodeGetDebugParam &param);

//JZNodeGetDebugParamResp
class JZNodeGetDebugParamResp
{
public:
    JZNodeGetDebugParamResp();

    int stack;
    QList<JZNodeIRParam> coors;
    QList<JZNodeDebugParamValue> values;
};
QDataStream &operator<<(QDataStream &s, const JZNodeGetDebugParamResp &param);
QDataStream &operator>>(QDataStream &s, JZNodeGetDebugParamResp &param);

//JZNodeSetDebugParam
class JZNodeSetDebugParam
{
public:
    JZNodeSetDebugParam();

    int stack;
    JZNodeIRParam coor;
    QString value;
};
QDataStream &operator<<(QDataStream &s, const JZNodeSetDebugParam &param);
QDataStream &operator>>(QDataStream &s, JZNodeSetDebugParam &param);

//JZNodeSetDebugParamResp
class JZNodeSetDebugParamResp
{
public:
    JZNodeSetDebugParamResp();

    int stack;
    JZNodeIRParam coor;
    JZNodeDebugParamValue value;
};
QDataStream &operator<<(QDataStream &s, const JZNodeSetDebugParamResp &param);
QDataStream &operator>>(QDataStream &s, JZNodeSetDebugParamResp &param);

//JZNodeScriptInfo
class JZNodeScriptInfo
{
public:
    JZNodeScriptInfo();

    const JZFunctionDefine *function(QString name) const;

    QString file;
    QString className;
    QMap<int, NodeInfo> nodeInfo;
    QList<JZFunctionDefine> functionList;
    QMap<QString, JZFunction> runtimeInfo;
};
QDataStream &operator<<(QDataStream &s, const JZNodeScriptInfo &param);
QDataStream &operator>>(QDataStream &s, JZNodeScriptInfo &param);

//JZNodeProgramInfo
class JZNodeProgramInfo
{
public:    
    QString appPath;
};
QDataStream &operator<<(QDataStream &s, const JZNodeProgramInfo &param);
QDataStream &operator>>(QDataStream &s, JZNodeProgramInfo &param);

//JZNodeDebugInfo
class JZNodeDebugInfo
{
public:    
    JZNodeDebugInfo();

    QList<BreakPoint> breakPoints;
};
QDataStream &operator<<(QDataStream &s, const JZNodeDebugInfo &param);
QDataStream &operator>>(QDataStream &s, JZNodeDebugInfo &param);

//JZNodeRuntimeWatch
class JZNodeRuntimeWatch
{
public:
    JZNodeRuntimeWatch();

    JZNodeRuntimeInfo runtimInfo;
    QMap<int, JZNodeDebugParamValue> values;
};
QDataStream &operator<<(QDataStream &s, const JZNodeRuntimeWatch &param);
QDataStream &operator>>(QDataStream &s, JZNodeRuntimeWatch &param);

#endif
