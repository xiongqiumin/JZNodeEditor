#ifndef JZNODE_COMPILER_H_
#define JZNODE_COMPILER_H_

#include "JZProject.h"
#include "JZNodeProgram.h"
#include "JZNodeExpression.h"
#include "JZScriptFile.h"

class GraphNode
{
public:
    GraphNode();

    JZNode *node;
    QList<JZNodeGemo> next;

    QMap<int,QList<JZNodeGemo>> paramIn;  //输入位置
    QMap<int,QList<JZNodeGemo>> paramOut; //输出位置
};
typedef QSharedPointer<GraphNode> GraphNodePtr;

class Graph
{
public:
    Graph();
    ~Graph();

    bool check();
    bool toposort(); //拓扑排序
    void clear();

    QList<GraphNode*> topolist;
    QMap<int, GraphNodePtr> m_nodes;
    QString error;
};
typedef QSharedPointer<Graph> GraphPtr;

struct NodeInfo
{        
    struct Jump
    {            
        int pc;
    };        

    int start;
    int end;
    QList<Jump> jmpList;     
};

struct ScriptInfo
{    
    QList<GraphPtr> graphs;         
    QMap<JZNode*,Graph*> nodeGraph;
    QMap<int,NodeInfo> nodeInfo;
    QList<JZEventHandle> events;

    QList<JZNodeIR> statmentList;
};


class JZNodeCompiler
{
public:
    static int paramId(int nodeId,int propId);
    static int paramId(const JZNodeGemo &gemo);    
    static QString paramName(int id);
    static JZNodeGemo paramGemo(int id);    

    JZNodeCompiler();
    ~JZNodeCompiler();
     
    bool build(JZScriptFile *file);
    ScriptInfo result();    

    int allocReg();
    void freeReg(int id);
    void copyVariable(int src,int dst);
    int addStatement(const JZNodeIR &ir);
    void addJumpNode(int idx);

    QString error();

protected:            
    bool genGraphs();
    Graph *getGraph(JZNode *node);
    void connectGraph(Graph *,JZNode *node);
    bool buildGraph(Graph *graph);
    bool addFlowEvent();
    bool addParamChangedEvent();
                
    /* build info*/        
    ScriptInfo m_info;
    JZScriptFile *m_script; 
    Graph *m_currentGraph;   
    NodeInfo *m_currentNodeInfo;

    QString m_error;
    int m_regId;
};

#endif
