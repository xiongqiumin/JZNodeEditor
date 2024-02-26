#ifndef JZNODE_COMPILER_H_
#define JZNODE_COMPILER_H_

#include "JZProject.h"
#include "JZNodeProgram.h"
#include "JZNodeExpression.h"
#include "JZScriptItem.h"
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

enum {
    OP_ComilerFlowOut = 0x8000,
    OP_ComilerStackInit,    
};

//JZNodeIRFlowOut
class JZNodeIRFlowOut : public JZNodeIR
{
public:
    JZNodeIRFlowOut();

    int fromId;
    int toId;
};

//JZNodeIRStackInit
class JZNodeIRStackInit : public JZNodeIR
{
public:
    JZNodeIRStackInit(int node_id,int prop_id);

    int nodeId;
    int propId;
};

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
    QMap<int,int> pinType;   //pin 类型

    int start;
    QList<NodeRange> ranges; //用于断点信息   
    QMap<int, QList<NodeRange>> dataRanges;
    QList<JZNodeIRPtr> statmentList;

    //调转信息    
    int parentId;           //用于subFlow 指明父节点
    QList<Jump> jmpList;
    QList<Jump> jmpSubList;
    QList<int> continuePc;  //父节点continuet跳出的地址
    QList<int> breakPc;
    QList<int> continueList; //子节点continue语句地址
    QList<int> breakList;
    int allSubReturn;

    QString error;
};

//CompilerInfo
class CompilerInfo
{
public:
    QMap<int, NodeCompilerInfo> nodeInfo;    
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

    bool genGraphs(JZScriptItem *file, QVector<GraphPtr> &result);
    bool build(JZScriptItem *file,JZNodeScript *result);        
    CompilerInfo compilerInfo();
    
    static const JZParamDefine *getVariableInfo(JZScriptItem *file, const QString &name);
    const JZParamDefine *getVariableInfo(const QString &name);
    bool checkVariableExist(const QString &var, QString &error);
    bool checkVariableType(const QString &var, const QString &className, QString &error);    

    void resetStack();
    int allocStack(int dataType);
    void addFunctionAlloc(const FunctionDefine &define);
    
    void setPinType(int nodeId, int propId, int type);    
    int pinType(int nodeId, int propId);
    int pinType(JZNodeGemo gemo);
    bool hasPinType(int nodeId, int propId);

    /*
    节点数据传递规则:
    flow 依赖 flow 节点, 被依赖flow节点计算后主动推送
    flow 依赖 data 节点, data节点展开
    data 依赖 flow 节点, 被依赖flow节点计算后主动推送
    data 依赖 data 节点, 节点计算前主动获取.

    addFlowInput，addDataInput 后，会自动插入JZNodeIRNodeId, 表示一个节点的开始, 用于断点
    */
    bool addFlowInput(int nodeId,QString &error);
    bool addFlowInput(int nodeId,int prop_id,QString &error);
    bool addDataInput(int nodeId,QString &error);
    bool addDataInput(int nodeId,int prop_id,QString &error); //获得指定nodeId 的 prop_id 输入
    void addFlowOutput(int nodeId);            

    int addNop();
    void addNodeStart(int id);
    int addExpr(const JZNodeIRParam &dst, const JZNodeIRParam &p1, const JZNodeIRParam &p2,int op);
    int addCompare(const JZNodeIRParam &p1, const JZNodeIRParam &p2,int op);
    int addSetVariable(const JZNodeIRParam &dst, const JZNodeIRParam &src);    
    int addStatement(JZNodeIRPtr ir);    
    
    int addJumpNode(int prop);      //设置下一个flow,应当在执行完操作后增加
    int addJumpSubNode(int prop);   //设置下一个sub flow    
    int addContinue();
    int addBreak();    
    void setBreakContinue(const QList<int> &breakPc, const QList<int> &continuePC);
    
    void addCall(const JZNodeIRParam &function, const QList<JZNodeIRParam> &paramIn, const QList<JZNodeIRParam> &paramOut);
    void addCall(const FunctionDefine *function, const QList<JZNodeIRParam> &paramIn, const QList<JZNodeIRParam> &paramOut);
    void addAllocLocal(const JZParamDefine *def, const JZNodeIRParam &value = JZNodeIRParam());
    void addAssert(const JZNodeIRParam &tips);       

    JZNodeIR *lastStatment();
    void replaceStatement(int pc,JZNodeIRPtr ir);
    void replaceStatement(int pc,QList<JZNodeIRPtr> ir_list);
    
    JZScriptItem *currentFile();
    Graph *currentGraph();
    int currentPc();
    int nextPc();
    const FunctionDefine *function(QString name);

    QString error();    

protected:    
    struct NodeCompilerStack
    {
        NodeCompilerInfo *nodeInfo;
        bool isFlow;
        int start;
        int debugStart;
    };

    void init(JZScriptItem *file);
    bool genGraphs();
    bool checkGraphs();
    bool checkBuildResult();
    Graph *getGraph(JZNode *node);
    void connectGraph(Graph *,JZNode *node);
    bool buildDataFlow(const QList<GraphNode*> &list);
    bool bulidControlFlow();    
    bool buildParamBinding();
    void replaceSubNode(int id,int parentId,int flow_index);
    int isAllFlowReturn(int id,bool root);
    void addEventHandle(const QList<GraphNode*> &list);      
    void addFunction(const FunctionDefine &define,int node_id);
    QString nodeName(JZNode *node);
    QString pinName(JZNodePin *prop);     
    
    bool compilerNode(JZNode *node);
    void pushCompilerNode(int id);
    void popCompilerNode();    

    bool checkPinInType(int nodeId,int prop_id,QString &error); //计算输入类型
    void setOutPinTypeDefault(JZNode *node);      //只有一种输出的设置为默认值
    void updateFlowOut();    
    void linkNodes();
    void updateDebugInfo();
    void updateDispayNode();
    void addNodeFlowPc(int node_id, int cond, int pc);
                
    JZNode* currentNode();
    NodeCompilerInfo *currentNodeInfo();

    /* build info*/        
    QVector<GraphPtr> m_graphList;

    QString m_className;    
    JZNodeScript *m_script;
    JZScriptItem *m_scriptFile;         
    Graph *m_currentGraph;               
        
    QList<NodeCompilerStack> m_compilerNodeStack;       //编译时node栈
    QList<JZNodeIRPtr> *m_statmentList;

    QMap<JZNode*,Graph*> m_nodeGraph;     //构建连通图使用
    QMap<int,NodeCompilerInfo> m_nodeInfo;        
    int m_stackId;       //当前栈位置，用于分配内存

    JZProject *m_project;
    QString m_error;
};

#endif
