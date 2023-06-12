#ifndef JZNODE_SCRIPT_FILE_H_
#define JZNODE_SCRIPT_FILE_H_

#include "JZNode.h"
#include "JZProjectItem.h"
#include "JZNodeFunctionDefine.h"
#include "JZNodeObject.h"

class JZScriptFile : public JZProjectItem
{
public:    
    JZScriptFile(int type);
    virtual ~JZScriptFile();

    void clear();
    const FunctionDefine &function();
    void setFunction(FunctionDefine def);

    void setBindClass(QString bindClass);
    QString bindClass();

    int addNode(JZNodePtr node);
    void insertNode(JZNodePtr node);
    void removeNode(int id);
    JZNode *getNode(int id);
    JZNodePin *getPin(const JZNodeGemo &gemo);
    void setNodePos(int id,QPointF pos);
    QPointF getNodePos(int id); 
    QList<int> nodeList();        

    bool canConnect(JZNodeGemo from, JZNodeGemo to,QString &error);
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
    QString m_bindClass;
    FunctionDefine m_function;    
};

class JZScriptLibraryFile : public JZProjectItem
{
public:
    JZScriptLibraryFile();
    virtual ~JZScriptLibraryFile();

    void addFunction(QString name,QStringList in,QStringList out);
public:

};

class JZScriptClassFile : public JZProjectItem
{
public:
    JZScriptClassFile();
    virtual ~JZScriptClassFile();    

    void init(QString className,QString super = QString());
    void unint();
    bool addMemberVariable(QString name,int dataType,const QVariant &v = QVariant());
    void removeMemberVariable(QString name);
    bool addMemberFunction(FunctionDefine func);
    void removeMemberFunction(QString func);
    JZNodeObjectDefine objectDefine();

protected:
    JZNodeObjectDefine m_define;
};

#endif
