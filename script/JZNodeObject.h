#ifndef JZNODE_OBJECT_H_
#define JZNODE_OBJECT_H_

#include "JZNodeFunctionDefine.h"
#include <QMap>

class JZNodeObjectDefine
{
public:
    JZNodeObjectDefine();

    JZNodeObjectDefine *super;
    QMap<QString,QVariant> params;
    QMap<QString,FunctionDefine> functions;
};

class JZNodeObject
{
public:
    JZNodeObject();
    ~JZNodeObject();
    
    QVariant param(QString name);
    void setParam(QString name,QVariant value);
    FunctionDefine *function(QString function);

    JZNodeObjectDefine *m_define;
    void *m_pointer;
};

class JZNodeObjectManager
{
public:
    static JZNodeObjectManager *instance();

    JZNodeObjectManager();
    void registWidgets();

    void regist(QString name,QString super,JZNodeObjectDefine define);
    void registQObject(QString className,JZNodeObjectDefine define);
    JZNodeObjectDefine *meta(QString name);
    JZNodeObject create(QString name);

protected:
    QMap<QString,JZNodeObjectDefine> m_metas;        
};

#endif
