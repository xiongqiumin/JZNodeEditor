#include "JZScriptEnvironment.h"
#include "JZRegExpHelp.h"
#include "JZNodeBind.h"
#include "JZContainer.h"
#include "JZNodeEngine.h"
#include "JZScriptBuildInFunction.h"
#include "runtime/JZWidgetBind.h"
#include "JZNodeEvent.h"

//JZScriptEnvironment
JZScriptEnvironment::JZScriptEnvironment()
    :m_funcManager(this),
     m_objectManager(this)
{    
    jzbind::setBindEnvironment(this);
    m_objectManager.init();
    m_funcManager.init();

    InitBuildInFunction();
    JZWidgetBindInit();

    JZModuleManager::instance()->regist(this);

    JZNodeEventFunctionInit(this);

    m_objectManager.setUserRegist(true);
    m_funcManager.setUserRegist(true);
    jzbind::setBindEnvironment(nullptr);
}

JZScriptEnvironment::~JZScriptEnvironment()
{    
    qDeleteAll(m_moduleList);
}

void JZScriptEnvironment::registType(const JZNodeTypeMeta &type_info)
{
    unregistType();
    
    auto define_list = type_info.objectList;
    auto &cobj_list = type_info.cobjectList;
    auto &function_list = type_info.functionList;
    QList<int> cobj_id;

    //delcare
    for (int i = 0; i < define_list.size(); i++)
    {
        QString error;
        define_list[i].id = m_objectManager.delcare(define_list[i].className, define_list[i].id);
    }
   
    //regist
    for (int i = 0; i < define_list.size(); i++)
    {
        QString error;
        m_objectManager.replace(define_list[i]);
    }
    for (int i = 0; i < function_list.size(); i++)        
        m_funcManager.registFunction(function_list[i]);
}

void JZScriptEnvironment::unregistType()
{
    for (int i = 0; i < m_moduleList.size(); i++)
        m_moduleList[i]->module->unregist(this);
    qDeleteAll(m_moduleList);
    m_moduleList.clear();

    m_funcManager.clearUserReigst();
    m_objectManager.clearUserReigst();    
}

JZNodeFactory *JZScriptEnvironment::factoryManager()
{
    return &m_nodeFactory;
}

const JZNodeFactory *JZScriptEnvironment::factoryManager() const
{
    return &m_nodeFactory;
}

JZNodeObjectManager *JZScriptEnvironment::objectManager()
{        
    return &m_objectManager;
}

const JZNodeObjectManager *JZScriptEnvironment::objectManager() const
{
    return &m_objectManager;
}

JZNodeFunctionManager *JZScriptEnvironment::functionManager()
{        
    return &m_funcManager;
}

const JZNodeFunctionManager *JZScriptEnvironment::functionManager() const
{
    return &m_funcManager;
}

const JZFunctionDefine* JZScriptEnvironment::function(const QString& name) const
{
    return m_funcManager.function(name);
}

const JZFunction* JZScriptEnvironment::functionImpl(const QString& name) const
{
    return m_funcManager.functionImpl(name);
}

JZScriptEnvironment::ModuleInfo *JZScriptEnvironment::module(QString name)
{
    for(int i = 0; i < m_moduleList.size(); i++)
    {
        if(m_moduleList[i]->module->name() == name)
            return m_moduleList[i];
    }

    auto m = JZModuleManager::instance()->module(name);
    if (!m)
        return nullptr;

    ModuleInfo *info = new ModuleInfo();
    info->module = m;
    info->ref = 0;
    m_moduleList.push_back(info);
    return info;
}

bool JZScriptEnvironment::loadModule(QString name)
{
    ModuleInfo *m = module(name);    
    if (!m)
        return false;
    
    m->ref++;
    if(m->ref == 1)
    {
        jzbind::setBindEnvironment(this);
        m->module->regist(this);
        jzbind::setBindEnvironment(nullptr);
    }

    auto depends = m->module->depends();
    for(int i = 0; i < depends.size(); i++)
    {
        if(!loadModule(depends[i]))
            return false;
    }        
    return true;
}

void JZScriptEnvironment::unloadModule(QString name)
{
    auto *m = module(name);
    Q_ASSERT(m);      

    m->ref--;
    if(m->ref == 0)
        m->module->unregist(this);

    auto depends = m->module->depends();
    for(int i = 0; i < depends.size(); i++)
        unloadModule(depends[i]);
}

int64_t JZScriptEnvironment::makeConvertId(int from, int to) const
{
    int64_t id = (int64_t)from << 32 | (int64_t)to;
    return id;
}

const JZNodeObjectDefine* JZScriptEnvironment::meta(int id) const
{
    return m_objectManager.meta(id);
}

const JZNodeObjectDefine* JZScriptEnvironment::meta(const QString& name) const
{
    return m_objectManager.meta(name);
}

bool JZScriptEnvironment::hasType(int type) const
{
    return m_objectManager.hasType(type);
}

QString JZScriptEnvironment::typeToName(int id) const
{   
    if (id == Type_none)
        return JZNodeType::typeName(id);

    bool isPoint = JZNodeType::isPointer(id);
    id = JZNodeType::baseType(id);
    QString suffix = isPoint ? "*" : "";

    if (JZNodeType::isEnum(id))
    {
        return m_objectManager.getEnumName(id) + suffix;
    }
    else if (id >= Type_class)
        return m_objectManager.getClassName(id) + suffix;
    else
        return JZNodeType::typeName(id) + suffix;
}

int JZScriptEnvironment::nameToType(const QString &name) const
{   
    bool isPoint = JZNodeType::isPointer(name);
    if (isPoint)
    {
        int base_type = nameToType(JZNodeType::baseType(name));
        return JZNodeType::pointerType(base_type);
    }

    int type = JZNodeType::nameToType(name);
    if (type != Type_none)
        return type;

    return m_objectManager.getId(name);
}

QList<int> JZScriptEnvironment::nameListToTypeList(const QStringList &names) const
{
    QList<int> ret;
    for (int i = 0; i < names.size(); i++)
        ret << nameToType(names[i]);
    
    return ret;
}

bool JZScriptEnvironment::hasCTypeid(const QString& name) const
{
    return ctypeidToType(name) != Type_none;
}

int JZScriptEnvironment::ctypeidToType(const QString &name) const
{
    if (JZNodeType::isPointer(name))
    {
        int base_type = ctypeidToType(JZNodeType::baseType(name));
        return JZNodeType::pointerType(base_type);
    }

    if(name == typeid(bool).name())
        return Type_bool;
    else if(name == typeid(int).name())
        return Type_int;
    else if(name == typeid(int64_t).name())
        return Type_int64;
    else if(name == typeid(double).name())
        return Type_double;
    else if(name == typeid(QString).name())
        return Type_string;
    else    
        return m_objectManager.getIdByCTypeid(name);
}

QString JZScriptEnvironment::ctypeidToName(const QString& name) const
{
    return typeToName(ctypeidToType(name));
}

int JZScriptEnvironment::variantType(const QVariant &v) const
{
    return JZNodeType::variantType(v);
}

QString JZScriptEnvironment::variantTypeName(const QVariant &v) const
{    
    return typeToName(variantType(v));
}

bool JZScriptEnvironment::isVaildType(QString type) const
{
    QString bast_type = JZNodeType::baseType(type);
    if (JZNodeType::nameToType(type) != Type_none)
        return true;

    return m_objectManager.meta(type)
        || m_objectManager.enumMeta(type);
}

bool JZScriptEnvironment::isSameType(int src_type,int dst_type) const
{    
    if (JZNodeType::isPointer(src_type) && JZNodeType::isPointer(dst_type))
    {
        int base_src = JZNodeType::baseType(src_type);
        int base_dst = JZNodeType::baseType(dst_type);
        return isInherits(base_src, base_dst);
    }

    if (src_type == dst_type)
        return true;
    else if (src_type == Type_arg || dst_type == Type_arg)
        return true;
    else if ((JZNodeType::isEnum(src_type) && dst_type == Type_int) || (src_type == Type_int && JZNodeType::isEnum(dst_type)))
        return true;
    else if (src_type >= Type_class && dst_type >= Type_class)
        return isInherits(src_type, dst_type);
    
    return false;
}

int JZScriptEnvironment::isInherits(const QString &type1, const QString &type2) const
{
    int t1 = JZScriptEnvironment::nameToType(type1);
    int t2 = JZScriptEnvironment::nameToType(type2);
    return m_objectManager.isInherits(t1, t2);
}

int JZScriptEnvironment::isInherits(int type1,int type2) const
{
    if (type1 == type2)
        return true;

    return m_objectManager.isInherits(type1,type2);
}

bool JZScriptEnvironment::isFunctionTypeMatch(const JZFunctionDefine* func1, const JZFunctionDefine* func2) const
{
    if (func1->paramIn.size() != func2->paramIn.size()
        || func1->paramOut.size() != func2->paramOut.size())
        return false;
    if (func1->isVirtualFunction != func2->isVirtualFunction)
        return false;
    if (func1->isFlowFunction != func2->isFlowFunction)
        return false;

    for (int i = 0; i < func1->paramIn.size(); i++)
    {
        if (i == 0 && func1->isVirtualFunction)
        {
            if (!isInherits(func1->className, func2->className))
                return false;
        }
        else
        {
            if (func1->paramIn[i].type != func2->paramIn[i].type)
                return false;
        }
    }
    for (int i = 0; i < func1->paramOut.size(); i++)
    {
        if (func1->paramOut[i].type != func2->paramOut[i].type)
            return false;
    }
    return true;
}

JZParamDefine JZScriptEnvironment::paramDefine(QString name, int data_type, QString value) const
{
    JZParamDefine p;
    p.name = name;
    p.type = typeToName(data_type);
    p.value = value;
    Q_ASSERT(p.type != Type_none);
    return p;
}

bool JZScriptEnvironment::canConvert(int type1,int type2) const
{   
    if(type1 == type2)
        return true;

    bool is_type1_ptr = JZNodeType::isPointer(type1);
    bool is_type2_ptr = JZNodeType::isPointer(type2);
    if (is_type1_ptr && is_type2_ptr)
    {
        if (JZNodeType::baseType(type2) == Type_arg)
            return true;

        return isInherits(JZNodeType::baseType(type1), JZNodeType::baseType(type2));
    }
    else if (!is_type1_ptr && is_type2_ptr)
    {
        if (JZNodeType::baseType(type2) == Type_arg)
            return true;

        return isInherits(type1,JZNodeType::baseType(type2));
    }
    else if (is_type1_ptr && !is_type2_ptr)
    {
        return false;
    }
    
    if(type2 == Type_any)
        return true;
    else if(type1 == Type_auto || type2 == Type_auto)
        return true;
    else if(type1 == Type_arg || type2 == Type_arg)
        return true;    
    else if(JZNodeType::isNumber(type1) && JZNodeType::isNumber(type2))
        return true;
    else if (type1 == Type_bool && JZNodeType::isNumber(type2))
        return true;
    else if (JZNodeType::isNumber(type1) && type2 == Type_bool)
        return true;
    else if ((type1 == Type_int && JZNodeType::isEnum(type2)) || (JZNodeType::isEnum(type1) && type2 == Type_int))
        return true;
    else if (JZNodeType::isEnum(type1) && JZNodeType::isEnum(type2))
    {
        auto meta1 = m_objectManager.enumMeta(type1);
        auto meta2 = m_objectManager.enumMeta(type2);
        if ((meta1->isFlag() && meta1->flagEnum() == type2)
            || (meta2->isFlag() && meta2->flagEnum() == type1))
            return true;
        
        return false;
    }
    else if (type1 == Type_nullptr && type2 >= Type_class)
        return true;    
    else if (type1 >= Type_class && type2 >= Type_class)
    {
        if (m_objectManager.isInherits(type1, type2))
            return true;
    }

    int64_t id = makeConvertId(type1, type2);
    if (convertMap.contains(id))
        return true;

    return false;
}

bool JZScriptEnvironment::canConvertExplicitly(int from,int to) const
{
    if(canConvert(from,to))
        return true;

    if(from == Type_any)
        return true;
    if(from == Type_string && JZNodeType::isNumber(to))
        return true;
    if(JZNodeType::isNumber(from) && to == Type_string)
        return true;

    return false;
}

QVariant JZScriptEnvironment::tryConvertTo(const QVariant &v, int dst_type) const
{
    int src_type = variantType(v);
    if (src_type == dst_type)
        return v;

    if (!JZNodeType::isPointer(src_type) && JZNodeType::isPointer(dst_type)
        && isInherits(src_type, JZNodeType::baseType(dst_type)))
    {
        return JZNodeType::convertToPointer(v);
    }

    if (dst_type == Type_any)
    {
        JZNodeVariantAny any;
        any.value = v;
        return QVariant::fromValue(any);
    }
    else if (src_type == Type_any)
    {
        auto *ptr = (const JZNodeVariantAny*)v.data();
        return convertTo(ptr->value, dst_type);
    }
    else if (src_type == Type_nullptr && dst_type >= Type_class)
    {
        return QVariant::fromValue(JZNodeObjectNull());
    }
    else if (src_type >= Type_class && dst_type >= Type_class)
    {
        if (m_objectManager.isInherits(src_type, dst_type))
            return v;
    }
    else if (JZNodeType::isNumber(src_type) && JZNodeType::isNumber(dst_type))
    {
        if (src_type == Type_int)
        {
            int i = v.toInt();
            if (dst_type == Type_bool)
                return (bool)i;
            else if (dst_type == Type_int64)
                return (qint64)i;
            else
                return (double)i;
        }
        else if (src_type == Type_int64)
        {
            qint64 i = (qint64)v.toLongLong();
            if (dst_type == Type_bool)
                return (int)i;
            else if (dst_type == Type_int)
                return (int)i;
            else
                return (double)i;
        }
        else
        {
            double d = v.toDouble();
            if (dst_type == Type_bool)
                return (bool)d;
            else if (dst_type == Type_int)
                return (int)d;
            else
                return (qint64)d;
        }
    }
    else if (src_type == Type_bool && JZNodeType::isNumber(dst_type))
    {
        bool ret = v.toBool();
        if (dst_type == Type_int8)
            return QVariant::fromValue((int8_t)ret);
        else if (dst_type == Type_int16)
            return QVariant::fromValue((int16_t)ret);
        else if (dst_type == Type_int)
            return QVariant::fromValue((int)ret);
        else if (dst_type == Type_int64)
            return QVariant::fromValue((int64_t)ret);
        else if (dst_type == Type_uint8)
            return QVariant::fromValue((uint8_t)ret);
        else if (dst_type == Type_int16)
            return QVariant::fromValue((uint16_t)ret);
        else if (dst_type == Type_uint)
            return QVariant::fromValue((uint)ret);
        else if (dst_type == Type_uint64)
            return QVariant::fromValue((uint64_t)ret);
        else if (dst_type == Type_float)
            return QVariant::fromValue((float)ret);
        else if (dst_type == Type_double)
            return QVariant::fromValue((double)ret);
    }
    else if (JZNodeType::isNumber(src_type) && dst_type == Type_bool)
    {
        if (src_type == Type_int8)
            return (bool)v.value<int8_t>();
        else if (src_type == Type_int16)
            return (bool)v.value<int16_t>();
        else if (src_type == Type_int)
            return (bool)v.value<int>();
        else if (src_type == Type_int64)
            return (bool)v.value<int64_t>();
        else if (src_type == Type_uint8)
            return (bool)v.value<uint8_t>();
        else if (src_type == Type_uint16)
            return (bool)v.value<uint16_t>();
        else if (src_type == Type_uint)
            return (bool)v.value<uint>();
        else if (src_type == Type_uint64)
            return (bool)v.value<uint64_t>();
        else if (src_type == Type_float)
            return (bool)v.value<float>();
        else if (src_type == Type_double)
            return (bool)v.value<double>();
    }
    else if (src_type == Type_string && JZNodeType::isNumber(dst_type))
    {
        QString str = v.toString();
        if (dst_type == Type_bool)
            return str == "true";
        else if (dst_type == Type_int)
            return str.toInt();
        else if (dst_type == Type_int64)
            return str.toLongLong();
        else
            return str.toDouble();
    }
    else if (JZNodeType::isNumber(src_type) && dst_type == Type_string)
    {
        if (src_type == Type_bool)
            return v.toBool() ? "true" : "false";
        else if (src_type == Type_int)
            return QString::number(v.toInt());
        else if (src_type == Type_int64)
            return QString::number(v.toLongLong());
        else
            return QString::number(v.toDouble(), 'f');
    }

    int64_t cvt_id = makeConvertId(src_type, dst_type);
    auto it = convertMap.find(cvt_id);
    if (it != convertMap.end())
        return it.value()(this, v);
    
    return QVariant();
}

QVariant JZScriptEnvironment::convertTo(const QVariant &v, int dst_type) const
{
    QVariant value = tryConvertTo(v, dst_type);
    Q_ASSERT_X(value.isValid(), "Convert Failed", qUtf8Printable(variantType(v) + " -> " + typeToName(dst_type)));
    return value;
}

QVariant JZScriptEnvironment::clone(const QVariant &v) const
{
    return QVariant();
}

int JZScriptEnvironment::upType(int type1, int type2) const
{
    if (type1 > type2)
        std::swap(type1, type2);
    
    if (type1 == type2)
        return type1;
        
    if (type1 >= Type_bool && type1 <= Type_double
        && type2 >= Type_bool && type2 <= Type_double)
        return type2;

    if(type1 == Type_int && JZNodeType::isEnum(type2))
        return Type_int;

    if(type1 == Type_nullptr && type2 >= Type_class)
        return type2;

    if(type1 >= Type_class && type2 >= Type_class)
    {
        if(isInherits(type2,type1))
            return type1;
    }

    return Type_none;
}

int JZScriptEnvironment::upType(QList<int> types) const
{
    if (types.size() == 0)
        return Type_none;

    int type = types[0];
    for (int i = 1; i < types.size(); i++)
        type = upType(type, types[i]);
    
    return type;
}

int JZScriptEnvironment::matchType(QList<int> src_types,QList<int> dst_types) const
{   
    if(dst_types.size() == 1 && dst_types[0] == Type_arg)
        return upType(src_types);

    QList<int> dst_allow_type;    
    //在dst中选择能被所有src转换到的类型
    for (int i = 0; i < dst_types.size(); i++)
    {
        bool can_convert = true;
        for (int j = 0; j < src_types.size(); j++)
        {
            if (!canConvert(src_types[j], dst_types[i]))
            {
                can_convert = false;
                break;
            }                
        }
        if (can_convert)
            dst_allow_type << dst_types[i];
    }
    if (dst_allow_type.size() == 0)
        return Type_none;
    if (dst_allow_type.size() == 1)
        return dst_allow_type[0];    

    QList<int> dst_near_type;
    //对scr 选择dst_allow_type 里面最近似的类型
    for (int i = 0; i < src_types.size(); i++)
    {
        int near_type = Type_none;
        int near_dis = INT_MAX;
        for (int j = 0; j < dst_allow_type.size(); j++)
        {
            if (src_types[i] == dst_allow_type[j])
            {
                near_type = dst_allow_type[j];
                break;
            }
            else
            {
                int cur_near = 0;
                if (JZNodeType::isBaseOrEnum(src_types[i]))
                {
                    cur_near = abs(src_types[i] - dst_allow_type[j]);
                }
                else if(JZNodeType::isObject(src_types[i]))
                {

                }

                if (cur_near < near_dis)
                {
                    near_type = dst_allow_type[j];
                    near_dis = cur_near;
                }
            }
        }
        dst_near_type << near_type;
    }
    return upType(dst_near_type);
}

int JZScriptEnvironment::stringType(const QString &text) const
{
    if (text == "false" || text == "true")
        return Type_bool;
    else if (text == "null")
        return Type_nullptr;

    bool isInt = JZRegExpHelp::isInt(text);
    bool isHex = JZRegExpHelp::isHex(text);
    bool isFloat = JZRegExpHelp::isFloat(text);
    if (isInt || isHex)
        return Type_int;
    else if(isFloat)
        return Type_double;

    return Type_string;
}

QVariant JZScriptEnvironment::defaultValue(int type) const
{
    if(type == Type_any)
    {
        JZNodeVariantAny any;
        return QVariant::fromValue(any);
    }
    else if(type == Type_bool)
        return false;
    else if(type == Type_int)
        return 0;
    else if(type == Type_int64)
        return (qint64)0;
    else if(type == Type_double)
        return (double)0.0;
    else if(type == Type_string)
        return QString();
    else if(type == Type_function)
        return QVariant::fromValue(JZFunctionPointer());
    else if(type == Type_nullptr)
        return QVariant::fromValue(JZNodeObjectNull());
    else if(type >= Type_enum && type < Type_class)
    {
        auto meta = m_objectManager.enumMeta(type);
        return meta->defaultValue();
    }

    Q_ASSERT_X(0,"Type ",qUtf8Printable(typeToName(type)));
    return QVariant();
}

QString JZScriptEnvironment::defaultValueString(int type) const
{
    if (type == Type_any)
        return QString();
    else if (type == Type_bool)
        return "false";
    else if (type == Type_int || type == Type_int64 || type == Type_double)
        return "0";
    else if (type == Type_string)
        return QString();
    else if (type == Type_function)
        return "null";
    else if (type == Type_nullptr)
        return "null";
    else if (type >= Type_enum && type < Type_class)
    {
        auto meta = m_objectManager.enumMeta(type);
        return meta->defaultKey();
    }
    else
        return "{}";
}

QVariant JZScriptEnvironment::initValue(int type, const QString &text) const
{
    if (text.isEmpty())
        return JZScriptEnvironment::defaultValue(type);

    if (type == Type_string)
    {
        return text;
    }
    else if (type == Type_bool)
    {
        if (text == "false")
            return false;
        else if (text == "true")
            return true;
    }
    else if (type == Type_int || type == Type_int64 || type == Type_double)
    {        
        bool isInt = JZRegExpHelp::isInt(text);
        bool isHex = JZRegExpHelp::isHex(text);
        bool isFloat = JZRegExpHelp::isFloat(text);
        
        if (isHex)
            return text.toInt(nullptr, 16);
        else
        {
            if (type == Type_int || type == Type_int64)
            {
                if (isFloat)
                    return (int)text.toDouble();
                else if (isInt)
                    return text.toInt();
            }
            else if (type == Type_double)
            {
                if (isFloat || isInt)
                    return text.toDouble();
            }
            else
            {
                if (isInt)
                    return text.toInt();
                if (isFloat)
                    return text.toDouble();
            }
        }
    }
    else if(type == Type_function)
    {   
        JZFunctionPointer func;
        func.functionName = text;
        return QVariant::fromValue(func);
    }
    else if(type == Type_nullptr)
    {
        if(text == "null")
            return QVariant::fromValue(JZNodeObjectNull());
    }
    else if(type >= Type_enum && type < Type_class)
    {
        auto enum_meta = m_objectManager.enumMeta(type);
        if(!enum_meta->hasKey(text))
            return QVariant();

        return enum_meta->keyToValue(text);
    }    

    Q_ASSERT_X(0,"Type ",qUtf8Printable(typeToName(type)));
    return true;
}

bool JZScriptEnvironment::isListType(int type) const
{
    if (!hasType(type))
        return false;

    QString type_name = typeToName(type);
    return type_name.startsWith("QList<");
}

bool JZScriptEnvironment::listValueType(int type, int& value_type) const
{
    if (!isListType(type))
        return false;

    QString type_name = typeToName(type);

    QString list_value;
    if (!JZNodeType::listValueType(type_name, list_value))
        return false;

    value_type = nameToType(list_value);
    return value_type != Type_none;
}

bool JZScriptEnvironment::isMapType(int type) const
{
    if (!hasType(type))
        return false;

    QString type_name = typeToName(type);
    return type_name.startsWith("QMap<");
}

bool JZScriptEnvironment::mapIteratorType(int type, int& iterator_type) const
{
    if (!isMapType(type))
        return false;

    QString type_name = typeToName(type);
    type_name = JZNodeType::mapIteratorType(type_name);
    int it_type = nameToType(type_name);
    return (it_type != Type_none);
}

bool JZScriptEnvironment::mapKeyValueType(int type, int& key_type, int& value_type) const
{
    if (!isMapType(type))
        return false;

    QString type_name = typeToName(type);

    QString map_key, map_value;
    if (!JZNodeType::mapValueType(type_name, map_key,map_value))
        return false;

    key_type = nameToType(map_key);
    value_type = nameToType(map_value);
    return (key_type != Type_none && value_type != Type_none);
}

void JZScriptEnvironment::registConvert(int from, int to, ConvertFunc func)
{
    int id = (int64_t)from << 32 | (int64_t)to;
    convertMap[id] = func;
}