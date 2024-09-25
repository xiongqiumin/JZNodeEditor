#ifndef JZNODE_EDITOR_MANAGER_H_
#define JZNODE_EDITOR_MANAGER_H_

#include <QString>
#include <QMap>
#include "JZNodeCoreDefine.h"

class JZNodeParamEditWidget;
class JZNodeParamDisplayWidget;
class JZScriptEnvironment;

typedef JZNodeParamEditWidget *(*CreateParamEditFunc)();
typedef JZNodeParamDisplayWidget *(*CreateParamDisplayFunc)();

typedef QVariant(*CreateParamFunc)(JZScriptEnvironment *env,const QString &value);
typedef QByteArray(*ParamPackFunc)(JZScriptEnvironment *env,const QVariant &value);
typedef QVariant(*ParamUnpackFunc)(JZScriptEnvironment *env,const QByteArray &value);

template <class T>
JZNodeParamEditWidget *CreateParamEditWidget() { return new T(); }

template <class T>
JZNodeParamDisplayWidget *CreateParamDisplayWidget() { return new T(); }

//JZNodeParamDelegate
class JZCORE_EXPORT JZNodeParamDelegate
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

class JZCORE_EXPORT JZNodeEditorManager
{
public:
    JZNodeEditorManager(JZScriptEnvironment *env);
    ~JZNodeEditorManager();

    JZScriptEnvironment *env();

    void init();
    void setUserRegist(bool flag);
    void clearUserRegist();

    void registCustomFunctionNode(QString function,int node_type);
    void unregistCustomFunctionNode(QString function);
    int customFunctionNode(QString function);

    void registDelegate(int data_type, JZNodeParamDelegate delegate);
    JZNodeParamDelegate *delegate(int data_type);

protected:    
    QMap<QString, int> m_functionMap;    
    QMap<int, JZNodeParamDelegate> m_delegateMap;
    
    bool m_userRegist;
    QStringList m_userFunctionList;
    QList<int> m_userDelegateList;

    JZScriptEnvironment *m_env;
};

#endif // !JZNODE_EDITOR_MANAGER_H_
