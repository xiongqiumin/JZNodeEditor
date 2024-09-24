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
class JZNodeObjectManager;
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
    JZNodeObjectManager *manager;
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
class JZScriptEnvironment;
class JZCORE_EXPORT JZNodeObject : public QObject
{
    Q_OBJECT

public:        
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

    bool m_isNull;
    const JZNodeObjectDefine *m_define;
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

void JZObjectConnect(JZNodeObject *sender, JZFunctionPointer single, JZNodeObject *recv, JZFunctionPointer function);
void JZObjectDisconnect(JZNodeObject *sender, JZFunctionPointer single, JZNodeObject *recv, JZFunctionPointer function);
bool JZObjectIsList(JZNodeObject *obj);
bool JZObjectIsMap(JZNodeObject *obj);
bool JZObjectIsSet(JZNodeObject *obj);

class JZScriptEnvironment;
class JZCORE_EXPORT JZNodeObjectManager
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

    void setUnitTest(bool flag);

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
    
    int getQObjectType(const QString &name) const;
    void setQObjectType(const QString &name,int id);

    int delcare(const QString &name, int id = -1);
    int delcareCClass(const QString &name, const QString &ctype_id, int id = -1);

    int regist(const JZNodeObjectDefine &define);    
    int registCClass(const JZNodeObjectDefine &define,const QString &type_id);
    void replace(const JZNodeObjectDefine &define);    
    
    JZNodeObject* create(int type_id) const;
    JZNodeObject* create(const QString &name) const;
    JZNodeObject* createByCTypeid(const QString &ctype_id) const;
    JZNodeObject* createRefrence(int type_id, void *cobj, bool owner) const;
    JZNodeObject* createRefrence(const QString &type_name,void *cobj,bool owner) const;
    JZNodeObject* createRefrenceByCTypeid(const QString &ctype_id,void *cobj,bool owner) const;
    
    JZNodeObject* createNull(int type) const;
    JZNodeObject* createNull(const QString &name) const;
    void destory(JZNodeObject *obj) const;

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
    QVariant objectCreate() const
    {
        auto obj = createByCTypeid(typeid(T).name());
        return QVariant::fromValue(JZNodeObjectPtr(obj, true));
    }

    template<class T>
    QVariant objectRefrence(T ptr, bool owner) const
    {
        static_assert(std::is_pointer<T>(), "only support class pointer");
        QString c_typeid = typeid(std::remove_pointer_t<T>).name();
        auto obj = createRefrenceByCTypeid(c_typeid, ptr, owner);
        return QVariant::fromValue(JZNodeObjectPtr(obj, true));
    }

    template<class T>
    T *objectCast(const QVariant &v) const
    {
        int c_type = getIdByCTypeid(typeid(T).name());
        auto obj = toJZObject(v);
        Q_ASSERT(obj->isInherits(c_type));
        return (T*)obj->cobj();
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
    bool m_testMode;
};

#endif
