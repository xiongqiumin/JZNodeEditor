#ifndef JZNODE_PARAM_DELEGATE_H_
#define JZNODE_PARAM_DELEGATE_H_

#include "JZNodeParamEditWidget.h"
#include "JZNodeParamDisplayWidget.h"

typedef JZNodeParamEditWidget *(*CreateParamEditFunc)();
typedef QVariant (*CreateParamFunc)(const QString &value);
typedef JZNodeParamDisplayWidget *(*CreateParamDisplayFunc)();
typedef QByteArray (*ParamPackFunc)(const QVariant &value);
typedef QVariant (*ParamUnpackFunc)(const QByteArray &value);

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

//JZNodeParamDelegateManager
class JZCORE_EXPORT JZNodeParamDelegateManager
{
public:
    static JZNodeParamDelegateManager *instance();

    void registDelegate(int data_type,JZNodeParamDelegate delegate);    
    bool hasDelegate(int data_type);    

    JZNodeParamDelegate *delegate(int data_type);    
    CreateParamEditFunc editFunction(int data_type);
    CreateParamDisplayFunc displayFunction(int data_type);

protected:    
    JZNodeParamDelegateManager();
    ~JZNodeParamDelegateManager();

    QMap<int, JZNodeParamDelegate> m_delegateMap;    
};

void InitParamDelegate();

#endif