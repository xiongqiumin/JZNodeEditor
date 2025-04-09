#ifndef JZNODE_DEBUG_PACKET_H_
#define JZNODE_DEBUG_PACKET_H_

#include "3rd/JZCommon/jzNet/JZNetPack.h"
#include "JZProject.h"
#include "JZNodeProgram.h"
#include "JZNodeEngine.h"

const int NetPack_debugPacket = NetPack_user;

enum
{
    Cmd_none,
    Cmd_response,
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
    Cmd_addWatch,
    Cmd_removeWatch,
    Cmd_clearWatch,
    Cmd_watchChanged,
    Cmd_log,
};

template<class T>
QByteArray netDataPack(const T &data)
{
    QByteArray buffer;
    QDataStream s(&buffer, QIODevice::WriteOnly);
    s << data;
    return buffer;
}

template<class T>
T netDataUnPack(const QByteArray &buffer)
{    
    QDataStream s(buffer);
    T data;
    s >> data;
    return data;
}

class JZNodeDebugPacket : public JZNetPack 
{
public:
    JZNodeDebugPacket();
    ~JZNodeDebugPacket();

    virtual int type() const;
	virtual void saveToStream(QDataStream &s) const;
	virtual void loadFromStream(QDataStream &s);    

    int cmd;
    QByteArray buffer;
};

//DebugNodeGemo
class DebugNodeGemo
{
public:
    QString filePath;
    int nodeId;
    int pinId;
    int statck;
};
QDataStream &operator<<(QDataStream &s, const DebugNodeGemo &param);
QDataStream &operator>>(QDataStream &s, DebugNodeGemo &param);

//JZNodeDebugParamValue
class JZNodeDebugParamValue
{
public:
    JZNodeDebugParamValue();

    int type;           //数据类型
    QString value;      //值
    QMap<QString, JZNodeDebugParamValue> params;  //子项的值
};
QDataStream &operator<<(QDataStream &s, const JZNodeDebugParamValue &param);
QDataStream &operator>>(QDataStream &s, JZNodeDebugParamValue &param);

//JZNodeGetDebugParam
class JZNodeGetDebugParam
{
public:
    JZNodeGetDebugParam();

    QString filePath;
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

    JZNodeGetDebugParam req;
    QList<JZNodeDebugParamValue> values;
};
QDataStream &operator<<(QDataStream &s, const JZNodeGetDebugParamResp &param);
QDataStream &operator>>(QDataStream &s, JZNodeGetDebugParamResp &param);

//JZNodeGetDebugParamBin
class JZNodeGetDebugParamBin
{
public:
    JZNodeGetDebugParamBin();

    QString filePath;
    int stack;
    QList<JZNodeIRParam> coors;
};
QDataStream &operator<<(QDataStream &s, const JZNodeGetDebugParamBin &param);
QDataStream &operator>>(QDataStream &s, JZNodeGetDebugParamBin &param);

//JZNodeGetDebugParamBinResp
class JZNodeGetDebugParamBinResp
{
public:
    JZNodeGetDebugParamBinResp();

    QString filePath;
    int stack;
    QList<JZNodeIRParam> coors;
};
QDataStream &operator<<(QDataStream &s, const JZNodeGetDebugParam &param);
QDataStream &operator>>(QDataStream &s, JZNodeGetDebugParam &param);

//JZNodeSetDebugParam
class JZNodeSetDebugParam
{
public:
    JZNodeSetDebugParam();

    QString filePath;
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

    JZNodeSetDebugParam req;
    bool ret;
};
QDataStream &operator<<(QDataStream &s, const JZNodeSetDebugParamResp &param);
QDataStream &operator>>(QDataStream &s, JZNodeSetDebugParamResp &param);

//JZNodeSetDebugParamBin
class JZNodeSetDebugParamBin
{
public:
    JZNodeSetDebugParamBin();

    QString filePath;
    int stack;
    JZNodeIRParam coor;
    QByteArray value;
};
QDataStream &operator<<(QDataStream &s, const JZNodeSetDebugParamBin &param);
QDataStream &operator>>(QDataStream &s, JZNodeSetDebugParamBin &param);

//JZNodeSetDebugParamBinResp
class JZNodeSetDebugParamBinResp
{
public:
    JZNodeSetDebugParamBinResp();

    JZNodeSetDebugParamBin req;
    bool ret;
};
QDataStream &operator<<(QDataStream &s, const JZNodeSetDebugParamBinResp &param);
QDataStream &operator>>(QDataStream &s, JZNodeSetDebugParamBinResp &param);

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
QDataStream &operator<<(QDataStream &s, const JZNodeDebugInfo& param);
QDataStream &operator>>(QDataStream &s, JZNodeDebugInfo& param);

//JZNodeRuntimeWatch
class JZNodeRuntimeWatch
{
public:
    JZNodeRuntimeWatch();

    QList<DebugNodeGemo> m_watchs;
};
QDataStream &operator<<(QDataStream &s, const JZNodeRuntimeWatch &param);
QDataStream &operator>>(QDataStream &s, JZNodeRuntimeWatch &param);

//JZNodeRuntimeWatchResult
class JZNodeRuntimeWatchResult
{
public:
    JZNodeRuntimeWatchResult();

    JZNodeRuntimeInfo runtimInfo;
    QMap<int, JZNodeDebugParamValue> values;
};
QDataStream &operator<<(QDataStream &s, const JZNodeRuntimeWatchResult &param);
QDataStream &operator>>(QDataStream &s, JZNodeRuntimeWatchResult &param);

#endif
