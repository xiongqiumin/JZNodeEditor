#ifndef JZNODE_COMPILER_H_
#define JZNODE_COMPILER_H_

#include "JZProject.h"
#include "JZNodeProgram.h"
#include "JZNodeExpression.h"
#include "JZScriptFile.h"
#include "JZProject.h"

/*
    节点数据传递规则:
    依赖 flow 节点, 被依赖节点计算后主动推送
    依赖 data 节点, 节点计算前主动获取.
*/
class JZNodeCompiler
{
public:
    static int paramId(int nodeId,int propId);
    static int paramId(const JZNodeGemo &gemo);    
    static QString paramName(int id);
    static JZNodeGemo paramGemo(int id);    

    JZNodeCompiler();
    ~JZNodeCompiler();

    bool genGraphs(JZScriptFile *file,JZNodeScript *result);
    bool build(JZScriptFile *file,JZNodeScript *result);

    int allocStack();
    void freeStack(int id);

    bool checkVariableExist(QString var,QString &error);
    bool checkVariableType(QString var,QString className,QString &error);

    bool addFlowInput(int nodeId,QString &error);
    bool addDataInput(int nodeId,QString &error);
    void addFlowOutput(int nodeId);        
    JZNodeIRParam localVariable(JZNodeIRParam param);
    int addExpr(JZNodeIRParam dst,JZNodeIRParam p1,JZNodeIRParam p2,int op);
    int addCompare(JZNodeIRParam p1,JZNodeIRParam p2,int op);
    int addSetVariable(JZNodeIRParam dst,JZNodeIRParam src);
    int addStatement(JZNodeIRPtr ir);    
    int addJumpNode(int prop);
    int addJumpSubNode(int prop);
    int addContinue();
    int addBreak();
    int addReturn();
    void addCall(JZNodeIRParam function,QList<JZNodeIRParam> paramIn,QList<JZNodeIRParam> paramOut);
    void setBreakContinue(QList<int> breakPc,QList<int> continuePC);    
    void replaceStatement(int pc,JZNodeIRPtr ir);
    
    int currentPc();
    const FunctionDefine *function(QString name);

    QString error();    

protected:            
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
        QList<int> continuePc;
        QList<int> breakPc;
        int parentId;           //用于subFlow 指明父节点
        QList<Jump> jmpList;
        QList<Jump> jmpSubList;    
        QList<int> continueList;    
        QList<int> breakList;
        QString error;
    };

    void init(JZScriptFile *file,JZNodeScript *result);
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
                
    /* build info*/        
    JZNodeScript *m_script;
    JZScriptFile *m_scriptFile;     
    JZProject *m_project;
    Graph *m_currentGraph;       
    NodeCompilerInfo *m_currentNodeInfo;
    JZNode* m_currentNode;
    QList<int> m_compilerNodes;

    QMap<JZNode*,Graph*> m_nodeGraph;
    QMap<int,NodeCompilerInfo> m_nodeInfo;
    
    QString m_error;
    int m_stackId;
};

#endif
