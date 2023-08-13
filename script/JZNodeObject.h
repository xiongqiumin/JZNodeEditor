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
    void (*addEventFilter)(void*,int);
};

class JZZNodeEnumDefine
{
public:
    QString name;
    QStringList enums;
};


class JZZNodeFlagDefine
{
public:
    QString name;
    QStringList enums;
};

class JZNodeObjectDefine
{
public:
    JZNodeObjectDefine();

    void addParam(JZParamDefine def);
    void removeParam(QString name);
    QStringList paramList() const;
    JZParamDefine *param(QString name);

    void addFunction(FunctionDefine def);
    void removeFunction(QString function);    
    int indexOfFunction(QString function) const;

    void addSingle(FunctionDefine def);
    void removeSingle(QString function);

    QString fullname() const;

    const FunctionDefine *function(QString function) const;
    
    QStringList singleList() const;
    const SingleDefine *single(QString function) const;
    
    QStringList eventList() const;
    const EventDefine *event(QString function) const;
    
    JZNodeObjectDefine *super() const;
    bool isInherits(int type) const;
    bool isInherits(const QString &name) const;
    bool isCopyable() const;

    int id;
    QString nameSpace;
    QString className;            
    QString superName;
    QMap<QString,JZParamDefine> params;
    QList<FunctionDefine> functions;
    QList<SingleDefine> singles;
    QList<EventDefine> events;
    bool isCObject;    
    bool isUiWidget;
    QString widgteXml;
    CMeta cMeta;
};
QDataStream &operator<<(QDataStream &s, const JZNodeObjectDefine &param);
QDataStream &operator>>(QDataStream &s, JZNodeObjectDefine &param);

class JZObjectNull
{
public:

};
QDataStream &operator<<(QDataStream &s, const JZObjectNull &param);
QDataStream &operator>>(QDataStream &s, JZObjectNull &param);
Q_DECLARE_METATYPE(JZObjectNull)

class JZNodeObject
{
public:    
    JZNodeObject(JZNodeObjectDefine *def);
    ~JZNodeObject();
    
    bool isInherits(int type) const;
    bool isInherits(const QString &name) const;
    bool isCopyable() const;
    bool isCObject() const;
    const QString &className() const;
    int type() const;
    const JZNodeObjectDefine *meta() const;

    bool hasParam(QString name) const;
    QVariant param(QString name) const;    
    void setParam(QString name,QVariant value);
    QVariant *paramRef(QString name);
    const FunctionDefine *function(QString function);

    void setCObject(void *cobj,bool owner);    
    void updateWidgetParam();
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

bool isJZObject(const QVariant &v);
JZNodeObject* toJZObject(const QVariant &v);

int JZClassId(const QString &name);
QString JZClassName(int id);
void JZObjectEvent(JZEvent *event);
void JZScriptLog(const QString &name);

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
    bool isInherits(QString class_name, QString super_name);
    bool isInherits(int class_name,int super_name);
    QStringList getClassList();

    int getClassIdByTypeid(QString name);
    
    int getClassIdByQObject(QString name);
    void declareQObject(int id,QString name);

    int delcare(QString name, QString type_id, int id = -1);
    int regist(JZNodeObjectDefine define);
    void replace(JZNodeObjectDefine define);
    int registCClass(JZNodeObjectDefine define,QString type_id);
    void registEnum(QString enumName, QString ctype_id);
    void unregist(int id);    
    void clearUserReigst();    
    
    JZNodeObjectPtr create(int type_id);
    JZNodeObjectPtr createCClass(QString ctype_id);
    JZNodeObjectPtr createCClassRefrence(QString ctype_id,void *cobj);
    JZNodeObjectPtr clone(JZNodeObjectPtr other);

protected:
    void create(JZNodeObjectDefine *define,JZNodeObject *obj);
    void copy(JZNodeObject *src,JZNodeObject *dst);
    void initFunctions();

    QMap<int,QSharedPointer<JZNodeObjectDefine>> m_metas;
    QMap<QString,int> m_typeidMetas;
    QMap<QString,int> m_qobjectId;
    int m_objectId;
    int m_enumId;
};

template<class T>
JZNodeObjectPtr JZObjectCreate()
{
    return JZNodeObjectManager::instance()->createCClass(typeid(T).name());
}

template<class T>
JZNodeObjectPtr JZObjectRefrence(T ptr)
{
    static_assert(std::is_pointer<T>(), "only support class pointer");
    QString c_typeid = typeid(std::remove_pointer_t<T>).name();
    return JZNodeObjectManager::instance()->createCClassRefrence(c_typeid,ptr);
}

#endif
