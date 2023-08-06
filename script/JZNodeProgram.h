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
    Reg_Call = Reg_Start,               //函数传递参数, 调用函数时将 RegCall 数据拷贝到 Stack_User
    Reg_Cmp = 2500000,
};

//TopoTool
class TopoTool
{
public:
    TopoTool();

    void addDepend(int id,QStringList in,QStringList out);
    bool toposort(QList<int> &list);

protected:
    class TopoNode
    {
    public:
        int id;
        QStringList in;
        QStringList out;
        QList<int> next;
    };
    QList<TopoNode> m_list;
};

class GraphNode
{
public:
    GraphNode();

    JZNode *node;
    QList<JZNodeGemo> next;    

    QMap<int,int> pinType;  //节点数据类型
    QMap<int,QList<JZNodeGemo>> paramIn;  //输入位置
    QMap<int,QList<JZNodeGemo>> paramOut; //输出位置
};
typedef QSharedPointer<GraphNode> GraphNodePtr;

//Graph
class Graph
{
public:
    Graph();
    ~Graph();

    bool check();
    bool toposort(); //拓扑排序
    void clear();

    GraphNode *graphNode(int id);
    JZNode *node(int id);
    JZNodePin *pin(JZNodeGemo gemo);
    JZNodePin *pin(int nodeId,int pinId);

    QList<GraphNode*> topolist;
    QMap<int, GraphNodePtr> m_nodes;
    QString error;
};
typedef QSharedPointer<Graph> GraphPtr;

//JZEventHandle
class JZEventHandle
{
public:
    JZEventHandle();
    
    int type;        
    QString sender;
    FunctionDefine function;
};
QDataStream &operator<<(QDataStream &s, const JZEventHandle &param);
QDataStream &operator>>(QDataStream &s, JZEventHandle &param);

//JZEventHandle
class JZParamChangeHandle
{
public:
    JZParamChangeHandle();

    QString paramName;
    FunctionDefine function;
};
QDataStream &operator<<(QDataStream &s, const JZParamChangeHandle &param);
QDataStream &operator>>(QDataStream &s, JZParamChangeHandle &param);

//NodeInfo
struct NodeInfo
{        
    NodeInfo();
    
    int node_id;
    int node_type;
    bool isFlow;   
    QString error;
};
QDataStream &operator<<(QDataStream &s, const NodeInfo &param);
QDataStream &operator>>(QDataStream &s, NodeInfo &param);

//JZFunction
class JZFunction
{
public:
    QString file;
    int addr;
};
QDataStream &operator<<(QDataStream &s, const JZFunction &param);
QDataStream &operator >> (QDataStream &s, JZFunction &param);

//JZNodeScript
class JZNodeScript
{    
public:    
    JZNodeScript();
    void clear();    
    FunctionDefine *function(QString name);

    QString file;
    QString className;
    QList<GraphPtr> graphs;     
    QMap<int, NodeInfo> nodeInfo;
    QList<JZNodeIRPtr> statmentList;
    QList<JZParamDefine> localVariable;

    QList<JZParamChangeHandle> paramChanges;    
    QList<JZEventHandle> events;
    QList<FunctionDefine> functionList;        
    QList<JZNodeIRParam> watchList;

    QMap<QString, JZFunction> runtimeInfo;

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

    bool load(QString file);
    bool save(QString file);
    void clear();

    FunctionDefine *function(QString name);
    JZNodeScript *script(QString path);
    JZNodeScript *functionScript(QString function);
    int functionAddr(QString function);

    QList<JZNodeScript*> scriptList();
    JZNodeScript *objectScript(QString className);    
    QList<JZEventHandle*> eventList() const;

    QList<JZNodeObjectDefine> objectDefines();
    QMap<QString,JZParamDefine> variables();        
    QString dump();   
    QString error();
    
protected:
    Q_DISABLE_COPY(JZNodeProgram);

    friend JZNodeBuilder;    
    QString paramName(JZNodeIRParam param);

    QStringList m_opNames;
    QMap<QString,JZNodeScriptPtr> m_scripts; 
    QMap<QString,JZParamDefine> m_variables;        
    QList<JZNodeObjectDefine> m_objectDefines;        
    QString m_error;
};

#endif
