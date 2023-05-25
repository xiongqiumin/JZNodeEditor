#ifndef JZNODE_COMPILER_H_
#define JZNODE_COMPILER_H_

#include "JZProject.h"
#include "JZNodeProgram.h"
#include "JZNodeExpression.h"
#include "JZScriptFile.h"

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
     
    bool build(JZScriptFile *file,JZNodeScript *result);        

    int allocStack();
    void freeStack(int id);

    void addFlowInput(int nodeId);
    void addFlowOutput(int nodeId);
    void addDataInput(int nodeId);

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
    void setBreakContinue(QList<int> breakPc,QList<int> continuePC);    
    void replaceStatement(int pc,JZNodeIRPtr ir);

    NodeInfo *currentNodeInfo();
    int currentPc();

    QString error();    

protected:            
    bool genGraphs();
    Graph *getGraph(JZNode *node);
    void connectGraph(Graph *,JZNode *node);
    bool buildDataFlow(const QList<GraphNode*> &list);
    bool bulidControlFlow(Graph *graph);    
    bool buildParamBinding(Graph *graph);
    void replaceSubNode(int id,int parentId,int flow_index);   
    void addEventHandle(const QList<GraphNode*> &list);    
                
    /* build info*/        
    JZNodeScript *m_script;
    JZScriptFile *m_scriptFile;     
    Graph *m_currentGraph;       
    NodeInfo *m_currentNodeInfo;
    QMap<JZNode*,Graph*> m_nodeGraph;
    QMap<int,NodeInfo> m_nodeInfo;
    
    QString m_error;
    int m_stackId;
};

#endif
