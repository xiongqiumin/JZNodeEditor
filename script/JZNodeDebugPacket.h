#ifndef JZNODE_DEBUG_PACKET_H_
#define JZNODE_DEBUG_PACKET_H_

#include "JZNetPack.h"
#include "JZProject.h"
#include "JZNodeProgram.h"

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

template<class T>
QByteArray netDataPack(const T &param)
{
    QByteArray buffer;
    QDataStream s(&buffer,QIODevice::WriteOnly);
    s << param;
    return buffer;
}

template<class T>
T netDataUnPack(const QByteArray &buffer)
{
    T param;
    QDataStream s(buffer);
    s >> param;
    return param;
}

//JZNodeParamCoor
class JZNodeParamCoor
{
public:
    enum {
        Local,
        This,
        Global,
        Node,
        Reg,
    };

    JZNodeParamCoor();

    int type;
    
    int stack;
    QString name;
    int id;        //节点id
};
QDataStream &operator<<(QDataStream &s, const JZNodeParamCoor &param);
QDataStream &operator>>(QDataStream &s, JZNodeParamCoor &param);

//JZNodeDebugParamValue
class JZNodeDebugParamValue
{
public:
    int type;
    QVariant value;
    QMap<QString, JZNodeDebugParamValue> params;
};
QDataStream &operator<<(QDataStream &s, const JZNodeDebugParamValue &param);
QDataStream &operator>>(QDataStream &s, JZNodeDebugParamValue &param);

//JZNodeDebugParamInfo
class JZNodeDebugParamInfo
{
public:
    QList<JZNodeParamCoor> coors;
    QList<JZNodeDebugParamValue> values;
};
QDataStream &operator<<(QDataStream &s, const JZNodeDebugParamInfo &param);
QDataStream &operator>>(QDataStream &s, JZNodeDebugParamInfo &param);

//JZNodeScriptInfo
class JZNodeScriptInfo
{
public:
    JZNodeScriptInfo();

    const FunctionDefine *function(QString name) const;

    QString file;
    QString className;
    QMap<int, NodeInfo> nodeInfo;
    QList<FunctionDefine> functionList;
    QMap<QString, JZFunction> runtimeInfo;
};
QDataStream &operator<<(QDataStream &s, const JZNodeScriptInfo &param);
QDataStream &operator>>(QDataStream &s, JZNodeScriptInfo &param);

//JZNodeProgramInfo
class JZNodeProgramInfo
{
public:
    JZNodeProgramInfo();

    const JZNodeObjectDefine *meta(QString name) const;
    const FunctionDefine *function(QString name) const;

    QMap<QString, JZNodeScriptInfo> scripts;
    QList<FunctionDefine> globalFunstions;
    QMap<QString, JZParamDefine> globalVariables;
    QList<JZNodeObjectDefine> objectDefines;    
};
QDataStream &operator<<(QDataStream &s, const JZNodeProgramInfo &param);
QDataStream &operator>>(QDataStream &s, JZNodeProgramInfo &param);

//JZNodeDebugInfo
class JZNodeDebugInfo
{
public:
    JZNodeDebugInfo();

    QMap<QString,QVector<int>> breakPoints;
};
QDataStream &operator<<(QDataStream &s, const JZNodeDebugInfo &param);
QDataStream &operator>>(QDataStream &s, JZNodeDebugInfo &param);


#endif
