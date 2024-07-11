﻿#include <QMetaObject>
#include <QApplication>
#include <QDebug>
#include <QTime>
#include "JZNodeObject.h"
#include "JZNodeUiLoader.h"
#include "JZNodeQtWrapper.h"
#include "JZNodeEngine.h"
#include "JZNodeBind.h"

JZNodeVariantAny JZObjectClone(const JZNodeVariantAny &v)
{
    JZNodeVariantAny ret;
    ret.value = JZNodeType::clone(v.value);
    return ret;
}

int JZClassId(const QString &name)
{
    return JZNodeObjectManager::instance()->getClassId(name);
}

QString JZClassName(int id)
{
    return JZNodeObjectManager::instance()->getClassName(id);
}

void JZObjectConnect(JZNodeObject *sender, JZFunctionPointer single, JZNodeObject *recv, JZFunctionPointer slot)
{
    auto s = JZNodeObjectManager::instance()->single(single.functionName);
    auto func = JZNodeFunctionManager::instance()->function(slot.functionName);
    Q_ASSERT(s && func && JZNodeType::sigSlotTypeMatch(s,func));
    if (s->csingle)
        s->csingle->connect(sender,recv,func->name);
    else
        sender->singleConnect(s->name,recv,func->name);
}

void JZObjectDisconnect(JZNodeObject *sender, JZFunctionPointer single, JZNodeObject *recv, JZFunctionPointer slot)
{
    auto s = JZNodeObjectManager::instance()->single(single.functionName);
    auto func = JZNodeFunctionManager::instance()->function(slot.functionName);
    Q_ASSERT(s && func);
    if (s->csingle)
        s->csingle->disconnect(sender,recv,func->name);
    else
        sender->singleDisconnect(s->name,recv,func->name);
}

bool JZObjectIsList(JZNodeObject *obj)
{
    return obj->className().startsWith("List<");
}

bool JZObjectIsMap(JZNodeObject *obj)
{
    return obj->className().startsWith("Map<");
}

bool JZObjectIsSet(JZNodeObject *obj)
{
    return obj->className().startsWith("Set<");
}

void JZNodeDisp(const JZNodeVariantAny &v)
{    
    QString str = JZNodeType::debugString(v.value);
    JZScriptLog(str);    
}

//CMeta
CMeta::CMeta()
{
    isCopyable = false;
    isAbstract = false;

    create = nullptr;
    copy = nullptr;
    destory = nullptr;
}

//JZNodeObjectDefine
JZNodeObjectDefine::JZNodeObjectDefine()
{
    id = -1;
    isCObject = false;
    isUiWidget = false;    
    valueType = false;
}

QString JZNodeObjectDefine::fullname() const
{
    QString name;
    if(!nameSpace.isEmpty())
        name = nameSpace + "::";
    name += className;
    return name;
}

void JZNodeObjectDefine::addParam(const JZParamDefine &def)
{
    params[def.name] = def;
}

void JZNodeObjectDefine::removeParam(const QString &name)
{
    params.remove(name);
}

QStringList JZNodeObjectDefine::paramList(bool hasParent) const
{
    QStringList list = this->params.keys();
    if (hasParent)
    {
        auto def = this->super();
        while (def)
        {
            list << def->params.keys();
            def = def->super();
        }
    }
    return list;
}

const JZParamDefine *JZNodeObjectDefine::param(const QString &name) const
{
    QStringList list = name.split(".");

    auto def = this;
    for(int i = 0; i < list.size(); i++)
    {
        while(def)
        {
            auto it = def->params.find(list[i]);
            if(it != def->params.end())
            {
                if(i == list.size() - 1)
                    return &it.value();
                else
                {
                    def = JZNodeObjectManager::instance()->meta(it->dataType());
                    break;
                }
            }
            def = def->super();
        }
        if(!def)
            break;
    }
    return nullptr;
}

JZFunctionDefine JZNodeObjectDefine::initStaticFunction(QString function)
{
    JZFunctionDefine def;
    def.name = function;
    def.className = className;
    return def;  
}

JZFunctionDefine JZNodeObjectDefine::initMemberFunction(QString function)
{
    JZFunctionDefine def;
    def.name = function;
    def.className = className;
    def.paramIn.push_back(JZParamDefine("this",className));
    return def;    
}

JZFunctionDefine JZNodeObjectDefine::initVirtualFunction(QString name)
{
    auto func_def = function(name);
    Q_ASSERT(func_def);

    JZFunctionDefine new_def = *func_def;
    new_def.isCFunction = false;
    new_def.className = className;
    return new_def;
}

JZFunctionDefine JZNodeObjectDefine::initSlotFunction(QString name,QString single)
{
    const JZParamDefine *param_def = param(name);
    Q_ASSERT(param_def);

    auto param_meta = JZNodeObjectManager::instance()->meta(param_def->dataType());
    Q_ASSERT(param_meta && param_meta->single(single));
    
    auto s = param_meta->single(single);
    QString func_name = "on_" + name + "_" + single;
    JZFunctionDefine func_def = initMemberFunction(func_name);
    for(int i = 0; i < s->paramOut.size(); i++)
    {
        func_def.paramIn.push_back(s->paramOut[i]);
    }
    return func_def;
}

void JZNodeObjectDefine::addFunction(const JZFunctionDefine &def)
{
    Q_ASSERT(!def.name.contains(".") && !def.name.contains("::"));
    functions.push_back(def);    
}

void JZNodeObjectDefine::removeFunction(const QString &function)
{
    int index = indexOfFunction(function);
    if(index != -1)
        functions.removeAt(index);
}

int JZNodeObjectDefine::indexOfFunction(const QString &function) const
{
    for (int i = 0; i < functions.size(); i++)
    {
        if (functions[i].name == function)
            return i;
    }
    return -1;
}

bool JZNodeObjectDefine::checkFunction(const QString &function,QString &error) const
{
    int count = 0;
    const JZFunctionDefine *func = nullptr;
    for (int i = 0; i < functions.size(); i++)        
    {
        if(function == functions[i].name)
        {
            func = &functions[i];
            count++;
        }
    }
    if(count > 1)
    {
        error = "存在重名函数";
        return false;
    }

    const JZNodeObjectDefine *def = this->super();
    while (def)
    {
        for (int i = 0; i < def->functions.size(); i++)  
        {
            if(def->functions[i].name == function)
            {
                const JZFunctionDefine *super_func = &def->functions[i];
                if(!func->isVirtualFunction)
                {
                    error = "父类存在重名函数";
                    return false;
                }

                if(!JZNodeType::functionTypeMatch(func,super_func))
                {
                    error = "函数签名不匹配";
                    return false;
                }
            }
        }         
        def = def->super();        
    }    
    return true;
}

const JZFunctionDefine *JZNodeObjectDefine::function(const QString &name) const
{
    int index = indexOfFunction(name);
    if(index >= 0)
        return &functions[index];

    auto def = super();
    if(def)
        return def->function(name);
    else
        return nullptr;
}

QStringList JZNodeObjectDefine::functionList() const
{
    QSet<QString> set;
    auto def = this;
    while (def)
    {
        for (int i = 0; i < def->functions.size(); i++)        
            set << def->functions[i].name;

        def = def->super();        
    }    
    return set.toList();
}

QStringList JZNodeObjectDefine::JZNodeObjectDefine::singleList() const
{
    QSet<QString> set;
    auto def = this;
    while(def)
    {
        for (int i = 0; i < def->singles.size(); i++)        
            set << def->singles[i].name;

        def = def->super();
    }
    return set.toList();
}

const JZSingleDefine *JZNodeObjectDefine::single(const QString &function) const
{
    for(int i = 0; i < singles.size(); i++)
    {
        if(singles[i].name == function)
            return &singles[i];
    }

    auto def = super();
    if(def)
        return def->single(function);
    else
        return nullptr;
}

QStringList JZNodeObjectDefine::slotList() const
{
    QSet<QString> set;
    auto def = this;
    while (def)
    {
        for (int i = 0; i < def->functions.size(); i++)
        {
            if(def->functions[i].isSlot)
                set << def->functions[i].name;
        } 
        def = def->super();        
    }    
    return set.toList();
}

const JZFunctionDefine *JZNodeObjectDefine::slot(const QString &name) const
{
    return function(name);
}

const JZNodeObjectDefine *JZNodeObjectDefine::super() const
{
    if(superName.isEmpty())
        return nullptr;

    return JZNodeObjectManager::instance()->meta(superName);
}

bool JZNodeObjectDefine::isInherits(int type) const
{
    return JZNodeObjectManager::instance()->isInherits(id,type);
}

bool JZNodeObjectDefine::isInherits(const QString &name) const
{
    return isInherits(JZNodeObjectManager::instance()->getClassId(name));
}

bool JZNodeObjectDefine::isCopyable() const
{
    if(isCObject)
        return cMeta.isCopyable;
    else
    {
        for(auto &v: params)
        {
            if(JZNodeType::isObject(v.dataType()))
            {                
                bool ret = JZNodeObjectManager::instance()->meta(v.dataType())->isCopyable();
                if(!ret)
                    return false;
            }
        }
    }    
    return true;
}

bool JZNodeObjectDefine::isValueType() const
{
    return valueType;
}

QDataStream &operator<<(QDataStream &s, const JZNodeObjectDefine &param)
{
    Q_ASSERT(!param.isCObject);

    s << param.id;
    s << param.nameSpace;
    s << param.className;
    s << param.superName;
    s << param.valueType;

    s << param.params;
    s << param.functions;
    s << param.singles;

    s << param.isCObject;
    s << param.isUiWidget;
    s << param.widgetXml;
    s << param.widgetParams;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeObjectDefine &param)
{
    s >> param.id;
    s >> param.nameSpace;
    s >> param.className;
    s >> param.superName;
    s >> param.valueType;

    s >> param.params;
    s >> param.functions;
    s >> param.singles;
    
    s >> param.isCObject;
    s >> param.isUiWidget;
    s >> param.widgetXml;
    s >> param.widgetParams;
    return s;
}

//JZNodeCObjectDelcare
JZNodeCObjectDelcare::JZNodeCObjectDelcare()
{
    id = -1;
}

QDataStream &operator<<(QDataStream &s, const JZNodeCObjectDelcare &param)
{
    s << param.className;
    s << param.id; 
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeCObjectDelcare &param)
{
    s >> param.className;
    s >> param.id; 
    return s;
}

//JZObjectNull
JZObjectNull::JZObjectNull()
{
}

QDataStream &operator<<(QDataStream &s, const JZObjectNull &param)
{
    return s;
}
QDataStream &operator>>(QDataStream &s, JZObjectNull &param)
{
    return s;
}

//JZNodeObject
JZNodeObject::JZNodeObject(JZNodeObjectDefine *def)
{    
    m_define = def;
    m_cobj = nullptr;
    m_cobjOwner = false;
    m_isNull = true;
}

JZNodeObject::~JZNodeObject()
{    
    if(m_cobjOwner && m_define->isCObject)
    {
        if (isInherits("Object"))
        {
            auto obj = (QObject*)m_cobj;
            if(obj->parent())
                return;
        }
        m_define->cMeta.destory(m_cobj);
    }
}

bool JZNodeObject::isNull() const
{
    return m_isNull;
}

bool JZNodeObject::isInherits(int type) const
{
    return JZNodeObjectManager::instance()->isInherits(m_define->id,type);
}

bool JZNodeObject::isInherits(const QString &name) const
{
    return isInherits(JZNodeObjectManager::instance()->getClassId(name));
}

bool JZNodeObject::isCopyable() const
{
    return m_define->isCopyable();
}

bool JZNodeObject::isCObject() const
{
    return m_define->isCObject;
}

const QString &JZNodeObject::className() const
{
    return m_define->className;
}

int JZNodeObject::type() const
{
    return m_define->id;
}

const JZNodeObjectDefine *JZNodeObject::meta() const
{
    return m_define;
}

bool JZNodeObject::getParamRef(const QString &name,QVariant *&ref,QString &error)
{
    QStringList list = name.split(".");
    JZNodeObject *obj = this;
    for(int i = 0; i < list.size() - 1; i++)
    {
        auto it = obj->m_params.find(list[i]);
        if(it == obj->m_params.end())
        {
            error = "no such element";
            return false;
        }

        QVariant *v = it->data();
        if(!isJZObject(*v))
        {
            error = "not a object";
            return false;
        }
        obj = toJZObject(*v);
    }
    if(!obj->m_params.contains(list.back()))
    {
        error = "no such element";
        return false;
    }
    ref = obj->m_params[list.back()].data();
    return true;
}

bool JZNodeObject::hasParam(const QString &name) const
{
    JZNodeObject *obj = const_cast<JZNodeObject *>(this);
    QVariant *ptr = nullptr;
    QString error;
    return obj->getParamRef(name,ptr,error);
}

QStringList JZNodeObject::paramList() const
{
    return m_define->paramList(true);
}

const QVariant &JZNodeObject::param(const QString &name) const
{
    JZNodeObject *obj = const_cast<JZNodeObject *>(this);
    QVariant *ptr = nullptr;
    QString error;
    if(!obj->getParamRef(name,ptr,error))
        throw std::runtime_error(qPrintable(error));
    
    return *ptr;
}

void JZNodeObject::setParam(const QString &name,const QVariant &value)
{
    QVariant *ptr = nullptr;
    QString error;
    if(!getParamRef(name,ptr,error))
        throw std::runtime_error(qPrintable(error));
    *ptr = value;
}

QVariant *JZNodeObject::paramRef(const QString &name)
{
    QVariant *ptr = nullptr;
    QString error;
    getParamRef(name,ptr,error);
    return ptr;
}

const JZFunctionDefine *JZNodeObject::function(const QString &name) const
{
    return m_define->function(name);
}

QStringList JZNodeObject::functionList() const
{
    return m_define->functionList();
}

const JZSingleDefine *JZNodeObject::single(QString function) const
{
    return m_define->single(function);
}

QStringList JZNodeObject::singleList() const
{
    return m_define->singleList();
}

void JZNodeObject::onRecvDestory(QObject *obj)
{   
    for(int i = m_connectList.size() - 1; i >= 0; i--)
    {
        if(m_connectList[i].recv == obj)
            m_connectList.removeAt(i);
    }
}

int JZNodeObject::singleConnectCount(JZNodeObject *recv)
{
    int count = 0;
    for(int i = 0; i < m_connectList.size(); i++)
    {
        if(m_connectList[i].recv == recv)
            count++;
    }
    return count;
}

void JZNodeObject::singleConnect(QString sig,JZNodeObject *recv,QString slot)
{
    if(singleConnectCount(recv) == 1)
    {
        connect(recv,&QObject::destroyed,this,&JZNodeObject::onRecvDestory);
        connect(this,&JZNodeObject::sig,recv,&JZNodeObject::onSig);
    }
}

void JZNodeObject::singleDisconnect(QString sig,JZNodeObject *recv,QString slot)
{
    if(singleConnectCount(recv) == 0)
    {
        disconnect(recv,&QObject::destroyed,this,&JZNodeObject::onRecvDestory);
        disconnect(this,&JZNodeObject::sig,recv,&JZNodeObject::onSig);
    }
}

void JZNodeObject::singleEmit(QString sig_name,const QVariantList &params)
{
    for(int i = 0; i < m_connectList.size(); i++)
    {
        if(m_connectList[i].single == sig_name)
            emit sig(m_connectList[i].slot,params);
    }
}

void JZNodeObject::updateUiWidget(QWidget *widget)
{
    Q_ASSERT(isInherits("Widget"));

    QObject *obj = (QObject*)m_cobj;
    auto inst = JZNodeObjectManager::instance();
    
    auto def = m_define;
    for(int i = 0; i < m_define->widgetParams.size(); i++)
    {
        auto &param_def = m_define->widgetParams[i];

        QWidget *w = obj->findChild<QWidget*>(param_def.name);
        if (w)
        {
            JZNodeObject *jzobj = new JZNodeObject(inst->meta(param_def.dataType()));
            jzobj->setCObject(w, false);
            JZNodeObjectPtr ptr(jzobj,true);
            m_params[param_def.name] = QVariantPtr(new QVariant(QVariant::fromValue(ptr)));
        }
    }

    QStringList list = m_define->functionList();
    for(int i = 0; i < list.size(); i++)
    {
        QString func = list[i];
        if(!func.startsWith("on_"))
            continue;

        int idx1 = 3;
        int idx2 = func.lastIndexOf("_");
        if(idx2 == -1 || idx2 == idx1)
            continue;

        QString param_name = func.mid(3,idx2 - idx1);
        if(!hasParam(param_name))
        {
            qDebug() << "connect slot by name no param: " + param_name;
            continue;
        }

        QString sig = func.mid(idx2 + 1);
        auto obj = toJZObject(param(param_name));
        if(!obj)
        {
            qDebug() << "connect slot by name object not init";
            continue;
        }

        auto sig_func = obj->single(sig);
        if(!sig_func)
        {
            qDebug() << "connect slot by name no single: " + sig;
            continue;
        }

        auto slot_func = function(func);
        if(!JZNodeType::sigSlotTypeMatch(sig_func,slot_func))
        {
            qDebug() << "connect slot by name slot function no match." << sig_func->delcare() << slot_func->delcare();
            continue;
        }

        JZFunctionPointer sig_func_ptr;
        sig_func_ptr.functionName = sig_func->fullName();

        JZFunctionPointer slot_func_ptr;
        slot_func_ptr.functionName = slot_func->fullName();
        JZObjectConnect(obj,sig_func_ptr,this,slot_func_ptr);
    }
}

void JZNodeObject::onSig(QString name,const QVariantList &params)
{
    auto func = function(name);
    QString full_name = func->fullName();

    QVariantList in,out;
    JZNodeObjectPtr ptr(this,false);
    in << QVariant::fromValue(ptr);
    for(int i = 0; i < func->paramIn.size() - 1; i++)
        in << params[i];
    JZScriptInvoke(full_name,in,out);
}

void JZNodeObject::setCObject(void *obj,bool owner)
{
    Q_ASSERT(obj);
    if(m_cobj && m_cobjOwner)
        m_define->cMeta.destory(obj);

    m_cobj = obj;
    m_cobjOwner = owner;
    m_isNull = false;

    if(m_define->isInherits("Object"))
        ((QObject*)m_cobj)->setProperty("JZObject",QVariant::fromValue<void*>(this));
}

void *JZNodeObject::cobj() const
{
    return m_cobj;
}

void JZNodeObject::setCOwner(bool owner)
{
    m_cobjOwner = owner;
}

//JZNodeObjectRef
JZNodeObjectPtr::JZNodeObjectPtrData::JZNodeObjectPtrData()
{
    isOwner = false;
    object = nullptr;
}

JZNodeObjectPtr::JZNodeObjectPtrData::~JZNodeObjectPtrData()
{
    if(isOwner && object)
        delete object;
}

JZNodeObjectPtr::JZNodeObjectPtr()
{
}

JZNodeObjectPtr::JZNodeObjectPtr(JZNodeObject *obj,bool isOwner)
{
    data = QSharedPointer<JZNodeObjectPtrData>(new JZNodeObjectPtrData());
    data->isOwner = isOwner;
    data->object = obj;
}

JZNodeObjectPtr::~JZNodeObjectPtr()
{
}

JZNodeObject *JZNodeObjectPtr::object() const
{
    return data->object;
}

void JZNodeObjectPtr::releaseOwner()
{
    data->isOwner = false;
}

QDebug operator<<(QDebug dbg, const JZNodeObjectPtr ptr)
{
    Q_ASSERT(ptr.object());
    dbg << JZNodeType::debugString(ptr.object());
    return dbg;
}

bool isJZObject(const QVariant &v)
{
    return (v.userType() == qMetaTypeId<JZNodeObjectPtr>());
}

JZNodeObject* toJZObject(const QVariant &v)
{
    if (v.userType() == qMetaTypeId<JZNodeObjectPtr>())
    {
        auto ptr = (JZNodeObjectPtr*)v.data();
        return ptr->object();
    }
    else
    {      
        Q_ASSERT(0);
        return nullptr;
    }
}

JZNodeObjectPtr toJZObjectPtr(const QVariant &v)
{
    if (v.userType() == qMetaTypeId<JZNodeObjectPtr>())
    {
        return v.value<JZNodeObjectPtr>();
    }
    else
    {      
        Q_ASSERT(0);
        return JZNodeObjectPtr();
    }
}

JZObjectNull* toJZNullptr(const QVariant &v)
{
    if (v.userType() == qMetaTypeId<JZObjectNull>())
        return (JZObjectNull*)v.data();
    else
        return nullptr;
}

JZNodeObject* qobjectToJZObject(QObject *obj)
{
    auto ptr = obj->property("JZObject").value<void*>();
    if(!ptr)
        return nullptr;
    else
        return (JZNodeObject*)ptr;
}

JZNodeObject *objectFromString(const QString &className,const QString &text)
{
    QString func = className + "." + "__fromString__";
    QVariantList in,out;
    in << text;
    if(!g_engine->call(func,in,out))
        return nullptr;

    JZNodeObjectPtr ptr = out[0].value<JZNodeObjectPtr>();
    ptr.releaseOwner();
    return ptr.object();
}   


//JZNodeObjectManager
JZNodeObjectManager *JZNodeObjectManager::instance()
{
    static JZNodeObjectManager inst;
    return &inst;
}

JZNodeObjectManager::JZNodeObjectManager()
{        
    m_objectId = Type_internalObject;
    m_enumId = Type_internalEnum;    
}

JZNodeObjectManager::~JZNodeObjectManager()
{    
    m_metas.clear();    
}

void JZNodeObjectManager::init()
{  
    QMetaType::registerDebugStreamOperator<JZNodeObjectPtr>();           

    jzbind::ClassBind<JZNodeVariantAny> cls_any(Type_any, "any");
    cls_any.def("type", false, &JZNodeVariantAny::type);
    cls_any.regist();

    jzbind::ClassBind<JZFunctionPointer> cls_function(Type_function,"function");
    cls_function.regist();

    registQtClass();    
    initFunctions();    
    
    m_objectId = Type_userObject;
}

void JZNodeObjectManager::initFunctions()
{    
    JZNodeFunctionManager::instance()->registCFunction("print",true,jzbind::createFuncion(JZNodeDisp));
    JZNodeFunctionManager::instance()->registCFunction("clone", true, jzbind::createFuncion(JZObjectClone));
}

int JZNodeObjectManager::getId(const QString &type_name)
{
    int type = getClassId(type_name);
    if (type != Type_none)
        return type;

    type = getEnumId(type_name);
    if (type != Type_none)
        return type;

    return type;
}

int JZNodeObjectManager::getIdByCType(const QString &type_name)
{
    return m_typeidMetas.value(type_name, Type_none);
}

int JZNodeObjectManager::delcare(const QString &name, int id)
{
    JZNodeObjectDefine def;
    def.className = name;
    def.id = id;
    return regist(def);    
}

int JZNodeObjectManager::delcareCClass(const QString &name, const QString &c_typeid, int id)
{    
    JZNodeObjectDefine def;
    def.className = name;
    def.id = id;
    return registCClass(def, c_typeid);    
}

int JZNodeObjectManager::regist(const JZNodeObjectDefine &info)
{
    Q_ASSERT(!info.className.isEmpty() && (info.superName.isEmpty() || getClassId(info.superName) != -1));    
    
    JZNodeObjectDefine *def = new JZNodeObjectDefine();
    *def = info;
    if(info.id != -1)
    {
        Q_ASSERT(!meta(info.className) || meta(info.className)->id == info.id);
        def->id = info.id;
        m_objectId = qMax(m_objectId,def->id + 1);
    }
    else
    {
        Q_ASSERT(!meta(info.className));
        def->id = m_objectId++;
    }

    m_metas.insert(def->id ,QSharedPointer<JZNodeObjectDefine>(def));
    return def->id;
}

void JZNodeObjectManager::replace(const JZNodeObjectDefine &define)
{
    Q_ASSERT(m_metas.contains(define.id));
        
    JZNodeObjectDefine *ptr = m_metas[define.id].data();
    *ptr = define;
}

int JZNodeObjectManager::registCClass(const JZNodeObjectDefine &define,const QString &ctype_id)
{
    int id = regist(define);
    m_typeidMetas[ctype_id] = id;
    return id;
}

int JZNodeObjectManager::registEnum(const JZNodeEnumDefine &define)
{
    int id = -1;
    if (define.type() == -1)
        id = m_enumId++;
    else
        id = define.type();

    m_enums[id] = define;    
    m_enums[id].setType(id);
    return id;
}

int JZNodeObjectManager::registCEnum(const JZNodeEnumDefine &define, const QString &ctype_id)
{
    int id = registEnum(define);
    m_typeidMetas[ctype_id] = id;    
    return id;
}

void JZNodeObjectManager::unregist(int name)
{
    m_metas.remove(name);
}

void JZNodeObjectManager::setQObjectType(const QString &name,int id)
{
    m_qobjectId[name] = id;
}

void JZNodeObjectManager::clearUserReigst()
{
    auto it = m_metas.begin();
    while(it != m_metas.end())
    {
        if(it.value()->id >= Type_userObject)
            it = m_metas.erase(it);
        else
            it++;
    }
    m_objectId = Type_userObject;
}

JZEnum JZNodeObjectManager::createEnum(int enumType)
{
    JZEnum e;
    e.type = enumType;
    e.value = enumMeta(enumType)->defaultValue();
    return e;
}

JZNodeObjectDefine *JZNodeObjectManager::meta(int id)
{
    return m_metas.value(id,nullptr).data();
}

JZNodeObjectDefine *JZNodeObjectManager::meta(const QString &className)
{
    if(className.isEmpty())
        return nullptr;

    return meta(getClassId(className));
}

const JZSingleDefine *JZNodeObjectManager::single(const QString &name)
{
    QStringList list = name.split(".");
    if (list.size() != 2)
        return nullptr;

    auto cls = meta(list[0]);
    if (!cls)
        return nullptr;

    return cls->single(list[1]);
}

int JZNodeObjectManager::getQObjectType(const QString &name)
{
    return m_qobjectId.value(name,Type_none);
}

int JZNodeObjectManager::getClassId(const QString &class_name)
{
    auto it = m_metas.begin();
    while(it != m_metas.end())
    {
        if(it.value()->className == class_name)
            return it.key();

        it++;
    }
    return Type_none;
}

bool JZNodeObjectManager::isInherits(const QString &class_name, const QString &super_name)
{
    int class_type = getClassId(class_name);
    int super_type = getClassId(super_name);    
    return isInherits(class_type, super_type);
}

bool JZNodeObjectManager::isInherits(int class_name,int super_name)
{
    if(class_name == super_name)
        return true;

    auto def = meta(class_name);
    while(def)
    {
        int super_id = getClassId(def->superName);
        if(super_id == Type_none)
            break;
        else if(super_id == super_name)
            return true;

        def = meta(super_id);
    }
    return false;
}

QStringList JZNodeObjectManager::getClassList()
{
    QStringList list;

    auto it = m_metas.begin();
    while (it != m_metas.end())
    {
        list << (it.value()->className);
        it++;
    }
    return list;
}

QString JZNodeObjectManager::getClassName(int type_id)
{
    auto it = m_metas.find(type_id);
    if(it == m_metas.end())
        return QString();

    return it.value()->className;
}

JZNodeEnumDefine *JZNodeObjectManager::enumMeta(int type_id)
{
    auto it = m_enums.find(type_id);
    if (it == m_enums.end())
        return nullptr;

    return &it.value();
}

JZNodeEnumDefine *JZNodeObjectManager::enumMeta(const QString &enumName)
{
    auto it = m_enums.begin();
    while (it != m_enums.end())
    {
        if (it->name() == enumName)
            return &it.value();
        it++;
    }
    return nullptr;
}

QString JZNodeObjectManager::getEnumName(int type_id)
{
    auto it = m_enums.find(type_id);
    if (it == m_enums.end())
        return QString();

    return it->name();
}

int JZNodeObjectManager::getEnumId(const QString &enumName)
{
    auto it = m_enums.begin();
    while (it != m_enums.end())
    {
        if (it->name() == enumName)
            return it.key();
        it++;
    }
    return -1;
}

QStringList JZNodeObjectManager::getEnumList()
{
    QStringList result;
    auto it = m_enums.begin();
    while (it != m_enums.end())
    {
        result << it->name();
        it++;
    }

    return result;
}

void JZNodeObjectManager::copy(JZNodeObject *dst,JZNodeObject *src)
{
    Q_ASSERT(dst->isCopyable());

    if(src->isCObject())    
    {
        src->meta()->cMeta.copy(dst->cobj(),src->cobj());
        return;
    }

    auto list = dst->paramList();
    for(int i = 0; i < list.size(); i++)
    {        
        QVariant v = dst->param(list[i]);
        if(isJZObject(v))
        {
            JZNodeObject *src_ptr = toJZObject(src->param(list[i]));
            JZNodeObject *dst_ptr = toJZObject(v);
            copy(dst_ptr,src_ptr);
        }
        else
            dst->setParam(list[i],v);        
    }
}

void JZNodeObjectManager::create(const JZNodeObjectDefine *def,JZNodeObject *obj)
{
    if (def->isCObject)
    {
        auto cobj = def->cMeta.create();
        obj->setCObject(cobj,true);
        return;
    }        

    if (def->isUiWidget)
    {
        JZNodeUiLoader loader;
        QWidget *widget = loader.create(def->widgetXml);
        Q_ASSERT(widget); 

        obj->setCObject(widget, false);
        obj->updateUiWidget(widget);
    }
    else
    {
        if (!def->superName.isEmpty())
            create(def->super(), obj);
    }

    auto list = def->paramList(true);
    for (int i = 0; i < list.size(); i++)
    {
        auto param = def->param(list[i]);
        if (!obj->m_params.contains(param->name)) //可能父构造函数已经创建
        {
            auto ptr = QVariantPtr(new QVariant());
            *ptr = JZNodeType::initValue(param->dataType(), param->value);
            obj->m_params[param->name] = ptr;
        }
    }
}


JZNodeObject* JZNodeObjectManager::createNull(int type)
{
    JZNodeObjectDefine *def = meta(type);
    Q_ASSERT(def);

    JZNodeObject *obj = new JZNodeObject(def);
    return obj;
}

JZNodeObject* JZNodeObjectManager::createNull(const QString &name)
{
    int id = getClassId(name);
    return create(id);
}

JZNodeObject* JZNodeObjectManager::create(int type)
{
    JZNodeObjectDefine *def = meta(type);
    Q_ASSERT(def);

    JZNodeObject *obj = new JZNodeObject(def);    
    create(def,obj);
    obj->m_isNull = false;
    return obj;
}

JZNodeObject* JZNodeObjectManager::create(const QString &name)
{
    int id = getClassId(name);
    return create(id);
}

JZNodeObject* JZNodeObjectManager::createByTypeid(const QString &type_id)
{
    Q_ASSERT(m_typeidMetas.contains(type_id));

    int className = m_typeidMetas[type_id];
    return create(className);
}

JZNodeObject* JZNodeObjectManager::createRefrence(int type_id, void *ptr, bool owner)
{
    auto def = meta(type_id);
    JZNodeObject *obj = new JZNodeObject(def);
    obj->setCObject(ptr, owner);
    return obj;
}

JZNodeObject* JZNodeObjectManager::createRefrence(const QString &type_name,void *ptr, bool owner)
{
    int type_id = getId(type_name);
    return createRefrence(type_id,ptr,owner);
}

JZNodeObject* JZNodeObjectManager::createRefrenceByTypeid(const QString &ctype_id,void *ptr, bool owner)
{
    if(!m_typeidMetas.contains(ctype_id))
        return nullptr;
    
    return createRefrence(m_typeidMetas[ctype_id],ptr,owner);
}

JZNodeObject* JZNodeObjectManager::clone(JZNodeObject *other)
{
    Q_ASSERT(other->isCopyable());

    JZNodeObject* ret = create(other->type());
    copy(ret,other);    
    return ret;
}
