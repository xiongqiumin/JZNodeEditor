#ifndef JZNODE_OBJECT_H_
#define JZNODE_OBJECT_H_

#include <QMap>
#include <QWidget>
#include <functional>
#include "JZNodeFunctionDefine.h"
#include "JZNodeEnum.h"

class CMeta
{
public:
    CMeta();

    bool isAbstract;
    bool isCopyable;
    bool isCompare;
    std::function<void*()> create;
    std::function<void(void*,void*)> copy;  //src->dst
    std::function<bool(void*,void*)> equal;
    std::function<void(void*)> destory;
};

/* 对于虚函数，this 参数应当为基类指针
*/
class JZNodeObjectManager;
class JZNodeObjectDefine
{
public:
    JZNodeObjectDefine();

    QString fullname() const;
    int baseId() const;    

    void addParam(const JZParamDefine &def);
    void removeParam(const QString &name);
    QStringList paramList(bool hasParent) const;
    const JZParamDefine *param(const QString &name) const;

    JZFunctionDefine initStaticFunction(QString function) const;
    JZFunctionDefine initMemberFunction(QString function) const;
    JZFunctionDefine initVirtualFunction(QString function) const;
    JZFunctionDefine initSlotFunction(QString param,QString single) const;

    void addFunction(const JZFunctionDefine &def);
    void removeFunction(const QString &function);    
    int indexOfFunction(const QString &function) const;
    bool check(QString &error) const;
    const JZFunctionDefine *function(const QString &function) const;
    QStringList functionList() const;
    QStringList virtualFunctionList() const;
    
    const JZSignalDefine *signal(const QString &function) const;
    QStringList signalList() const;

    const JZFunctionDefine *slot(const QString &function) const;
    QStringList slotList() const;
    
    const JZNodeObjectDefine *super() const;
    const JZNodeObjectDefine *cSuper() const; //最近的c类
    bool isInherits(int type) const;
    bool isInherits(const QString &name) const;
    bool isAbstract() const;
    bool isCopyable() const;
    bool isValueType() const;

    int id;
    QString nameSpace;
    QString className;            
    QString superName;
    bool valueType;
    QMap<QString,JZParamDefine> params;
    QMap<QString,JZCParamDefine> cparams;
    QList<JZFunctionDefine> functions;
    QList<JZSignalDefine> singles;
    QStringList enums;

    bool isUiWidget;
    QString widgetXml;
    QList<JZParamDefine> widgetParams;
    QMap<QString,JZNodeParamBind> widgetBind;

    bool isCObject;    
    CMeta cMeta;
    JZNodeObjectManager *manager;
};
QDataStream &operator<<(QDataStream &s, const JZNodeObjectDefine &param);
QDataStream &operator>>(QDataStream &s, JZNodeObjectDefine &param);

class JZNodeCObjectDelcare
{
public:
    JZNodeCObjectDelcare();
    
    QString className;
    int id;
};
QDataStream &operator<<(QDataStream &s, const JZNodeCObjectDelcare &param);
QDataStream &operator>>(QDataStream &s, JZNodeCObjectDelcare &param);

class JZNodeObjectManager;
class JZScriptEnvironment;
class JZNodeObject : public QObject
{
    Q_OBJECT

public:        
    bool isInherits(int type) const;
    bool isInherits(const QString &name) const;
    bool isCopyable() const;
    bool isCObject() const;
    bool isValueType() const;
    const QString &className() const;
    int type() const;
    int baseType() const;
    const JZNodeObjectDefine *meta() const;

    bool hasParam(const QString &name) const;
    QVariant param(const QString &name) const;
    void setParam(const QString &name,const QVariant &value);    
    QStringList paramList() const;
    QVariantPtr* paramRef(const QString& name);

    const JZFunctionDefine *function(const QString &function) const;
    QStringList functionList() const;
    
    const JZSignalDefine *signal(QString function) const;
    QStringList signalList() const;

    void singleConnect(QString sig, QString slot);
    void singleDisconnect(QString sig, QString slot);

    void singleConnect(QString sig,JZNodeObject *recv,QString slot);
    void singleDisconnect(QString sig,JZNodeObject *recv,QString slot);
    void singleEmit(QString sig,const QVariantList &params);

    void *cobj() const;
    void setCObject(void *cobj,bool owner);        
    void setCOwner(bool owner);

    void updateUiWidget(QWidget *widget);
    void autoConnect();
    void autoBind();
    void autoInit();

    const JZNodeObjectManager *manager() const;

signals:
    void sigTrigger(QString slot_function,const QVariantList &params);
    void sigValueChanged(const QString &name);

public slots:
    void onSigTrigger(QString slot_function,const QVariantList &params);
    void onDestory(QObject *obj);
    void onRecvDestory(QObject *obj);

protected:        
    Q_DISABLE_COPY(JZNodeObject);  
    friend JZNodeObjectManager;

    struct ConnectInfo
    {
        QString single;
        JZNodeObject *recv;
        QString slot;
    };         

    JZNodeObject(const JZNodeObjectDefine *def);
    ~JZNodeObject();
   
    const JZCParamDefine *cparam(const QString &name) const;
    int singleConnectCount(JZNodeObject *recv)  const;
    void clearCObj();

    const JZNodeObjectDefine *m_define;
    void *m_cobj;
    bool m_cobjOwner;
    QMap<QString, QVariantPtr> m_params;
    QList<ConnectInfo> m_connectList;
};

//JZNodeObjectData
class JZNodeObjectData
{
public:
    JZNodeObjectData();
    ~JZNodeObjectData();

    bool isOwner;
    JZNodeObject *object;
};

/*
JZNodeObjectPointer sharedPointer 和 weakPointer 做在一起的
代表值类型
isOwner 代表是否所有object, QWidget 回调时存在不需要管理的情况
会根据QObject是否有父类，决定是否释放
*/
class JZNodeObjectPointer
{
public:
    JZNodeObjectPointer();
    JZNodeObjectPointer(int data_type);
    JZNodeObjectPointer(JZNodeObject *obj,bool isOwner);
    ~JZNodeObjectPointer();

    int type() const;
    JZNodeObject *object() const;

    void relaseObject();   //设置为空
    void releaseOwner();
    JZNodeObjectPointer toWeakPointer();

    bool operator ==(const JZNodeObjectPointer &other) const;
    bool operator !=(const JZNodeObjectPointer &other) const;

protected:
    int m_dataType;
    QSharedPointer<JZNodeObjectData> m_data;
};
Q_DECLARE_METATYPE(JZNodeObjectPointer)

bool isJZObject(const QVariant &v);
JZNodeObject* toJZObject(const QVariant &v);
JZNodeObjectPointer toJZObjectHolder(const QVariant &v);
JZNodeObject* qobjectToJZObject(QObject *obj);

void JZObjectConnect(JZNodeObject* sender, JZFunctionPointer single, JZFunctionPointer slot);
void JZObjectDisconnect(JZNodeObject* sender, JZFunctionPointer single, JZFunctionPointer slot);
void JZObjectConnect(JZNodeObject *sender, JZFunctionPointer single, JZNodeObject *recv, JZFunctionPointer function);
void JZObjectDisconnect(JZNodeObject *sender, JZFunctionPointer single, JZNodeObject *recv, JZFunctionPointer function);
bool JZObjectIsList(JZNodeObject *obj);
bool JZObjectIsMap(JZNodeObject *obj);
bool JZObjectIsSet(JZNodeObject *obj);

class JZScriptEnvironment;
class JZNodeObjectManager
{
public:    
    JZNodeObjectManager(JZScriptEnvironment *env);
    ~JZNodeObjectManager();     

    void init();

    JZScriptEnvironment *env();
    const JZScriptEnvironment *env() const;

    int getId(const QString &type_name) const;
    int getIdByCTypeid(const QString &ctypeid) const;

    void setUserRegist(bool flag);
    void clearUserReigst();

    bool hasType(int type_id) const;
    const JZNodeObjectDefine *meta(const QString &className) const;
    const JZNodeObjectDefine *meta(int type_id) const;
    QString getClassName(int type_id) const;
    int getClassId(const QString &class_name) const;
    bool isInherits(const QString &class_name, const QString &super_name) const;
    bool isInherits(int class_name,int super_name) const;
    QStringList getClassList() const;

    const JZNodeEnumDefine *enumMeta(int type_id) const;
    const JZNodeEnumDefine *enumMeta(const QString &enumName) const;
    QString getEnumName(int type_id) const;
    int getEnumId(const QString &enumName) const;
    QStringList getEnumList() const;

    const JZSignalDefine *signal(const QString &name) const;
    
    int getQObjectType(const QString & type_name) const;
    void setQObjectType(const QString & type_name,int id);

    int delcare(const QString & type_name, int id = Type_none);
    int delcareCClass(const QString & type_name, const QString &ctype_id, int id = Type_none);

    int regist(const JZNodeObjectDefine &define);    
    int registCClass(const JZNodeObjectDefine &define,const QString &type_id);
    void replace(const JZNodeObjectDefine &define);    
    
    JZNodeObject* create(int type_id) const;
    JZNodeObject* create(const QString & type_name) const;
    JZNodeObject* createByCTypeid(const QString &ctype_id) const;
    JZNodeObject* createReference(int type_id, void *cobj, bool owner) const;
    JZNodeObject* createReference(const QString &type_name,void *cobj,bool owner) const;
    JZNodeObject* createReferenceByCTypeid(const QString &ctype_id,void *cobj,bool owner) const;
    
    void destory(JZNodeObject *obj) const;

    JZNodeObjectPointer createHolder(int type_id) const;
    JZNodeObjectPointer createHolder(const QString& type_name) const;

    JZNodeObject* clone(JZNodeObject *src) const;
    bool equal(JZNodeObject* o1,JZNodeObject *o2) const;

    //enum
    int registEnum(const JZNodeEnumDefine &enumName);
    int registCEnum(const JZNodeEnumDefine &enumName, const QString &ctype_id);    
    void setFlag(int flag, int flag_enum);
    void unregist(int id);        

    JZEnum createEnum(int enumType) const;

    //template
    template<class T>
    JZNodeObject* objectCreate() const
    {
        auto obj = createByCTypeid(typeid(T).name());
        return obj;
    }

    template<class T>
    JZNodeObject *objectReference(T ptr, bool cowner) const
    {
        static_assert(std::is_pointer<T>(), "only support class pointer");
        QString c_typeid = typeid(std::remove_pointer_t<T>).name();
        auto obj = createReferenceByCTypeid(c_typeid, ptr, cowner);
        return obj;
    }

    template<class T>
    JZNodeObjectPointer objectCreatePointer() const
    {
        auto obj = objectCreate<T>();
        return JZNodeObjectPointer(obj,true);
    }

    template<class T>
    JZNodeObjectPointer objectReferencePointer(T ptr, bool cowner) const
    {
        auto obj = objectReference<T>(ptr, cowner);
        return JZNodeObjectPointer(obj,true);  //这里代表true是不是管理obj， 上面的cowner代表是不是管理c  
    }

    template<class T>
    QVariant objectCreateVariant() const
    {
        auto holder = objectCreatePointer<T>();
        return QVariant::fromValue(holder);
    }

    template<class T>
    QVariant objectReferenceVariant(T ptr, bool cowner) const
    {
        auto holder = objectReferencePointer<T>(ptr, cowner);
        return QVariant::fromValue(holder);
    }

    template<class T>
    T* objectCast(JZNodeObject* obj) const
    {
        static_assert(!std::is_pointer<T>(), "do not use pointer");
        int c_type = getIdByCTypeid(typeid(T).name());
        Q_ASSERT(obj->isInherits(c_type));
        return (T*)obj->cobj();
    }

    template<class T>
    T* objectCast(const JZNodeObjectPointer &holder) const
    {
        auto obj = holder.object();
        return objectCast<T>(obj);
    }

    template<class T>
    T *objectCast(const QVariant &v) const
    {
        auto obj = toJZObject(v);
        return objectCast<T>(obj);
    }

protected:
    void initFunctions();
    void create(const JZNodeObjectDefine *define,JZNodeObject *obj) const;
    void copy(JZNodeObject *dst,JZNodeObject *src) const;    
    
    JZScriptEnvironment *m_env;    
    
    QMap<QString, int> m_ctypeidMap;

    QMap<int,JZNodeEnumDefine> m_enums;
    int m_enumId;
    
    QMap<int, QSharedPointer<JZNodeObjectDefine>> m_metas;
    QMap<QString,int> m_qobjectId;
    int m_objectId;
    bool m_userRegist;
};

template<class T>
T* JZObjectCast(JZNodeObject *obj)
{
    return obj->manager()->objectCast<T>(obj);
}

#endif
