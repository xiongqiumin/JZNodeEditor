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
    
    int node_id;
    int node_type;
    bool isFlow;       
    
    QList<NodeParamInfo> paramIn;
    QList<NodeParamInfo> paramOut;

    QList<NodeRange> pcRanges;
};
QDataStream &operator<<(QDataStream &s, const NodeInfo &param);
QDataStream &operator>>(QDataStream &s, NodeInfo &param);

//NodeWatch
class NodeWatch
{
public:
    int traget;
    QList<int> source;
};
QDataStream &operator<<(QDataStream &s, const NodeWatch &param);
QDataStream &operator>>(QDataStream &s, NodeWatch &param);

//JZNodeScript
class JZNodeScript
{    
public:    
    JZNodeScript();
    void clear();    

    JZFunction *function(QString name);
    
    JZNodeScript *clone();
    void saveToStream(QDataStream &s);
    void loadFromStream(QDataStream &s);

    QString file;
    QString className; 
    QList<JZNodeIRPtr> statmentList;

    QList<JZFunction> functionList;    
    QList<NodeWatch> watchList;           //display node        
    QMap<int,NodeInfo> nodeInfo;   
    QList<JZParamDefine> localVariables;

protected:
    Q_DISABLE_COPY(JZNodeScript);    
};
typedef QSharedPointer<JZNodeScript> JZNodeScriptPtr;

class JZNodeTypeMeta
{
public:
    void clear();

    const JZFunctionDefine *function(QString name) const;
    const JZNodeObjectDefine *object(QString name) const;

    QList<JZFunctionDefine> functionList;
    QList<JZNodeObjectDefine> objectList;       
    QList<JZNodeCObjectDelcare> cobjectList;
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
    QMap<QString, JZNodeParamBind> bindInfo(QString className);    

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
    QMap<QString, QMap<QString, JZNodeParamBind>> m_binds;    
};

#endif
