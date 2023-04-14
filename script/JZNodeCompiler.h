#ifndef JZNODE_COMPILER_H_
#define JZNODE_COMPILER_H_

#include "JZProject.h"
#include "JZNodeProgram.h"
#include "JZNodeExpression.h"

enum
{
    Build_none,
    Build_flow,
    Build_paramBinding,
    Build_function,    
};

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

class JZNodeCompiler
{
public:
    JZNodeCompiler();
    ~JZNodeCompiler();

    bool build(JZProject *project,JZNodeProgram &result);    
    const QList<Graph*> &graphs() const;        

    int paramId(int nodeId,int propId);
    int paramId(const JZNodeGemo &gemo);    

    int allocReg();
    void freeReg(int id);
    void copyVariable(int src,int dst);
    int addStatement(const JZNodeIR &ir);
    void addJumpNode(int idx);

    QString error();

protected:
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
    
    bool genGraphs();
    Graph *getGraph(JZNode *node);
    void connectGraph(Graph *,JZNode *node);
    
    bool buildGraph(Graph *graph);
    bool addFlowEvent();
    bool addParamChangedEvent();
    
    JZNodeProgram m_program;
    JZProject *m_project;        

    QList<GraphPtr> m_graphs;     
    Graph *m_currentGraph;   
    QMap<JZNode*,Graph*> m_nodeGraph;        

    QMap<int,NodeInfo> m_nodeInfo;
    NodeInfo *m_currentNodeInfo;
    int m_buildType;

    QString m_error;
    int m_regId;
};

#endif
