#ifndef JZNODE_SCRIPT_FILE_H_
#define JZNODE_SCRIPT_FILE_H_

#include "JZNode.h"
#include "JZProjectItem.h"
#include "JZNodeFunctionDefine.h"
#include "JZNodeObject.h"

class JZScriptClassFile;
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
    QList<int> nodeList();        

    void setNodePos(int id,QPointF pos);
    QPointF getNodePos(int id);

    bool canConnect(JZNodeGemo from, JZNodeGemo to,QString &error);
    int addConnect(JZNodeGemo from, JZNodeGemo to);
    bool hasConnect(JZNodeGemo from, JZNodeGemo to);
    void insertConnect(const JZNodeConnect &connect);
    void removeConnect(int id);
    void removeConnectByNode(int node_id, int prop_id);
    JZNodeConnect *getConnect(int id);
    QList<int> getConnectId(int node_id, int propId = -1);    // propId = -1 得到节点所有连线
    QList<int> getConnectOut(int node_id, int propId = -1);
    QList<int> getConnectInput(int node_id, int propId = -1);
    QList<JZNodeConnect> connectList();

    void saveToStream(QDataStream &s);
    void loadFromStream(QDataStream &s);    

    JZParamDefine *localVariableInfo(const QString &name);
    void addLocalVariable(QString name, int type, QVariant v = QVariant());
    void removeLocalVariable(QString name);
    void renameLocalVariable(QString oldName, QString newName);
    void setLocalVariableType(QString name,int type);
    QStringList localVariableList();

protected:
    int m_nodeId;
    QMap<int, JZNodePtr> m_nodes;        
    QList<JZNodeConnect> m_connects;    
    FunctionDefine m_function;
    QMap<QString, JZParamDefine> m_variables;

    QMap<int, QPointF> m_nodesPos;
};

class JZScriptLibraryFile : public JZProjectItem
{
public:
    JZScriptLibraryFile();
    virtual ~JZScriptLibraryFile();

    void addFunction(QString name,QStringList in,QStringList out);
public:

};

class JZParamFile;
class JZScriptClassFile : public JZProjectItem
{
public:
    JZScriptClassFile();
    virtual ~JZScriptClassFile();    

    virtual void saveToStream(QDataStream &s);
    virtual void loadFromStream(QDataStream &s);

    void init(QString className,QString super = QString());
    void uninit();
    void reinit();

    QString className() const;
    int classType() const;

    bool addMemberVariable(QString name,int dataType,const QVariant &v = QVariant());
    void removeMemberVariable(QString name);
    bool addMemberFunction(FunctionDefine func);
    void removeMemberFunction(QString func);
    JZNodeObjectDefine objectDefine();

protected:   
    JZParamFile *getParamFile();
    QString m_className;
    QString m_super;
    int m_classId;
};

#endif
