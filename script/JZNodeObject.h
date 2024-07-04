#ifndef JZNODE_OBJECT_H_
#define JZNODE_OBJECT_H_

#include <QMap>
#include <QWidget>
#include <functional>
#include "JZEvent.h"
#include "JZNodeFunctionDefine.h"
#include "JZNodeEnum.h"

class CMeta
{
public:
    CMeta();

    bool isAbstract;
    bool isCopyable;
    std::function<void*()> create;
    std::function<void(void*,void*)> copy;
    std::function<void(void*)> destory;
};

class JZNodeObjectDefine
{
public:
    JZNodeObjectDefine();

    QString fullname() const;

    void addParam(const JZParamDefine &def);
    void removeParam(const QString &name);
    QStringList paramList(bool hasParent) const;
    const JZParamDefine *param(const QString &name) const;

    JZFunctionDefine initStaticFunction(QString function);
    JZFunctionDefine initMemberFunction(QString function);
    JZFunctionDefine initVirtualFunction(QString function);
    JZFunctionDefine initSlotFunction(QString param,QString single);

    void addFunction(const JZFunctionDefine &def);
    void removeFunction(const QString &function);    
    int indexOfFunction(const QString &function) const;
    bool checkFunction(const QString &function,QString &error) const;
    const JZFunctionDefine *function(const QString &function) const;
    QStringList functionList() const;
    
    const JZSingleDefine *single(const QString &function) const;
    QStringList singleList() const;

    const JZFunctionDefine *slot(const QString &function) const;
    QStringList slotList() const;
    
    const JZNodeObjectDefine *super() const;
    bool isInherits(int type) const;
    bool isInherits(const QString &name) const;
    bool isCopyable() const;
    bool isValueType() const;

    int id;
    QString nameSpace;
    QString className;            
    QString superName;
    bool valueType;
    QMap<QString,JZParamDefine> params;
    QList<JZFunctionDefine> functions;
    QList<JZSingleDefine> singles;

    bool isUiWidget;
    QString widgetXml;
    QList<JZParamDefine> widgetParams;

    bool isCObject;    
    CMeta cMeta;
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

class JZObjectNull
{
public:
    JZObjectNull();
};
QDataStream &operator<<(QDataStream &s, const JZObjectNull &param);
QDataStream &operator>>(QDataStream &s, JZObjectNull &param);
Q_DECLARE_METATYPE(JZObjectNull)

class JZNodeObjectManager;
class JZNodeObject : public QObject
{
    Q_OBJECT

public:    
    JZNodeObject(JZNodeObjectDefine *def);
    ~JZNodeObject();

    bool isNull() const;
    bool isInherits(int type) const;
    bool isInherits(const QString &name) const;
    bool isCopyable() const;
    bool isCObject() const;
    const QString &className() const;
    int type() const;
    const JZNodeObjectDefine *meta() const;

    bool hasParam(const QString &name) const;
    const QVariant &param(const QString &name) const;
    void setParam(const QString &name,const QVariant &value);
    QVariant *paramRef(const QString &name);
    QStringList paramList() const;

    const JZFunctionDefine *function(const QString &function) const;
    QStringList functionList() const;
    
    const JZSingleDefine *single(QString function) const;
    QStringList singleList() const;

    void singleConnect(QString sig,JZNodeObject *recv,QString slot);
    void singleDisconnect(QString sig,JZNodeObject *recv,QString slot);
    void singleEmit(QString sig,const QVariantList &params);

    void *cobj() const;
    void setCObject(void *cobj,bool owner);        
    void setCOwner(bool owner);

    void updateUiWidget(QWidget *widget);

signals:
    void sig(QString slot_function,const QVariantList &params);

public slots:
    void onSig(QString slot_function,const QVariantList &params);
    void onRecvDestory(QObject *obj);

protected:    
    Q_DISABLE_COPY(JZNodeObject);  

    struct ConnectInfo
    {
        QString single;
        JZNodeObject *recv;
        QString slot;
    };
      
    friend JZNodeObjectManager;

    bool getParamRef(const QString &name,QVariant* &ref,QString &error);    
    int singleConnectCount(JZNodeObject *recv);

    bool m_isNull;
    JZNodeObjectDefine *m_define;    
    QMap<QString,QVariantPtr> m_params;    
    void *m_cobj;
    bool m_cobjOwner;

    QList<ConnectInfo> m_connectList;
};

class JZNodeObjectPtr
{
public:
    JZNodeObjectPtr();
    JZNodeObjectPtr(JZNodeObject *obj,bool isOwner);
    ~JZNodeObjectPtr();

    JZNodeObject *object() const;
    void releaseOwner();

protected:
    class JZNodeObjectPtrData
    {
    public:
        JZNodeObjectPtrData();
        ~JZNodeObjectPtrData();

        bool isOwner;
        JZNodeObject *object;
    };

    QSharedPointer<JZNodeObjectPtrData> data;
};
Q_DECLARE_METATYPE(JZNodeObjectPtr)

bool isJZObject(const QVariant &v);
JZNodeObject* toJZObject(const QVariant &v);
JZNodeObjectPtr toJZObjectPtr(const QVariant &v);
JZObjectNull* toJZNullptr(const QVariant &v);
JZNodeObject* qobjectToJZObject(QObject *obj);

int JZClassId(const QString &name);
QString JZClassName(int id);
void JZObjectConnect(JZNodeObject *sender, JZFunctionPointer single, JZNodeObject *recv, JZFunctionPointer function);
void JZObjectDisconnect(JZNodeObject *sender, JZFunctionPointer single, JZNodeObject *recv, JZFunctionPointer function);
bool JZObjectIsList(JZNodeObject *obj);
bool JZObjectIsMap(JZNodeObject *obj);
bool JZObjectIsSet(JZNodeObject *obj);

class JZNodeObjectManager
{
public:
    static JZNodeObjectManager *instance();
    JZNodeObjectManager();   
    ~JZNodeObjectManager();     

    void init();
    int getId(QString type_name);
    int getIdByCType(QString type_name);

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

    const JZSingleDefine *single(QString name);
    
    int getQObjectType(QString name);
    void setQObjectType(QString name,int id);

    int delcare(QString name, int id = -1);
    int delcareCClass(QString name, QString ctype_id, int id = -1);

    int regist(JZNodeObjectDefine define);    
    int registCClass(JZNodeObjectDefine define,QString type_id);
    void replace(JZNodeObjectDefine define);    
    
    JZNodeObject* create(int type_id);
    JZNodeObject* create(QString name);
    JZNodeObject* createByTypeid(QString ctype_id);
    JZNodeObject* createRefrence(int type_id, void *cobj, bool owner);
    JZNodeObject* createRefrence(QString type_name,void *cobj,bool owner);
    JZNodeObject* createRefrenceByTypeid(QString ctype_id,void *cobj,bool owner);
    
    JZNodeObject* createNull(int type);
    JZNodeObject* createNull(QString name);

    JZNodeObject* clone(JZNodeObject *other);

    //enum
    int registEnum(JZNodeEnumDefine enumName);
    int registCEnum(JZNodeEnumDefine enumName, QString ctype_id);
    void unregist(int id);    
    void clearUserReigst();

    JZEnum createEnum(int enumType);

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
JZNodeObject *JZObjectCreate()
{
    auto ptr = JZNodeObjectManager::instance()->createByTypeid(typeid(T).name());
    return ptr;
}

template<class T>
JZNodeObject *JZObjectCreateRefrence(T ptr,bool owner)
{
    static_assert(std::is_pointer<T>(), "only support class pointer");
    QString c_typeid = typeid(std::remove_pointer_t<T>).name();
    auto obj = JZNodeObjectManager::instance()->createRefrenceByTypeid(c_typeid, ptr, owner);
    return obj;
}

template<class T>
T *JZObjectCast(JZNodeObject *obj)
{
    int c_type = JZNodeObjectManager::instance()->getIdByCType(typeid(T).name());
    Q_ASSERT(obj->isInherits(c_type));
    return (T*)obj->cobj();
}

template<class T>
T *JZObjectCast(const QVariant &v)
{
    return JZObjectCast<T>(toJZObject(v));
}

#endif
