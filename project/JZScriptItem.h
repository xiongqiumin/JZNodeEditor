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

    bool isFunction() const;

    void loadFinish();
    void clear();
    int nextId();

    QByteArray toBuffer();
    bool fromBuffer(const QByteArray &object);

    const JZFunctionDefine &function();
    void setFunction(JZFunctionDefine def);

    int addNode(JZNode *node);
    void insertNode(JZNode *node);
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
    void removeConnectByNode(int node_id, int pinId);
    JZNodeConnect *getConnect(int id);
    QList<int> getConnectPin(int node_id, int pinId = -1);    // pinId = -1 得到节点所有连线
    QList<int> getConnectOut(int node_id, int pinId = -1);
    QList<int> getConnectInput(int node_id, int pinId = -1);
    QList<JZNodeConnect> connectList();    
    
    void addLocalVariable(const JZParamDefine &def);
    void addLocalVariable(const QString &name,int dataType,const QString &value = QString());            
    void removeLocalVariable(QString name);    
    void setLocalVariable(QString name, const JZParamDefine &def);
    const JZParamDefine *localVariable(QString name);
    QStringList localVariableList(bool hasFunc);

protected:    
    int m_nodeId;
    QMap<int, JZNode*> m_nodes;        
    QList<JZNodeGroup> m_groups;
    QList<JZNodeConnect> m_connects;    
    JZFunctionDefine m_function;    

    QMap<int, QPointF> m_nodesPos;
    QMap<QString, JZParamDefine> m_variables;
};

#endif
