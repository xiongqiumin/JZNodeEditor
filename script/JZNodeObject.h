#ifndef JZNODE_OBJECT_H_
#define JZNODE_OBJECT_H_

#include <QMap>
#include <QWidget>
#include <functional>
#include "JZEvent.h"
#include "JZNodeFunctionDefine.h"
#include "JZNodeEnum.h"

class JZCORE_EXPORT CMeta
{
public:
    CMeta();

    bool isAbstract;
    bool isCopyable;
    std::function<void*()> create;
    std::function<void(void*,void*)> copy;  //src->dst
    std::function<bool(void*,void*)> equal;
    std::function<void(void*)> destory;
};

/* 对于虚函数，this 参数应当为基类指针
*/
class JZCORE_EXPORT JZNodeObjectDefine
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
    const JZNodeObjectDefine *cBase() const; //最近的c定义
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
};
QDataStream &operator<<(QDataStream &s, const JZNodeObjectDefine &param);
QDataStream &operator>>(QDataStream &s, JZNodeObjectDefine &param);

class JZCORE_EXPORT JZNodeCObjectDelcare
{
public:
    JZNodeCObjectDelcare();
    
    QString className;
    int id;
};
QDataStream &operator<<(QDataStream &s, const JZNodeCObjectDelcare &param);
QDataStream &operator>>(QDataStream &s, JZNodeCObjectDelcare &param);

class JZCORE_EXPORT JZObjectNull
{
public:
    JZObjectNull();
};
QDataStream &operator<<(QDataStream &s, const JZObjectNull &param);
QDataStream &operator>>(QDataStream &s, JZObjectNull &param);
Q_DECLARE_METATYPE(JZObjectNull)

class JZNodeObjectManager;
class JZCORE_EXPORT JZNodeObject : public QObject
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
    QVariant param(const QString &name) const;
    void setParam(const QString &name,const QVariant &value);    
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
    void autoConnect();
    void autoBind();

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
   
    const JZCParamDefine *cparam(const QString &name) const;
    int singleConnectCount(JZNodeObject *recv);
    void clearCObj();

    bool m_isNull;
    JZNodeObjectDefine *m_define;    
    QMap<QString, QVariantPtr> m_params;
    void *m_cobj;
    bool m_cobjOwner;

    QList<ConnectInfo> m_connectList;
};

class JZCORE_EXPORT JZNodeObjectPtr
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

JZCORE_EXPORT bool isJZObject(const QVariant &v);
JZCORE_EXPORT JZNodeObject* toJZObject(const QVariant &v);
JZCORE_EXPORT JZNodeObjectPtr toJZObjectPtr(const QVariant &v);
JZCORE_EXPORT JZNodeObject* qobjectToJZObject(QObject *obj);
JZCORE_EXPORT JZNodeObject* objectFromString(int type,const QString &text);

int JZClassId(const QString &name);
QString JZClassName(int id);
void JZObjectConnect(JZNodeObject *sender, JZFunctionPointer single, JZNodeObject *recv, JZFunctionPointer function);
void JZObjectDisconnect(JZNodeObject *sender, JZFunctionPointer single, JZNodeObject *recv, JZFunctionPointer function);
bool JZObjectIsList(JZNodeObject *obj);
bool JZObjectIsMap(JZNodeObject *obj);
bool JZObjectIsSet(JZNodeObject *obj);

class JZCORE_EXPORT JZNodeObjectManager
{
public:
    static JZNodeObjectManager *instance();
    JZNodeObjectManager();   
    ~JZNodeObjectManager();     

    void init();
    int getId(const QString &type_name);
    int getIdByCTypeid(const QString &ctypeid);

    void setUserRegist(bool flag);
    void setUnitTest(bool flag);

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
    JZNodeObject* createByCTypeid(const QString &ctype_id);
    JZNodeObject* createRefrence(int type_id, void *cobj, bool owner);
    JZNodeObject* createRefrence(const QString &type_name,void *cobj,bool owner);
    JZNodeObject* createRefrenceByCTypeid(const QString &ctype_id,void *cobj,bool owner);
    
    JZNodeObject* createNull(int type);
    JZNodeObject* createNull(const QString &name);

    JZNodeObject* clone(JZNodeObject *src);
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
    
    QMap<QString, int> m_ctypeidMap;
    
    QMap<int,JZNodeEnumDefine> m_enums;
    int m_enumId;
    
    QMap<int, QSharedPointer<JZNodeObjectDefine>> m_metas;
    QMap<QString,int> m_qobjectId;
    int m_objectId;
    bool m_testMode;
};

template<class T>
QVariant JZObjectCreate()
{
    auto obj = JZNodeObjectManager::instance()->createByCTypeid(typeid(T).name());
    return QVariant::fromValue(JZNodeObjectPtr(obj,true));
}

template<class T>
QVariant JZObjectCreateRefrence(T ptr,bool owner)
{
    static_assert(std::is_pointer<T>(), "only support class pointer");
    QString c_typeid = typeid(std::remove_pointer_t<T>).name();
    auto obj = JZNodeObjectManager::instance()->createRefrenceByCTypeid(c_typeid, ptr, owner);
    return QVariant::fromValue(JZNodeObjectPtr(obj,true));
}

template<class T>
T *JZObjectCast(const QVariant &v)
{
    int c_type = JZNodeObjectManager::instance()->getIdByCTypeid(typeid(T).name());
    auto obj = toJZObject(v);
    Q_ASSERT(obj->isInherits(c_type));
    return (T*)obj->cobj();
}

#endif
