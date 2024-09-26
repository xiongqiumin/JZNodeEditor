#ifndef JZ_SCRIPT_ENVIRONMENT_H_
#define JZ_SCRIPT_ENVIRONMENT_H_

#include "JZNodeObject.h"
#include "JZModule.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeEditorManager.h"

class JZScriptEnvironment;
typedef QVariant (*ConvertFunc)(const JZScriptEnvironment *env,const QVariant& v);

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
    const JZNodeObjectManager *objectManager() const;

    JZNodeFunctionManager *functionManager();
    const JZNodeFunctionManager *functionManager() const;

    JZNodeEditorManager *editorManager();
    const JZNodeEditorManager *editorManager() const;
    
    QString typeToName(int id) const;
    int nameToType(const QString &name) const;
    QList<int> nameToTypeList(const QStringList &names) const;
    int typeidToType(const QString &name) const;
    int variantType(const QVariant &v) const;
    QString variantTypeName(const QVariant &v) const;
    int stringType(const QString &text) const;

    bool isSameType(const QVariant &src_v, const QVariant &dst_v) const;
    bool isSameType(int src_type, int dst_type) const;

    int isInherits(const QString &type1, const QString &type2) const;
    int isInherits(int type1, int type2) const;

    JZParamDefine paramDefine(QString name, int data_type, QString value = QString()) const;
    void registConvert(int from, int to, ConvertFunc func);
    bool canConvert(int from,int to) const;    //隐式转换
    bool canConvertExplicitly(int from,int to) const;    //被 convertTo 支持的
    QVariant convertTo(int type,const QVariant &v) const;
    QVariant clone(const QVariant &v) const;        

    bool isVaildType(QString type) const;
    int upType(int type1, int type2) const;  //提升类型
    int upType(QList<int> types) const;
    int matchType(QList<int> src_types,QList<int> dst_types) const;
    bool canInitValue(int type,const QString &v) const;
    QVariant defaultValue(int type) const;
    QString defaultValueString(int type) const;
    QVariant initValue(int type, const QString &v) const;

protected:
    struct ModuleInfo
    {
        JZModule *module;
        int ref;
    };

    ModuleInfo *module(QString name);    
    int64_t makeConvertId(int from, int to) const;

    JZNodeFunctionManager m_funcManager;
    JZNodeObjectManager m_objectManager;
    JZNodeEditorManager m_editorManager;    
    QList<ModuleInfo*> m_moduleList;

    QMap<int64_t,ConvertFunc> convertMap;    
};


#endif