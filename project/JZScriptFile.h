#ifndef JZNODE_SCRIPT_FILE_H_
#define JZNODE_SCRIPT_FILE_H_

#include "JZNode.h"
#include "JZProjectItem.h"

class JZScriptFile : public JZProjectItem
{
public:    
    JZScriptFile(int type,bool dir);
    virtual ~JZScriptFile();

    void clear();

    int addNode(JZNodePtr node);
    void insertNode(JZNodePtr node);
    void removeNode(int id);
    JZNode *getNode(int id);
    JZNodePin *getPin(const JZNodeGemo &gemo);
    void setNodePos(int id,QPointF pos);
    QPointF getNodePos(int id); 
    QList<int> nodeList();    

    int addConnect(JZNodeGemo from, JZNodeGemo to);
    void insertConnect(const JZNodeConnect &connect);
    void removeConnect(int id);
    void removeConnectByNode(int node_id, int prop_id, int type);
    JZNodeConnect *getConnect(int id);
    QList<int> getConnectId(int node_id, int propId = -1, int flag = Prop_All);
    QList<JZNodeConnect> connectList();

    void saveToStream(QDataStream &s);
    void loadFromStream(QDataStream &s);

protected:
    int m_nodeId;
    QMap<int, JZNodePtr> m_nodes;    
    QMap<int, QPointF> m_nodesPos;   
    QList<JZNodeConnect> m_connects;    
};









#endif
