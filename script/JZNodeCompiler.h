#ifndef JZNODE_COMPILER_H_
#define JZNODE_COMPILER_H_

#include "JZNodeProgram.h"
#include "JZScriptItem.h"
#include "JZProject.h"

enum VariableCoor{
    Variable_none,
    Variable_local,
    Variable_member,
    Variable_global,
};

class JZScriptInOutInfo
{
public:
    QStringList inList;
    QStringList outList;
};

class GraphNode
{
public:
    GraphNode();

    QList<JZNodeGemo> outPinList();

    JZNode *node;    
    bool isReached;

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

    QList<GraphNode*> eventList();

    GraphNode *graphNode(int id);
    JZNode *node(int id);
    JZNodePin *pin(JZNodeGemo gemo);
    JZNodePin *pin(int nodeId, int pinId);
    QList<JZNodeGemo> pinInput(int nodeId, int pinId);
    QList<JZNodeGemo> pinOutput(int nodeId, int pinId);

    //检测是否可达
    void walkFlowNode(GraphNode *node);
    void walkParamNode(GraphNode *node);   

    QList<GraphNode*> topolist;
    QMap<int, GraphNodePtr> m_nodes;
    QString error;
};
typedef QSharedPointer<Graph> GraphPtr;

//编译时占位的 statment
enum {
    OP_ComilerFlowOut = 0x8000,
    OP_ComilerStackInit, 
    OP_ComilerBreakContinue,
    OP_ComilerAllocAuto,
};

//JZNodeIRFlowOut
class JZNodeComilerBreakContinue : public JZNodeIR
{
public:
    enum JumpType{
        Jmp_break,
        Jmp_continue,
    };

    JZNodeComilerBreakContinue(int nod_id, JumpType jump_type);
    bool isBreak();

    JumpType jumpType;
    int nodeId;
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
    JZNodeIRStackInit();
};

//JZNodeIRAutoInit
class JZNodeIRAutoInit : public JZNodeIR
{
public:
    JZNodeIRAutoInit();

    QString name;
};


//NodeCompilerInfo
struct NodeCompilerInfo
{
    NodeCompilerInfo();

    int node_id;
    int node_type;    
    QMap<int,int> pinType;   //pin 类型

    int start;
    QList<NodeRange> ranges; //用于断点信息   
    QMap<int, QList<NodeRange>> dataRanges;
    QList<JZNodeIRPtr> statmentList;
    bool autoAddDebugStart;    

    QWeakPointer<JZNodeIR> breakIr;
    QWeakPointer<JZNodeIR> continueIr;

    QString error;
};

//CompilerResult
class CompilerResult
{
public:
    bool result;
    QString checkError;
    QMap<int, QString> nodeError;    
};

enum CompilerTip{
    Error_noType,
    Error_noVariable,
    Error_noFunction,
    Error_noImplement,
    Error_noClassMember,
    Errro_initVariableFailed,

    Error_functionParamIn,
    Error_functionParamOut,
};

class JZNodeCompiler;
class JZMacroIRReplace
{
public:
    JZMacroIRReplace(JZNodeCompiler *c);
    void replace(QList<JZNodeIRPtr> &ir_list);

    void setLocalMap(QMap<QString, int> localMap);

    int stackId();
    void setStackId(int stackId);
    int stackType(int stackId);

protected:
    void replaceIr(JZNodeIRParam& ir);
    
    int m_stackId;
    QMap<QString, int> m_localMap;
    QMap<int, JZNodeIRParam> m_idMap;  //node stack id 都要换
    QMap<int, int> m_newStackType;
    JZNodeCompiler* m_compiler;
};


class JZNodeBuilder;
class JZNodeCompiler
{
public:
    static int paramId(int nodeId,int pinId);
    static int paramId(const JZNodeGemo &gemo);    
    static QString paramName(int id);
    static QString paramName(const JZNodeGemo& gemo);
    static JZNodeGemo paramGemo(int id);    
    static VariableCoor variableCoor(JZScriptItem *file, QString name);
    static const JZParamDefine *getVariableInfo(JZScriptItem *file, const QString &name);
    static const JZFunctionDefine* function(JZScriptItem* file, const QString& name);
    static QString errorString(CompilerTip tip,QStringList args);

    JZNodeCompiler();
    ~JZNodeCompiler();

    void setBuilder(JZNodeBuilder *builder);
    const JZScriptEnvironment* env();
    
    bool genGraphs(JZScriptItem *file, QVector<GraphPtr> &result);
    bool genNodeInputOuput(JZScriptItem *file, JZScriptInOutInfo &result);
    bool build(JZScriptItem *file,JZNodeScript *result);
    CompilerResult compilerResult();
    
    bool checkParamDefine(const JZParamDefine *def, QString &error);
    const JZParamDefine *getVariableInfo(const QString &name);
    bool checkVariableExist(const QString &var, QString &error);              //检查是否存在
    bool checkVariableType(const QString& var, QString data_type, QString& error); //检查变量是类型
    bool checkVariableType(const QString &var,int data_type, QString &error); 
    bool checkInitValue(int data_type,const QString &value);   //检查能否用字符串初始化    

    void resetStack();
    int allocStack(QString type);
    int allocStack(int dataType);  //只是标记分配
    void setStackId(int id);
    int stackId();   //指向下一个stack
    int stackType(int id);
    void addFunctionAlloc(const JZFunctionDefine &define);  //初始化本地变量
    
    JZNodeIRParam paramRef(QString name);

    int pinInputCount(int nodeId, int pinId);
    void setPinType(int nodeId, int pinId, int type);    
    int pinType(int nodeId, int pinId);
    int pinType(JZNodeGemo gemo);
    bool hasPinType(int nodeId, int pinId);

    void setRefType(QString ref, int type);
    int refType(QString ref);

    bool isPinLiteral(int nodeId, int pinId);
    QString pinLiteral(int nodeId, int pinId);
    
    int irParamType(const JZNodeIRParam &param);    

    void setRegCallFunction(const JZFunctionDefine *func);

    /*
    节点数据传递规则:
    flow 依赖 flow 节点, 被依赖flow节点计算后主动推送
    flow 依赖 data 节点, data节点展开
    data 依赖 flow 节点, 被依赖flow节点计算后主动推送
    data 依赖 data 节点, 节点计算前主动获取.

    addFlowInput，addDataInput 后，会自动插入JZNodeIRNodeId, 表示一个节点的开始, 用于断点
    */   
    bool checkPinInType(int nodeId, const QList<int> &prop_list, QString &error); //计算输入类型

    bool addFlowInput(int nodeId,QString &error);
    bool addFlowInput(int nodeId,const QList<int> &prop_id,QString &error);
    bool addDataInput(int nodeId,QString &error);
    bool addDataInput(int nodeId,const QList<int> &prop_id,QString &error); //获得指定nodeId 的 prop_id 输入
    void addFlowOutput(int nodeId);         

    int addNop();    
    int addNodeEnter(int id);
    void setAutoaddNodeEnter(int m_id,bool flag);
    int addExpr(const JZNodeIRParam &dst, const JZNodeIRParam &p1, const JZNodeIRParam &p2, JZNodeIRType op);
    void addExprConvert(const JZNodeIRParam &dst, const JZNodeIRParam &p1, const JZNodeIRParam &p2, JZNodeIRType op);
    int addSingleExpr(const JZNodeIRParam &dst, const JZNodeIRParam &p1, JZNodeIRType op);
    int addCompare(const JZNodeIRParam &p1, const JZNodeIRParam &p2, JZNodeIRType op);
    void addCompareConvert(const JZNodeIRParam &p1, const JZNodeIRParam &p2, JZNodeIRType op);
    void addInitVariable(const JZNodeIRParam &dst, int dataType, const QString &value = QString());
    void addSetVariable(const JZNodeIRParam &dst, const JZNodeIRParam &src);   
    void addSetVariableConvert(const JZNodeIRParam &dst, const JZNodeIRParam &src);  //包含显示类型转换
    void addSetBuffer(const JZNodeIRParam &dst, const QByteArray &buffer);
    void addSetJson(const JZNodeIRParam& dst, const QString &name, const JZNodeIRParam &src);
    void addGetJson(const JZNodeIRParam& dst, const QString &name, const JZNodeIRParam &src);

    void addConvert(const JZNodeIRParam &dst, int dst_type, const JZNodeIRParam &src); //显示转换不检测能否转换
    int addStatement(JZNodeIRPtr ir);  
    
    JZNodeIRJmp* addJmp(JZNodeIRType type);
    int addContinue(int node_id);
    int addBreak(int node_id);
    void setBreakContinue(int breakPc, int continuePC);
    JZNode* breakParentNode(int child_id);
    JZNode* continueParentNode(int child_id);

    void addConstructor(SignalConnectInfo info);
    
    void addAlloc(int allocType, QString name, QString dataType);
    void addAlloc(int allocType, QString name, int dataType);
    void addAllocAuto(const QString& ir);
    void addCall(const QString &function, const QList<JZNodeIRParam> &paramIn, const QList<JZNodeIRParam> &paramOut);
    void addCall(const JZFunctionDefine *function, const QList<JZNodeIRParam> &paramIn, const QList<JZNodeIRParam> &paramOut);
    void addCallVirtual(const QString &function, const QList<JZNodeIRParam> &paramIn, const QList<JZNodeIRParam> &paramOut);  
    void addCallConvert(const QString &function, const QList<JZNodeIRParam> &paramIn, const QList<JZNodeIRParam> &paramOut);
    void addCallConvert(const JZFunctionDefine *function, const QList<JZNodeIRParam> &paramIn, const QList<JZNodeIRParam> &paramOut);
    void addAssert(const JZNodeIRParam &tips);       
        
    JZNode* nextFlowNode(JZNode* node, int pin);
    bool buildSubControlFlow(JZNode* node, QList<JZNodeIRPtr>& list);

    JZNodeIR *statment(int index);
    JZNodeIR *lastStatment();
    int indexOfStatment(JZNodeIR *);
    void removeStatement(int pc);
    void replaceStatement(int pc,JZNodeIRPtr ir);
    void replaceStatementList(int pc,QList<JZNodeIRPtr> ir_list);    
    void appendStatementList(const QList<JZNodeIRPtr>& ir_list);
    void adjustStatementPc(int start_index, int jmp_cond, int adjust);

    JZScriptItem *currentFile();
    Graph *currentGraph();
    JZNode *currentNode();

    int currentPc();
    int nextPc();
    const JZFunctionDefine *function(QString name);

protected:    
    friend JZNodeBuilder;

    struct NodeCompilerStack
    {
        NodeCompilerStack();

        NodeCompilerInfo *nodeInfo;
        bool isFlow;
        int start;
        int debugStart;
    };

    void init(JZScriptItem *file);
    bool genGraphs();
    bool checkGraphs();
    bool isBuildError();
    bool checkBuildStop();
    Graph *getGraph(JZNode *node);
    void connectGraph(Graph *,JZNode *node);
    bool buildDataFlow(const QList<GraphNode*> &list);
    
    /*
    先编译各个flow节点，最后连接，分开编译是因为out的时候无法确定对应输入节点的类型
    */
    bool buildControlFlow(JZNode* node);
    bool isAllFlowReturn(JZNode *node,int level);
    void addFunction(const JZFunctionDefine &define,int start_addr,int end_addr);    
    QString nodeName(JZNode *node);
    QString pinName(JZNodePin *pin);         

    void updateBuildGraph(const QList<GraphNode*> &root_list);
    bool checkFunction();
    
    bool compilerNode(JZNode *node);
    void pushCompilerNode(int id);
    void popCompilerNode();    
    
    void initStatmentStack(QList<JZNodeIRPtr>* statments);
    void pushStatmentList(QList<JZNodeIRPtr> *statments);
    void popStatmentList();
    int indexOfStatmentList(QList<JZNodeIRPtr>* statments,int op_type);
    
    void setOutPinTypeDefault(JZNode *node);      //只有一种输出的设置为默认值
    void updateFlowOut();      
    bool irParamTypeMatch(const JZNodeIRParam &p1,const JZNodeIRParam &p2,bool isSet);
    void dealAddCall(bool isVirtual,const JZFunctionDefine *func, const QList<JZNodeIRParam> &paramIn, const QList<JZNodeIRParam> &paramOut);
    bool hasStatementDepend(int pc);
    
    void log(QString error);
    void logE(QString error,const QVariantMap &args = QVariantMap());

    NodeCompilerInfo *currentNodeInfo();
    JZProject *project();

    /* build info*/        
    QVector<GraphPtr> m_graphList;

    QString m_className;    
    const JZFunctionDefine *m_regCallFunction;
    QList<JZNodeIRParam> m_regCallInput;
    JZNodeScript *m_script;
    JZScriptItem *m_scriptItem;         
    Graph *m_originGraph; //原始图
    GraphPtr m_buildGraph;  //当前处理的图
        
    QList<NodeCompilerStack> m_compilerNodeStack;       //编译时node栈
    QList<JZNodeIRPtr> *m_statmentList;   
    QList<QList<JZNodeIRPtr>*> m_statmentStak;

    QMap<JZNode*,Graph*> m_nodeGraph;     //构建连通图使用

    QString m_checkError;
    QMap<int,NodeCompilerInfo> m_nodeInfo;
    int m_stackId;       //当前栈位置，用于分配内存    
    QMap<int,int> m_stackType;     //stack 参数类型
    QMap<QString, int> m_refType;
    CompilerResult m_compilerInfo;
    
    const JZScriptEnvironment *m_env = nullptr;
    JZNodeBuilder *m_builder;
    QString m_ignoreError;  // 由于buildData 错误引起, 后续节点不在记录错误
};

#endif
