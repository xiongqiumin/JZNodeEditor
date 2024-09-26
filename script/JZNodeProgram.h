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
struct JZCORE_EXPORT NodeInfo
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
class JZCORE_EXPORT JZFunctionDebugInfo
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
class JZCORE_EXPORT JZNodeScript
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
class JZCORE_EXPORT JZNodeProgram 
{
public:
    JZNodeProgram();
    ~JZNodeProgram();

    bool isNull();
    bool load(QString file);
    bool save(QString file);
    void clear();

    const JZNodeTypeMeta &typeMeta() const;       
    QString applicationFilePath();
    
    QList<JZNodeScript*> scriptList();
    JZNodeScript *script(QString path);   
    
    const JZFunctionDebugInfo *debugInfo(QString name);        
    QString error();
    
protected:
    Q_DISABLE_COPY(JZNodeProgram);

    friend JZNodeBuilder;        
    
    QString m_filePath;
    QString m_error;

    JZNodeTypeMeta m_typeMeta;
    QMap<QString,JZNodeScriptPtr> m_scripts; 
    QMap<QString,JZParamDefine> m_variables;
};

#endif
