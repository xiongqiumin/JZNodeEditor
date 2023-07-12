#ifndef JZNODE_OBJECT_H_
#define JZNODE_OBJECT_H_

#include "JZEvent.h"
#include <QMap>
#include <QWidget>
#include "JZNodeFunctionDefine.h"

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
    QStringList paramList();
    JZParamDefine *param(QString name);

    void addFunction(FunctionDefine def);
    void removeFunction(QString function);    
    void addSingle(FunctionDefine def);
    void removeSingle(QString function);

    QString fullname() const;
    const FunctionDefine *function(QString function);
    const SingleDefine *single(QString function);
    JZNodeObjectDefine *super();    
    bool isInherits(int type) const;
    bool isInherits(const QString &name) const;
    bool isCopyable() const;

    int id;
    QString nameSpace;
    QString className;            
    QString superName;
    QMap<QString,JZParamDefine> params;
    QStringList functions;
    QList<SingleDefine> singles;
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
    
    bool isInherits(int type) const;
    bool isInherits(const QString &name) const;
    bool isCopyable() const;
    bool isCObject() const;
    bool isString() const;
    const QString &className() const;    

    bool hasParam(QString name) const;
    QVariant param(QString name) const;    
    void setParam(QString name,QVariant value);
    QVariant *paramRef(QString name);
    const FunctionDefine *function(QString function);

    void setCObject(void *cobj,bool owner);    
    void updateCRefs();
    void connectSingles(int type);

    JZNodeObjectDefine *define;
    QString name;    
    QMap<QString,QVariant> params;    
    void *cobj;
    bool cowner;

protected:
    Q_DISABLE_COPY(JZNodeObject);    

    bool getParamRef(const QString &name,QVariant* &ref,QString &error);
};
typedef QSharedPointer<JZNodeObject> JZNodeObjectPtr;
Q_DECLARE_METATYPE(JZNodeObject*)
Q_DECLARE_METATYPE(JZNodeObjectPtr)

bool isJZObjectDelcare(const QVariant &v);
bool isJZObject(const QVariant &v);
JZNodeObject* toJZObject(const QVariant &v);
int JZClassId(const QString &name);
QString JZClassName(int id);
void JZObjectEvent(JZEvent *event);

class JZNodeObjectManager
{
public:
    static JZNodeObjectManager *instance();
    JZNodeObjectManager();   
    ~JZNodeObjectManager(); 

    void init();
    JZNodeObjectDefine *meta(QString className);
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
JZNodeObjectPtr JZObjectRefrence(T *ptr)
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
