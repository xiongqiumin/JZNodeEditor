#ifndef JZNODE_PROGRAM_H_
#define JZNODE_PROGRAM_H_

#include <QThread>
#include "JZNode.h"
#include "JZNodeEvent.h"
#include "JZEvent.h"
#include "JZNodeIR.h"
#include "JZNodeFunctionDefine.h"
#include "JZNodeObject.h"

enum{    
    Stack_Node = 0,
    Stack_User = 1000000,    

    Reg_Start = 2000000,    
    Reg_Pc,
    Reg_Cmp,
    Reg_Call,   //函数传递参数, 调用函数时将 RegCall 数据拷贝到 Stack_User    
};

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

    QString file;
    QString className;
    QMap<int, NodeInfo> nodeInfo;    
    QList<JZNodeIRPtr> statmentList;            

    QList<JZFunction> functionList;    
    QList<NodeWatch> watchList;    //display node    

    void saveToStream(QDataStream &s);
    void loadFromStream(QDataStream &s);

protected:
    Q_DISABLE_COPY(JZNodeScript);    
};
typedef QSharedPointer<JZNodeScript> JZNodeScriptPtr;

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
    QString applicationFilePath();
    
    QStringList functionList();
    JZFunction *function(QString name);
    JZNodeScript *script(QString path);    

    QList<JZNodeScript*> scriptList();    
    QMap<QString, QString> bindInfo(QString className);
    
    QList<JZNodeObjectDefine> objectDefines();
    QMap<QString,JZParamDefine> globalVariables();    

    QString irToString(JZNodeIR *ir);
    QString dump();   
    QString error();
    
protected:
    Q_DISABLE_COPY(JZNodeProgram);

    friend JZNodeBuilder;    
    QString toString(JZNodeIRParam param);
    
    QString m_filePath;
    QString m_error;

    QMap<QString,JZNodeScriptPtr> m_scripts; 
    QMap<QString,JZParamDefine> m_variables;        
    QList<JZNodeObjectDefine> m_objectDefines;        
    QMap<QString, QMap<QString, QString>> m_binds;    
};

#endif
