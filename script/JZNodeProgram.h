#ifndef JZNODE_PROGRAM_H_
#define JZNODE_PROGRAM_H_

#include <QThread>
#include "JZNode.h"
#include "JZNodeEvent.h"
#include "JZEvent.h"
#include "JZNodeIR.h"
#include "JZNodeFunctionDefine.h"
#include "JZNodeObject.h"

//NodeRange
struct NodeRange
{
    NodeRange();

    int start;
    int debugStart;
    int end;
};
QDataStream &operator<<(QDataStream &s, const NodeRange &param);
QDataStream &operator>>(QDataStream &s, NodeRange &param);

//NodeParamInfo
struct NodeParamInfo
{
    JZParam define;
    int id;
};
QDataStream &operator<<(QDataStream &s, const NodeParamInfo &param);
QDataStream &operator>>(QDataStream &s, NodeParamInfo &param);

//NodeInfo
struct NodeInfo
{        
    NodeInfo();    
    
    QString name;
    int id;
    int type;
    bool isFlow;       
    
    QList<NodeParamInfo> paramIn;
    QList<NodeParamInfo> paramOut;
    QList<NodeRange> pcRanges;
};
QDataStream &operator<<(QDataStream &s, const NodeInfo &param);
QDataStream &operator>>(QDataStream &s, NodeInfo &param);

//JZFunctionDebugInfo
class JZFunctionDebugInfo
{
public:
    JZParamDefine *localParam(QString name);
    JZParam *nodeParam(int id);

    QMap<int, NodeInfo> nodeInfo;
    QList<JZParamDefine> localVariables;
};
QDataStream &operator<<(QDataStream &s, const JZFunctionDebugInfo &param);
QDataStream &operator>>(QDataStream &s, JZFunctionDebugInfo &param);

//JZNodeScript
class JZNodeScript
{    
public:    
    JZNodeScript();
    void clear();    

    JZFunction *function(QString name);
    JZFunctionDebugInfo *functionDebug(QString name);
    
    JZNodeScript *clone();
    void saveToStream(QDataStream &s);
    void loadFromStream(QDataStream &s);

    QString file;
    QString className; 
    QList<JZNodeIRPtr> statmentList;
    QList<JZFunction> functionList;

    QList<JZFunctionDebugInfo> functionDebugList;

protected:
    Q_DISABLE_COPY(JZNodeScript);    
};
typedef QSharedPointer<JZNodeScript> JZNodeScriptPtr;

//Depends
class ScriptDepend
{
public:
    struct FunctionHook
    {
        FunctionHook();
        
        bool enable;
        int nodeId;
        int pc;
        QString function;
        QList<JZParamDefine> params;
    };
    
    void clear();    
    JZParamDefine *getGlobal(QString name);
    JZParamDefine *getMember(QString name);
    FunctionHook *getHook(int node_id);

    JZFunctionDefine function;
    QList<JZParamDefine> member;
    QList<JZParamDefine> global;
    QList<FunctionHook> hook;
};

//JZNodeTypeMeta
class JZNodeTypeMeta
{
public:
    void clear();

    const JZFunctionDefine *function(QString name) const;
    const JZNodeObjectDefine *object(QString name) const;

    QList<JZFunctionDefine> functionList;
    QList<JZNodeObjectDefine> objectList;       
    QList<JZNodeCObjectDelcare> cobjectList;
    QStringList moduleList;
};
QDataStream &operator<<(QDataStream &s, const JZNodeTypeMeta &param);
QDataStream &operator>>(QDataStream &s, JZNodeTypeMeta &param);
void JZNodeRegistType(const JZNodeTypeMeta &type_info);

//JZNodeProgram
class JZNodeBuilder;
class JZNodeProgram 
{
public:
    JZNodeProgram();
    ~JZNodeProgram();

    bool isNull();
    bool load(QString file);
    bool save(QString file);
    void clear();

    const JZNodeTypeMeta &typeMeta() const;   
    void registType();

    QString applicationFilePath();
    
    QList<JZNodeScript*> scriptList();
    JZNodeScript *script(QString path);   

    const JZFunctionDefine *function(QString name);

    QString irToString(JZNodeIR *ir);
    QString dump();   
    QString error();
    
protected:
    Q_DISABLE_COPY(JZNodeProgram);

    friend JZNodeBuilder;    
    QString toString(JZNodeIRParam param);
    
    QString m_filePath;
    QString m_error;

    JZNodeTypeMeta m_typeMeta;
    QMap<QString,JZNodeScriptPtr> m_scripts; 
    QMap<QString,JZParamDefine> m_variables;
};

#endif
