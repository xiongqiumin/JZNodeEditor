#ifndef JZ_SCRIPT_ENVIRONMENT_H_
#define JZ_SCRIPT_ENVIRONMENT_H_

#include "JZNodeObject.h"
#include "JZNodeFunctionManager.h"
#include "JZModule.h"

class JZScriptEnvironment;
typedef QVariant (*ConvertFunc)(JZScriptEnvironment *env,const QVariant& v);

//JZNodeTypeMeta
class JZCORE_EXPORT JZNodeTypeMeta
{
public:
    void clear();

    const JZFunctionDefine *function(QString name) const;
    const JZNodeObjectDefine *object(QString name) const;

    QList<JZFunctionDefine> functionList;
    QList<JZNodeObjectDefine> objectList;       
    QList<JZNodeCObjectDelcare> cobjectList;
    QStringList moduleList;
};
QDataStream &operator<<(QDataStream &s, const JZNodeTypeMeta &param);
QDataStream &operator>>(QDataStream &s, JZNodeTypeMeta &param);

class JZCORE_EXPORT JZScriptEnvironment
{
public:
    JZScriptEnvironment();
    ~JZScriptEnvironment();
    
    void registType(const JZNodeTypeMeta &type_info);
    void unregistType();

    bool loadModule(QString name);
    void unloadModule(QString name);

    JZNodeObjectManager *objectManager();
    JZNodeFunctionManager *functionManager();
    
    QString typeToName(int id);
    int nameToType(const QString &name);
    int typeidToType(const QString &name);
    int variantType(const QVariant &v);
    QString variantTypeName(const QVariant &v);
    int stringType(const QString &text);

    bool canConvert(int from,int to);    //隐式转换
    bool canConvertExplicitly(int from,int to);    //被 convertTo 支持的
    QVariant convertTo(int type,const QVariant &v); 
    void registConvert(int from, int to, ConvertFunc func);

    bool isBase(int type);
    bool isEnum(int type);
    bool isBaseOrEnum(int type);
    bool isBool(int type);
    bool isNumber(int type);
    bool isObject(int type);

    bool isNullObject(const QVariant &v);   
    bool isNullptr(const QVariant &v);
    bool isWidget(const QVariant &v);
    bool isSameType(const QVariant &src_v,const QVariant &dst_v);
    bool isSameType(int src_type,int dst_type);
    bool isLiteralType(int type);

    int isInherits(const QString &type1,const QString &type2);
    int isInherits(int type1,int type2);    

    bool isVaildType(QString type);
    int upType(int type1, int type2);  //提升类型
    int upType(QList<int> types);
    int matchType(QList<int> src_types,QList<int> dst_types);
    bool canInitValue(int type,const QString &v);
    QVariant defaultValue(int type);
    QVariant initValue(int type, const QString &v);

    template<class T>
    QVariant objectCreate()
    {
        auto obj = m_objectManager.createByCTypeid(typeid(T).name());
        return QVariant::fromValue(JZNodeObjectPtr(obj, true));
    }

    template<class T>
    QVariant objectRefrence(T ptr, bool owner)
    {
        static_assert(std::is_pointer<T>(), "only support class pointer");
        QString c_typeid = typeid(std::remove_pointer_t<T>).name();
        auto obj = m_objectManager.createRefrenceByCTypeid(c_typeid, ptr, owner);
        return QVariant::fromValue(JZNodeObjectPtr(obj, true));
    }

    template<class T>
    T *objectCast(const QVariant &v)
    {
        int c_type = m_objectManager.getIdByCTypeid(typeid(T).name());
        auto obj = toJZObject(v);
        Q_ASSERT(obj->isInherits(c_type));
        return (T*)obj->cobj();
    }

protected:
    struct ModuleInfo
    {
        JZModule *module;
        int ref;
    };

    ModuleInfo *module(QString name);    
    int64_t makeConvertId(int from, int to);

    JZNodeFunctionManager m_funcManager;
    JZNodeObjectManager m_objectManager;
    QList<ModuleInfo*> m_moduleList;

    QMap<int64_t,ConvertFunc> convertMap;
};


#endif