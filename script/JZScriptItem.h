#ifndef JZNODE_SCRIPT_FILE_H_
#define JZNODE_SCRIPT_FILE_H_

#include "JZNode.h"
#include "JZProjectItem.h"
#include "JZNodeFunctionDefine.h"
#include "JZNodeObject.h"

class JZScriptClassItem;
class JZNodeEvent;
class JZScriptItem : public JZProjectItem
{
public:    
    enum ScriptType{
        None,
        Function,
        Flow,
    };

    JZScriptItem(ScriptType type);
    virtual ~JZScriptItem();

    bool isFunction() const;

    void loadFinish();
    void clear();
    int nextId();    

    ScriptType scriptType() const;

    const JZFunctionDefine &function();
    void setFunction(JZFunctionDefine def);

    JZNodeEvent* startNode();
    const JZNodeEvent* startNode() const;
    JZNode* lastFlowNode();
    void insertFlow(JZNode* after, JZNode* insert_node);
    JZNode* nextFlowNode(const JZNode *node,int flow_id);

    int addNode(JZNode *node);
    void insertNode(JZNode *node);
    void removeNode(int id);
    void removeNodeOnly(int id);

    JZNode *getNode(int id);
    const JZNode *getNode(int id) const;
    JZNodePin *getPin(const JZNodeGemo &gemo);    
    QList<int> nodeList();            

    QList<JZNode*> findNodeByType(int type);

    int addGroup(const JZNodeGroup &group);
    void insertGroup(const JZNodeGroup &group);
    void removeGroup(int id);
    JZNodeGroup *getGroup(int id);
    QList<JZNodeGroup> groupList();
    QList<int> groupNodeList(int group_id);

    void setNodePos(int id,QPointF pos);
    QPointF getNodePos(int id);

    bool canConnect(JZNodeGemo from, JZNodeGemo to,QString &error);
    bool checkConnectNormal(JZNodeGemo from, JZNodeGemo to,QString &error);
    bool checkConnectType(JZNodeGemo from, JZNodeGemo to,QString &error); 
    int parentNode(int id);

    int addConnect(JZNodeGemo from, JZNodeGemo to);
    int addConnectForce(JZNodeGemo from, JZNodeGemo to);     // 不检测类型是否匹配
    bool hasConnect(JZNodeGemo from, JZNodeGemo to);
    void insertConnect(const JZNodeConnect &connect);
    void removeConnect(int id);
    void removeConnectByNode(int node_id, int pinId);
    JZNodeConnect *getConnect(int id);
    const JZNodeConnect *getConnect(int id) const;
    QList<int> getConnectPin(int node_id, int pinId = -1) const;    // pinId = -1 得到节点所有连线
    QList<int> getConnectOut(int node_id, int pinId = -1) const;
    QList<int> getConnectInput(int node_id, int pinId = -1) const;
    QList<JZNodeConnect> connectList() const;

    void addLocalVariable(const JZParamDefine &def);
    void addLocalVariable(const QString &name,QString dataType,const QString &value = QString());            
    void removeLocalVariable(QString name);    
    void clearLocalVariable();
    void setLocalVariable(QString name, const JZParamDefine &def);
    const JZParamDefine *localVariable(QString name);
    QStringList localVariableList(bool hasFunc);

protected:    
    virtual void saveToStream(QDataStream &s) const override;
    virtual bool loadFromStream(QDataStream &s) override;

    ScriptType m_scriptType;
    int m_nodeId;
    QMap<int, JZNode*> m_nodes;        
    QList<JZNodeGroup> m_groups;
    QList<JZNodeConnect> m_connects;    
    mutable JZFunctionDefine m_function;    
        
    QMap<QString, JZParamDefine> m_variables;    
};

bool isFunctionScriptItem(JZProjectItem* item);
bool isFlowScriptItem(JZProjectItem*item);

#endif
