﻿#ifndef JZNODE_OBJECT_H_
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
    std::function<bool(void*,void*)> equal;
    std::function<void(void*)> destory;
};

/* 对于虚函数，this 参数应当为基类指针
*/
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

    JZFunctionDefine initStaticFunction(QString function);
    JZFunctionDefine initMemberFunction(QString function);
    JZFunctionDefine initVirtualFunction(QString function);
    JZFunctionDefine initSlotFunction(QString param,QString single);

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
    QList<JZSignalDefine> singles;
    QStringList enums;

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
    bool isValueType() const;
    const QString &className() const;
    int type() const;
    int baseType() const;
    const JZNodeObjectDefine *meta() const;

    bool hasParam(const QString &name) const;
    const QVariant &param(const QString &name) const;
    void setParam(const QString &name,const QVariant &value);
    QVariant *paramRef(const QString &name);
    QStringList paramList() const;

    const JZFunctionDefine *function(const QString &function) const;
    QStringList functionList() const;
    
    const JZSignalDefine *signal(QString function) const;
    QStringList signalList() const;

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
    void onDestory(QObject *obj);
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
    void clearCObj();

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

    bool operator ==(const JZNodeObjectPtr &other) const;
    bool operator !=(const JZNodeObjectPtr &other) const;

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
JZNodeObject* qobjectToJZObject(QObject *obj);
JZNodeObject* objectFromString(int data_name,const QString &text);

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
    int getId(const QString &type_name);
    int getIdByCType(const QString &type_name);

    JZNodeObjectDefine *meta(const QString &className);
    JZNodeObjectDefine *meta(int type_id);
    QString getClassName(int type_id);
    int getClassId(const QString &class_name);    
    bool isInherits(const QString &class_name, const QString &super_name);
    bool isInherits(int class_name,int super_name);
    QStringList getClassList();

    JZNodeEnumDefine *enumMeta(int type_id);
    JZNodeEnumDefine *enumMeta(const QString &enumName);
    QString getEnumName(int type_id);
    int getEnumId(const QString &enumName);
    QStringList getEnumList();    

    const JZSignalDefine *signal(const QString &name);
    
    int getQObjectType(const QString &name);
    void setQObjectType(const QString &name,int id);

    int delcare(const QString &name, int id = -1);
    int delcareCClass(const QString &name, const QString &ctype_id, int id = -1);

    int regist(const JZNodeObjectDefine &define);    
    int registCClass(const JZNodeObjectDefine &define,const QString &type_id);
    void replace(const JZNodeObjectDefine &define);    
    
    JZNodeObject* create(int type_id);
    JZNodeObject* create(const QString &name);
    JZNodeObject* createByTypeid(const QString &ctype_id);
    JZNodeObject* createRefrence(int type_id, void *cobj, bool owner);
    JZNodeObject* createRefrence(const QString &type_name,void *cobj,bool owner);
    JZNodeObject* createRefrenceByTypeid(const QString &ctype_id,void *cobj,bool owner);
    
    JZNodeObject* createNull(int type);
    JZNodeObject* createNull(const QString &name);

    JZNodeObject* clone(JZNodeObject *other);
    bool equal(JZNodeObject* o1,JZNodeObject *o2);

    //enum
    int registEnum(const JZNodeEnumDefine &enumName);
    int registCEnum(const JZNodeEnumDefine &enumName, const QString &ctype_id);
    void unregist(int id);    
    void clearUserReigst();

    JZEnum createEnum(int enumType);

protected:
    void create(const JZNodeObjectDefine *define,JZNodeObject *obj);
    void copy(JZNodeObject *dst,JZNodeObject *src);
    void initFunctions();
    
    QMap<QString, int> m_typeidMetas;
    
    QMap<int,JZNodeEnumDefine> m_enums;
    int m_enumId;
    
    QMap<int, QSharedPointer<JZNodeObjectDefine>> m_metas;
    QMap<QString,int> m_qobjectId;
    int m_objectId;    
};

template<class T>
QVariant JZObjectCreate()
{
    auto obj = JZNodeObjectManager::instance()->createByTypeid(typeid(T).name());
    return QVariant::fromValue(JZNodeObjectPtr(obj,true));
}

template<class T>
QVariant JZObjectCreateRefrence(T ptr,bool owner)
{
    static_assert(std::is_pointer<T>(), "only support class pointer");
    QString c_typeid = typeid(std::remove_pointer_t<T>).name();
    auto obj = JZNodeObjectManager::instance()->createRefrenceByTypeid(c_typeid, ptr, owner);
    return QVariant::fromValue(JZNodeObjectPtr(obj,true));
}

template<class T>
T *JZObjectCast(const QVariant &v)
{
    int c_type = JZNodeObjectManager::instance()->getIdByCType(typeid(T).name());
    auto obj = toJZObject(v);
    Q_ASSERT(obj->isInherits(c_type));
    return (T*)obj->cobj();
}

#endif
