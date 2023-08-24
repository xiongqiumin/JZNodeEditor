#ifndef JZNODE_OBJECT_H_
#define JZNODE_OBJECT_H_

#include "JZEvent.h"
#include <QMap>
#include <QWidget>
#include "JZNodeFunctionDefine.h"
#include "JZNodeEnum.h"

class CMeta
{
public:
    CMeta();

    bool isAbstract;
    bool isCopyable;
    void* (*create)();
    void (*copy)(void *dst,void *src);
    void (*destory)(void *);
    void (*addEventFilter)(void *obj,int event_type);
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
    QStringList paramList(bool hasParent) const;
    const JZParamDefine *param(QString name) const;

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
    
    const JZNodeObjectDefine *super() const;
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

class JZNodeObjectManager;
class JZNodeObject
{
public:    
    JZNodeObject(JZNodeObjectDefine *def);
    ~JZNodeObject();
    
    bool isNull() const;
    void setNull();
    
    bool isInherits(int type) const;
    bool isInherits(const QString &name) const;
    bool isCopyable() const;
    bool isCObject() const;
    const QString &className() const;
    int type() const;
    const JZNodeObjectDefine *meta() const;

    bool hasParam(const QString &name) const;
    QVariant param(const QString &name) const;
    void setParam(const QString &name,QVariant value);
    QVariant *paramRef(const QString &name);
    QStringList paramList() const;

    const FunctionDefine *function(const QString &function) const;

    void setCObject(void *cobj,bool owner);    
    void *cobj() const;
    void setCowner(bool owner);

    void updateWidgetParam();
    void connectSingles(int type);    

protected:
    Q_DISABLE_COPY(JZNodeObject);    

    friend JZNodeObjectManager;

    bool getParamRef(const QString &name,QVariant* &ref,QString &error);

    bool m_isNull;
    JZNodeObjectDefine *m_define;
    QString m_name;
    QMap<QString, QVariant> m_params;
    void *m_cobj;
    bool m_cowner;
};
typedef QSharedPointer<JZNodeObject> JZNodeObjectPtr;
Q_DECLARE_METATYPE(JZNodeObject*)
Q_DECLARE_METATYPE(JZNodeObjectPtr)

bool isJZObject(const QVariant &v);
JZNodeObject* toJZObject(const QVariant &v); //对于空 object 返回 nullptr
JZNodeObject* toJZObjectOriginal(const QVariant &v); //对于空 object 也返会指针

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
    bool isInherits(QString class_name, QString super_name);
    bool isInherits(int class_name,int super_name);
    QStringList getClassList();

    JZNodeEnumDefine *enumMeta(int type_id);
    JZNodeEnumDefine *enumMeta(QString enumName);
    QString getEnumName(int type_id);
    int getEnumId(QString enumName);
    QStringList getEnumList();

    int getClassIdByTypeid(QString name);
    
    int getClassIdByQObject(QString name);
    void declareQObject(int id,QString name);

    int delcare(QString name, QString type_id, int id = -1);
    int regist(JZNodeObjectDefine define);    
    int registCClass(JZNodeObjectDefine define,QString type_id);
    void replace(JZNodeObjectDefine define);
    
    int registEnum(JZNodeEnumDefine enumName);
    int registCEnum(JZNodeEnumDefine enumName, QString ctype_id);
    void unregist(int id);    
    void clearUserReigst();    
    
    JZNodeObjectPtr create(int type_id);
    JZNodeObjectPtr createEmpty(int type_id);
    JZNodeObjectPtr createCClass(QString ctype_id);
    JZNodeObjectPtr createCClassRefrence(QString ctype_id,void *cobj,bool owner);
    JZNodeObjectPtr clone(JZNodeObject *other);

protected:
    void create(const JZNodeObjectDefine *define,JZNodeObject *obj);
    void copy(JZNodeObject *src,JZNodeObject *dst);
    void initFunctions();
    
    QMap<QString, int> m_typeidMetas;
    
    QMap<int,JZNodeEnumDefine> m_enums;
    int m_enumId;
    
    QMap<int, QSharedPointer<JZNodeObjectDefine>> m_metas;
    QMap<QString,int> m_qobjectId;
    int m_objectId;    
};

template<class T>
JZNodeObjectPtr JZObjectCreate()
{
    return JZNodeObjectManager::instance()->createCClass(typeid(T).name());
}

template<class T>
JZNodeObjectPtr JZObjectCreate(T ptr,bool owner = true)
{
    static_assert(std::is_pointer<T>(), "only support class pointer");
    QString c_typeid = typeid(std::remove_pointer_t<T>).name();
    return JZNodeObjectManager::instance()->createCClassRefrence(c_typeid, ptr, owner);
}

#endif
