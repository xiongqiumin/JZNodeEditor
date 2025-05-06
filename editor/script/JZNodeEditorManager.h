#ifndef JZNODE_EDITOR_MANAGER_H_
#define JZNODE_EDITOR_MANAGER_H_

#include <QString>
#include <QMap>
#include "JZNode.h"

class JZNodeGraphItem;
class JZNodeParamEditWidget;
class JZNodeParamDisplayWidget;
class JZScriptEnvironment;

typedef JZNodeParamEditWidget *(*CreateParamEditFunc)();
typedef JZNodeParamDisplayWidget *(*CreateParamDisplayFunc)();

typedef JZNodeGraphItem*(*CreateJZNodeGraphItemFunc)(JZNode *node);
typedef QVariant(*CreateParamFunc)(JZScriptEnvironment *env,const QString &value);
typedef QByteArray(*ParamPackFunc)(JZScriptEnvironment *env,const QVariant &value);
typedef QVariant(*ParamUnpackFunc)(JZScriptEnvironment *env,const QByteArray &value);

template <class T>
JZNodeGraphItem *CreateJZNodeGraphItem(JZNode *node) { return new T(node); }

template <class T>
JZNodeParamEditWidget *CreateParamEditWidget() { return new T(); }

template <class T>
JZNodeParamDisplayWidget *CreateParamDisplayWidget() { return new T(); }

//JZNodeParamDelegate
class JZNodeParamDelegate
{
public:
    JZNodeParamDelegate();

    int editType;
    CreateParamEditFunc createEdit;
    CreateParamDisplayFunc createDisplay;
    CreateParamFunc createParam;
    ParamPackFunc pack;
    ParamUnpackFunc unpack;
};

//JZLogicNode
class JZLogicNode
{
public:
    QString path;
    int nodeType;
};

//JZModuleEditor
class JZNodeEditorManager;
class JZModuleEditor
{
public:
    virtual void regist(JZNodeEditorManager *manger) = 0;
    virtual void unregist(JZNodeEditorManager *manger) = 0;
};

class JZNodeEditorManager
{
public:
    static JZNodeEditorManager *instance();

    JZNodeEditorManager();
    ~JZNodeEditorManager();    

    void init();
    void setUserRegist(bool flag);
    void clearUserRegist();    

    void addModule(JZModuleEditor *module); 

    void registLogicNode(int node_type,QString path, CreateJZNodeGraphItemFunc func = nullptr);
    void registLogicNode(JZLogicNode logic);
    QList<JZLogicNode>  logicNodeList();

    void registNodeItemCreator(int node_type, CreateJZNodeGraphItemFunc func);
    bool hasNodeItemCreator(int node_type);
    JZNodeGraphItem *createNodeItem(JZNode* node);

    void registDelegate(int data_type, JZNodeParamDelegate delegate);
    JZNodeParamDelegate *delegate(int data_type);

protected:        
    QMap<int, JZNodeParamDelegate> m_delegateMap;
    QMap<int, CreateJZNodeGraphItemFunc> m_nodeItemMap;
    QList<JZLogicNode> m_logicNode;
    
    bool m_userRegist;    
    QList<int> m_userDelegateList;
    QList<JZModuleEditor*> m_editorModules;
};

void JZNodeEditorInit();

#endif // !JZNODE_EDITOR_MANAGER_H_
