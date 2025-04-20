#ifndef JZNODE_EDITOR_MANAGER_H_
#define JZNODE_EDITOR_MANAGER_H_

#include <QString>
#include <QMap>

class JZNodeGraphItem;
class JZNodeParamEditWidget;
class JZNodeParamDisplayWidget;
class JZScriptEnvironment;

typedef JZNodeParamEditWidget *(*CreateParamEditFunc)();
typedef JZNodeParamDisplayWidget *(*CreateParamDisplayFunc)();

typedef JZNodeGraphItem*(*CreateJZNodeGraphItemFunc)();
typedef QVariant(*CreateParamFunc)(JZScriptEnvironment *env,const QString &value);
typedef QByteArray(*ParamPackFunc)(JZScriptEnvironment *env,const QVariant &value);
typedef QVariant(*ParamUnpackFunc)(JZScriptEnvironment *env,const QByteArray &value);

template <class T>
JZNodeGraphItem *CreateJZNodeGraphItem() { return new T(); }

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

class JZNodeEditorManager
{
public:
    static JZNodeEditorManager *instance();

    JZNodeEditorManager();
    ~JZNodeEditorManager();    

    void init();
    void setUserRegist(bool flag);
    void clearUserRegist();    

    void registLogicNode(JZLogicNode logic);
    QList<JZLogicNode>  logicNodeList();

    void registNodeItemCreator(int node_type, CreateJZNodeGraphItemFunc func);
    bool hasNodeItemCreator(int node_type);
    CreateJZNodeGraphItemFunc nodeItemCreator(int node_type);

    void registDelegate(int data_type, JZNodeParamDelegate delegate);
    JZNodeParamDelegate *delegate(int data_type);

protected:        
    QMap<int, JZNodeParamDelegate> m_delegateMap;
    QMap<int, CreateJZNodeGraphItemFunc> m_nodeItemMap;
    QList<JZLogicNode> m_logicNode;
    
    bool m_userRegist;    
    QList<int> m_userDelegateList;
};

void JZNodeEditorInit();

#endif // !JZNODE_EDITOR_MANAGER_H_
