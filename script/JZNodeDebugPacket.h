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
        Name,
        Id,
    };

    JZNodeParamCoor();

    int type;        
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
    QString value;
    QMap<QString, JZNodeDebugParamValue> params;
};
QDataStream &operator<<(QDataStream &s, const JZNodeDebugParamValue &param);
QDataStream &operator>>(QDataStream &s, JZNodeDebugParamValue &param);

//JZNodeDebugParamInfo
class JZNodeDebugParamInfo
{
public:
    JZNodeDebugParamInfo();

    int stack;
    QList<JZNodeParamCoor> coors;
    QList<JZNodeDebugParamValue> values;
};
QDataStream &operator<<(QDataStream &s, const JZNodeDebugParamInfo &param);
QDataStream &operator>>(QDataStream &s, JZNodeDebugParamInfo &param);

//JZNodeSetDebugParamInfo
class JZNodeSetDebugParamInfo
{
public:
    JZNodeSetDebugParamInfo();

    int stack;
    JZNodeParamCoor coor;
    QVariant value;
};
QDataStream &operator<<(QDataStream &s, const JZNodeSetDebugParamInfo &param);
QDataStream &operator >> (QDataStream &s, JZNodeSetDebugParamInfo &param);


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
    QString appPath;
};
QDataStream &operator<<(QDataStream &s, const JZNodeProgramInfo &param);
QDataStream &operator>>(QDataStream &s, JZNodeProgramInfo &param);

//JZNodeDebugInfo
class JZNodeDebugInfo
{
public:
    JZNodeDebugInfo();

    QMap<QString,QList<int>> breakPoints;
};
QDataStream &operator<<(QDataStream &s, const JZNodeDebugInfo &param);
QDataStream &operator>>(QDataStream &s, JZNodeDebugInfo &param);

//JZNodeValueChanged
class JZNodeValueChanged
{
public:
    JZNodeValueChanged();

    QString file;
    int id;
    QString value;
};
QDataStream &operator<<(QDataStream &s, const JZNodeValueChanged &param);
QDataStream &operator>>(QDataStream &s, JZNodeValueChanged &param);

#endif
