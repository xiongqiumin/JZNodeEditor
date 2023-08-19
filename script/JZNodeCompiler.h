#ifndef JZNODE_COMPILER_H_
#define JZNODE_COMPILER_H_

#include "JZProject.h"
#include "JZNodeProgram.h"
#include "JZNodeExpression.h"
#include "JZScriptFile.h"
#include "JZProject.h"

class GraphNode
{
public:
    GraphNode();

    JZNode *node;
    QList<JZNodeGemo> next;

    QMap<int, int> pinType;  //节点数据类型
    QMap<int, QList<JZNodeGemo>> paramIn;  //输入位置
    QMap<int, QList<JZNodeGemo>> paramOut; //输出位置
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
    JZNodePin *pin(int nodeId, int pinId);

    QList<GraphNode*> topolist;
    QMap<int, GraphNodePtr> m_nodes;
    QString error;
};
typedef QSharedPointer<Graph> GraphPtr;

//NodeCompilerInfo
struct NodeCompilerInfo
{
    NodeCompilerInfo();

    struct Jump
    {
        int prop;
        int pc;
    };

    int node_id;
    int node_type;
    int start;              //flow节点起始结束
    int end;
    QList<NodeRange> ranges;
    QList<int> continuePc;
    QList<int> breakPc;
    int parentId;           //用于subFlow 指明父节点
    QList<Jump> jmpList;
    QList<Jump> jmpSubList;
    QList<int> continueList;
    QList<int> breakList;
    QString error;
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

    bool genGraphs(JZScriptFile *file, QVector<GraphPtr> &result);
    bool build(JZScriptFile *file,JZNodeScript *result);
    const QMap<int, NodeCompilerInfo> &compilerInfo();
    
    static const JZParamDefine *getVariableInfo(JZScriptFile *file, const QString &name);
    const JZParamDefine *getVariableInfo(const QString &name);
    bool checkVariableExist(const QString &var, QString &error);
    bool checkVariableType(const QString &var, const QString &className, QString &error);

    int allocStack();
    void freeStack(int id);    
    void allocFunctionVariable();    

    /*
    节点数据传递规则:
    flow 依赖 flow 节点, 被依赖flow节点计算后主动推送
    flow 依赖 data 节点, data节点展开
    data 依赖 flow 节点, 被依赖flow节点计算后主动推送
    data 依赖 data 节点, 节点计算前主动获取.
    */
    bool addFlowInput(int nodeId,QString &error);
    bool addFlowInput(int nodeId,int prop_id,QString &error);
    bool addDataInput(int nodeId,QString &error);
    bool addDataInput(int nodeId,int prop_id,QString &error);
    void addFlowOutput(int nodeId);            

    int addNop();
    int addExpr(const JZNodeIRParam &dst, const JZNodeIRParam &p1, const JZNodeIRParam &p2,int op);
    int addCompare(const JZNodeIRParam &p1, const JZNodeIRParam &p2,int op);
    int addSetVariable(const JZNodeIRParam &dst, const JZNodeIRParam &src);
    int addStatement(JZNodeIRPtr ir);    
    int addJumpNode(int prop);      //设置下一个flow,应当在执行完操作后增加
    int addJumpSubNode(int prop);   //设置下一个sub flow
    int addContinue();
    int addBreak();    
    void addCall(const JZNodeIRParam &function, const QList<JZNodeIRParam> &paramIn, const QList<JZNodeIRParam> &paramOut);
    void addAllocLocal(JZParamDefine *def);    
    void addAssert(const JZNodeIRParam &tips);

    void setBreakContinue(const QList<int> &breakPc, const QList<int> &continuePC);
    void replaceStatement(int pc,JZNodeIRPtr ir);
    
    Graph *currentGraph();
    int currentPc();
    const FunctionDefine *function(QString name);

    QString error();    

protected:                
    void init(JZScriptFile *file);
    bool genGraphs();
    bool checkGraphs();
    bool checkBuildResult();
    Graph *getGraph(JZNode *node);
    void connectGraph(Graph *,JZNode *node);
    bool buildDataFlow(const QList<GraphNode*> &list);
    bool bulidControlFlow(Graph *graph);    
    bool buildParamBinding(Graph *graph);
    void buildWatchInfo(Graph *graph);
    void replaceSubNode(int id,int parentId,int flow_index);   
    void addEventHandle(const QList<GraphNode*> &list);      
    QString nodeName(JZNode *node);
    QString pinName(JZNodePin *prop);    
    bool compilerNode(JZNode *node);
    void pushCompilerNode(int id);
    void popCompilerNode();
    void allocLocalVariable(JZNodeIRParam param);
                
    /* build info*/        
    QString m_className;
    int m_stackId;          //当前栈位置
    JZNodeScript *m_script;
    JZScriptFile *m_scriptFile;         
    Graph *m_currentGraph;       
    QVector<GraphPtr> m_graphList;

    NodeCompilerInfo *m_currentNodeInfo;
    JZNode* m_currentNode;        
    QList<int> m_compilerNodeStack;       //编译时node栈

    QMap<JZNode*,Graph*> m_nodeGraph;     //构建连通图使用
    QMap<int,NodeCompilerInfo> m_nodeInfo;
    QList<JZParamDefine> m_localVaribales;
    
    JZProject *m_project;
    QString m_error;    
};

#endif
