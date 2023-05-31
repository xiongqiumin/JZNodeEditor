#ifndef JZNODE_SCRIPT_FILE_H_
#define JZNODE_SCRIPT_FILE_H_

#include "JZNode.h"
#include "JZProjectItem.h"
#include "JZNodeFunctionDefine.h"

class JZScriptFile : public JZProjectItem
{
public:    
    JZScriptFile(int type);
    virtual ~JZScriptFile();

    void clear();
    const FunctionDefine &function();
    void setFunction(FunctionDefine def);

    int addNode(JZNodePtr node);
    void insertNode(JZNodePtr node);
    void removeNode(int id);
    JZNode *getNode(int id);
    JZNodePin *getPin(const JZNodeGemo &gemo);
    void setNodePos(int id,QPointF pos);
    QPointF getNodePos(int id); 
    QList<int> nodeList();    

    bool canConnect(JZNodeGemo from, JZNodeGemo to);
    int addConnect(JZNodeGemo from, JZNodeGemo to);
    bool hasConnect(JZNodeGemo from, JZNodeGemo to);
    void insertConnect(const JZNodeConnect &connect);
    void removeConnect(int id);
    void removeConnectByNode(int node_id, int prop_id);
    JZNodeConnect *getConnect(int id);
    QList<int> getConnectId(int node_id, int propId = -1); // propId = -1 得到节点所有连线
    QList<JZNodeConnect> connectList();

    void saveToStream(QDataStream &s);
    void loadFromStream(QDataStream &s);

protected:
    int m_nodeId;
    QMap<int, JZNodePtr> m_nodes;    
    QMap<int, QPointF> m_nodesPos;   
    QList<JZNodeConnect> m_connects;    
    FunctionDefine m_function;
};


class JZScriptFunctionFile : public JZProjectItem
{
public:
    JZScriptFunctionFile();
    virtual ~JZScriptFunctionFile();

    void addFunction(QString name,QStringList in,QStringList out);
public:

};

class JZScriptClassFile : public JZProjectItem
{
public:
    JZScriptClassFile();
    virtual ~JZScriptClassFile();

    void addFunction(QStringList in,QStringList out);
    void addParam();
    void removeParam();

public:
    
};

#endif
