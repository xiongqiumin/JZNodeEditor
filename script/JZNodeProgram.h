#ifndef JZNODE_PROGRAM_H_
#define JZNODE_PROGRAM_H_

#include <QThread>
#include "JZNode.h"
#include "JZNodeEvent.h"
#include "JZEvent.h"
#include "JZNodeIR.h"
#include "JZNodeFunctionDefine.h"
#include "JZNodeObject.h"
#include "JZScriptEnvironment.h"

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
    NodeParamInfo *param(int id);
    
    QString name;
    int id;
    int type;
    bool isFlow;       
    
    //paramIn,paramOut 不保存全部的节点，只保存运行时可以修改的节点信息
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
    const JZParamDefine *localParam(QString name) const;
    const JZParam *nodeParam(int id) const;

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

    const JZFunction *function(QString name) const;
    const JZFunctionDebugInfo *functionDebug(QString name) const;
    
    void copyTo(JZNodeScript *other) const;
    void saveToStream(QDataStream &s) const;
    void loadFromStream(QDataStream &s);

    QString itemPath;
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
        QStringList params;
    };
    
    void clear();    
    FunctionHook *getHook(int node_id);

    JZFunctionDefine function;
    QMap<QString,QString> member;
    QMap<QString,QString> global;
    QList<FunctionHook> hook;
};

//JZNodeProgram
class JZNodeBuilder;
class JZNodeProgram 
{
public:
    JZNodeProgram();
    ~JZNodeProgram();

    bool isNull() const;

    bool load(QString file,QString &error);
    bool save(QString file);
    void clear();
    void copyTo(JZNodeProgram *other) const;
    
    void initEnv(JZScriptEnvironment *env) const;

    const JZNodeTypeMeta &typeMeta() const;       
    QString applicationFilePath() const;
    
    const JZFunction* function(QString name) const;

    QList<JZNodeScript*> scriptList() const;
    const JZNodeScript *script(QString path) const;
    void addScript(QString path, JZNodeScriptPtr);
    
    const JZFunctionDebugInfo *debugInfo(QString name) const;
    
protected:
    Q_DISABLE_COPY(JZNodeProgram);

    friend JZNodeBuilder;        

    void saveToStream(QDataStream &s) const;
    void loadFromStream(QDataStream &s);
     
    QString m_filePath;
    QString m_error;

    JZNodeTypeMeta m_typeMeta;
    QMap<QString,JZNodeScriptPtr> m_scripts; 
    QMap<QString,JZParamDefine> m_variables;
};

#endif
