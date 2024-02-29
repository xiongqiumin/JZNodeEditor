#ifndef JZNODE_SCRIPT_FILE_H_
#define JZNODE_SCRIPT_FILE_H_

#include "JZNode.h"
#include "JZProjectItem.h"
#include "JZNodeFunctionDefine.h"
#include "JZNodeObject.h"

class JZScriptClassItem;
class JZScriptItem : public JZProjectItem
{
public:    
    JZScriptItem(int type);
    virtual ~JZScriptItem();

    void loadFinish();
    void clear();
    int nextId();

    QByteArray toBuffer();
    bool fromBuffer(const QByteArray &object);

    const FunctionDefine &function();
    void setFunction(FunctionDefine def);

    int addNode(JZNodePtr node);
    void insertNode(JZNodePtr node);
    void removeNode(int id);
    JZNode *getNode(int id);
    JZNodePin *getPin(const JZNodeGemo &gemo);    
    QList<int> nodeList();            

    int addGroup(const JZNodeGroup &group);
    void insertGroup(const JZNodeGroup &group);
    void removeGroup(int id);
    JZNodeGroup *getGroup(int id);
    QList<JZNodeGroup> groupList();
    QList<int> groupNodeList(int group_id);

    void setNodePos(int id,QPointF pos);
    QPointF getNodePos(int id);

    bool canConnect(JZNodeGemo from, JZNodeGemo to,QString &error); 
    int parentNode(int id);

    int addConnect(JZNodeGemo from, JZNodeGemo to);
    bool hasConnect(JZNodeGemo from, JZNodeGemo to);
    void insertConnect(const JZNodeConnect &connect);
    void removeConnect(int id);
    void removeConnectByNode(int node_id, int prop_id);
    JZNodeConnect *getConnect(int id);
    QList<int> getConnectId(int node_id, int pinId = -1);    // pinId = -1 得到节点所有连线
    QList<int> getConnectOut(int node_id, int pinId = -1);
    QList<int> getConnectInput(int node_id, int pinId = -1);
    QList<JZNodeConnect> connectList();    
    
    void addLocalVariable(const JZParamDefine &def);            
    void removeLocalVariable(QString name);    
    const JZParamDefine *localVariable(QString name);
    QStringList localVariableList(bool hasFunc);

protected:    
    int m_nodeId;
    QMap<int, JZNodePtr> m_nodes;        
    QList<JZNodeGroup> m_groups;
    QList<JZNodeConnect> m_connects;    
    FunctionDefine m_function;    

    QMap<int, QPointF> m_nodesPos;
    QMap<QString, JZParamDefine> m_variables;
};

#endif
