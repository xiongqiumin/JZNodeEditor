#ifndef JZNODE_OBJECT_H_
#define JZNODE_OBJECT_H_

#include "JZNodeFunctionDefine.h"
#include <QMap>
#include <QDebug>

class CBaseFunction
{
public:
    void* (*create)();
    void (*copy)(void *,void *);
    void (*destory)(void *);
};

class JZNodeObjectDefine
{
public:
    JZNodeObjectDefine();
    FunctionDefine *function(QString function);
    
    QString className;
    bool isCObject;    
    JZNodeObjectDefine *super;
    QMap<QString,QVariant> params;
    QMap<QString,FunctionDefine> functions;   
    CBaseFunction cMeta;    
};

class JZNodeObjectDelcare
{
public:
    QString className;
};

class JZNodeObject
{
public:
    JZNodeObject(JZNodeObjectDefine *def);
    ~JZNodeObject();
    
    bool isCObject() const;
    bool isString() const;
    const QString &className() const;

    QVariant param(QString name) const;
    void setParam(QString name,QVariant value);
    FunctionDefine *function(QString function);

    JZNodeObjectDefine *define;  
    QMap<QString,QVariant> params;
    void *cobj;

protected:
    Q_DISABLE_COPY(JZNodeObject);
};
typedef QSharedPointer<JZNodeObject> JZNodeObjectPtr;
Q_DECLARE_METATYPE(JZNodeObjectDelcare)
Q_DECLARE_METATYPE(JZNodeObjectPtr)

QDebug operator<<(QDebug dbg, const JZNodeObjectPtr &obj);

class JZNodeObjectManager
{
public:
    static JZNodeObjectManager *instance();
    JZNodeObjectManager();   
    ~JZNodeObjectManager(); 

    void init();
    JZNodeObjectDefine *meta(QString name);
    QString getTypeid(QString name);

    void regist(JZNodeObjectDefine define,QString super = QString());
    void registCClass(JZNodeObjectDefine define,QString type_id,QString super = QString());
    void unregist(QString name);           
    
    JZNodeObjectPtr create(QString name);
    JZNodeObjectPtr createString(QString text);
    JZNodeObjectPtr createCClass(QString type_id);
    JZNodeObjectPtr clone(JZNodeObjectPtr other);       

protected:
    void create(JZNodeObjectDefine *define,JZNodeObject *obj);
    void copy(JZNodeObject *src,JZNodeObject *dst);
    void initWidgets();

    QMap<QString,JZNodeObjectDefine*> m_metas;        
    QMap<QString,QString> m_typeidMetas;
};

#endif
