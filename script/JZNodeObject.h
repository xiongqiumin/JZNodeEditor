#ifndef JZNODE_OBJECT_H_
#define JZNODE_OBJECT_H_

#include "JZNodeFunctionDefine.h"
#include <QMap>
#include <QWidget>

class CMeta
{
public:
    CMeta();

    bool isAbstract;
    bool isCopyable;
    void* (*create)();
    void (*copy)(void *,void *);
    void (*destory)(void *);
};

class JZNodeObjectDefine
{
public:
    JZNodeObjectDefine();

    void addParam(JZParamDefine def);
    void removeParam(QString name);
    void addFunction(FunctionDefine def);
    void removeFunction(QString function);

    FunctionDefine *function(QString function);    
    bool isCopyable() const;

    int id;
    QString className;            
    QString superName;
    QMap<QString,JZParamDefine> params;
    QMap<QString,FunctionDefine> functions;
    bool isCObject;
    CMeta cMeta;
};
QDataStream &operator<<(QDataStream &s, const JZNodeObjectDefine &param);
QDataStream &operator>>(QDataStream &s, JZNodeObjectDefine &param);

class JZNodeObject
{
public:    
    JZNodeObject(JZNodeObjectDefine *def);
    ~JZNodeObject();
    
    bool isCopyable() const;
    bool isCObject() const;
    bool isString() const;
    const QString &className() const;
    const QString fullName() const;

    QVariant param(QString name) const;
    void setParam(QString name,QVariant value);
    FunctionDefine *function(QString function);           

    void setCObject(void *cobj,bool owner);
    void addWatch(QString name);

    JZNodeObjectDefine *define;
    QString name;
    JZNodeObject *parent;
    QMap<QString,QVariant> params;
    QStringList watchList;
    void *cobj;
    bool cowner;

protected:
    Q_DISABLE_COPY(JZNodeObject);    
};
typedef QSharedPointer<JZNodeObject> JZNodeObjectPtr;
Q_DECLARE_METATYPE(JZNodeObject*)
Q_DECLARE_METATYPE(JZNodeObjectPtr)

bool isJZObjectDelcare(const QVariant &v);
bool isJZObject(const QVariant &v);
JZNodeObject* toJZObject(const QVariant &v);
void JZObjectValueChanged(const QString &name);
int JZClassId(const QString &name);
QString JZClassName(int id);

class JZNodeObjectManager
{
public:
    static JZNodeObjectManager *instance();
    JZNodeObjectManager();   
    ~JZNodeObjectManager(); 

    void init();
    JZNodeObjectDefine *meta(int type_id);
    QString getClassName(int type_id);
    int getClassId(QString class_name);
    bool isInherits(int class_name,int super_name);
    
    int getClassIdByTypeid(QString name);
    int getClassIdByCClassName(QString name);

    int regist(JZNodeObjectDefine define);
    void replace(JZNodeObjectDefine define);
    void registCClass(JZNodeObjectDefine define,QString type_id);
    void unregist(int id);
    void declareCClass(QString name,int id);
    void clearUserReigst();

    JZNodeObjectPtr create(int type_id);
    JZNodeObjectPtr createCClass(QString ctype_id);
    JZNodeObjectPtr createCClassRefrence(QString ctype_id,void *cobj);
    JZNodeObjectPtr clone(JZNodeObjectPtr other);

protected:
    void create(JZNodeObjectDefine *define,JZNodeObject *obj);
    void copy(JZNodeObject *src,JZNodeObject *dst);
    void initCore();
    void initWidgets();
    void initFunctions();

    QMap<int,QSharedPointer<JZNodeObjectDefine>> m_metas;
    QMap<QString,int> m_typeidMetas;
    QMap<QString,int> m_cClassId;
    int m_objectId;
};

template<class T>
JZNodeObjectPtr JZObjectCreate()
{
    return JZNodeObjectManager::instance()->createCClass(typeid(T).name());
}

template<class T>
JZNodeObjectPtr JZObjectRefrence(void *ptr)
{
    return JZNodeObjectManager::instance()->createCClassRefrence(typeid(T).name(),ptr);
}

class TestWindow : public QWidget
{
    Q_OBJECT

public:
    TestWindow();
    ~TestWindow();
};

#endif
