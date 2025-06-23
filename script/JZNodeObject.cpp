#include <QMetaObject>
#include <QApplication>
#include <QDebug>
#include <QTime>
#include <QTreeWidgetItem>
#include <QListWidgetItem>
#include <QTableWidgetItem>
#include <QPointer>
#include "JZNodeObject.h"
#include "JZNodeQtWrapper.h"
#include "JZNodeEngine.h"
#include "JZNodeBind.h"
#include "JZScriptEnvironment.h"
#include "runtime/JZNodeUiLoader.h"
#include "mvvm/JZNodeVariableBind.h"

void JZObjectConnect(JZNodeObject* sender, JZFunctionPointer signal, JZFunctionPointer slot)
{
    auto env = g_engine->environment();
    auto s = env->objectManager()->signal(signal.function);
    auto func = env->functionManager()->function(slot.function);
    Q_ASSERT(s && func && JZNodeType::sigSlotTypeMatch(s, func));
    if (s->csignal)
        s->csignal->connect(sender, func->name);
    else
    {
        sender->signalConnect(signal, slot);
    }
}

void JZObjectDisconnect(JZNodeObject* sender, JZFunctionPointer signal, JZFunctionPointer slot)
{
    auto env = g_engine->environment();
    auto s = env->objectManager()->signal(signal.function);
    auto func = env->functionManager()->function(slot.function);
    Q_ASSERT(s && func);
    if (s->csignal)
        s->csignal->disconnect(sender, func->name);
    else
    {        
        sender->signalDisconnect(signal, slot);
    }
}

void JZObjectConnect(JZNodeObject *sender, JZFunctionPointer signal, JZNodeObject *recv, JZFunctionPointer slot)
{
    auto env = g_engine->environment();
    auto s = env->objectManager()->signal(signal.function);
    auto func = env->functionManager()->function(slot.function);
    Q_ASSERT(s && func && JZNodeType::sigSlotTypeMatch(s,func));
    if (s->csignal)
        s->csignal->connect(sender,recv,func->name);
    else
        sender->signalConnect(signal,recv, slot);
}

void JZObjectDisconnect(JZNodeObject *sender, JZFunctionPointer signal, JZNodeObject *recv, JZFunctionPointer slot)
{
    auto env = g_engine->environment();
    auto s = env->objectManager()->signal(signal.function);
    auto func = env->functionManager()->function(slot.function);
    Q_ASSERT(s && func);
    if (s->csignal)
        s->csignal->disconnect(sender,recv,func->name);
    else
        sender->signalDisconnect(signal,recv, slot);
}

bool JZObjectIsList(JZNodeObject *obj)
{
    return obj->className().startsWith("QList<");
}

bool JZObjectIsMap(JZNodeObject *obj)
{
    return obj->className().startsWith("QMap<");
}

bool JZObjectIsSet(JZNodeObject *obj)
{
    return obj->className().startsWith("Set<");
}

//CMeta
CMeta::CMeta()
{
    isCopyable = false;
    isAbstract = false;
    isCompare = false;

    create = nullptr;
    copy = nullptr;
    destory = nullptr;
}

//JZNodeObjectWidgetDefine
JZNodeObjectWidgetDefine::JZNodeObjectWidgetDefine()
{
    type = Widget_None;
}

QDataStream &operator<<(QDataStream &s, const JZNodeObjectWidgetDefine &param)
{
    s << param.type << param.buffer;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeObjectWidgetDefine &param)
{
    s >> param.type >> param.buffer;
    return s;
}

//JZNodeObjectDefine
JZNodeObjectDefine::JZNodeObjectDefine()
{
    id = Type_none;
    isCObject = false;    
    valueType = false;
    manager = nullptr;
}

QString JZNodeObjectDefine::fullname() const
{
    QString name;
    if(!nameSpace.isEmpty())
        name = nameSpace + "::";
        
    name += className;
    return name;
}

int JZNodeObjectDefine::baseId() const
{
    auto def = this;
    while(def->super())
        def = def->super();
    return def->id;
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
    auto def = this;
    while (def)
    {
        auto it = def->params.find(name);
        if(it != def->params.end())
            return &it.value();

        for (int i = 0; i < widgetParams.size(); i++)
        {
            if (widgetParams[i].name == name)
                return &widgetParams[i];
        }

        def = def->super();
    }            
     
    return nullptr;
}

JZFunctionDefine JZNodeObjectDefine::initStaticFunction(QString function) const
{
    JZFunctionDefine def;
    def.name = function;
    def.className = className;
    return def;  
}

JZFunctionDefine JZNodeObjectDefine::initMemberFunction(QString function) const
{
    JZFunctionDefine def;
    def.name = function;
    def.className = className;
    def.paramIn.push_back(JZParamDefine("this", JZNodeType::pointerType(className)));
    return def;    
}

JZFunctionDefine JZNodeObjectDefine::initVirtualFunction(QString name) const
{
    auto func_def = function(name);
    Q_ASSERT(func_def);

    JZFunctionDefine new_def = *func_def;
    new_def.isCFunction = false;
    new_def.className = className;
    return new_def;
}

JZFunctionDefine JZNodeObjectDefine::initSlotFunction(QString name,QString signal) const
{
    const JZParamDefine *param_def = param(name);
    Q_ASSERT(param_def);

    auto param_meta = manager->meta(param_def->type);
    Q_ASSERT(param_meta && param_meta->signal(signal));
    
    auto s = param_meta->signal(signal);
    QString func_name = "on_" + name + "_" + signal;
    JZFunctionDefine func_def = initMemberFunction(func_name);
    for(int i = 0; i < s->paramOut.size(); i++)
    {
        func_def.paramIn.push_back(s->paramOut[i]);
    }
    return func_def;
}

void JZNodeObjectDefine::addFunction(const JZFunctionDefine &def)
{
    Q_ASSERT(!def.name.contains(".") && !def.name.contains("::") && def.className == className);
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

bool JZNodeObjectDefine::check(QString &error) const
{
    auto env = manager->env();
    int count = 0;
    if (!superName.isEmpty() && !super())
    {
        error = "super class " + superName + " not define";
        return false;
    }
   
    QStringList param_list = paramList(false);
    for (int i = 0; i < param_list.size(); i++)
    {
        auto param_def = param(param_list[0]);                
        if (!manager->env()->isVaildType(param_def->type))
        {
            error = "param " + param_def->type + " not define";
            return false;
        }
    }

    const JZFunctionDefine *func = nullptr;
    for (int i = 0; i < functions.size(); i++)
    {
        auto &func = functions[i];
        if (func.isMemberFunction() && func.paramIn[0].type != JZNodeType::pointerType(className))
        {
            error = "函数" + func.name + " this 类型不正确";
            return false;
        }

        for (int j = 0; j < functions.size(); j++)
        {
            if (i != j && functions[i].name == functions[j].name)
            {                
                error = "存在重名函数" + functions[i].name;
                return false;
            }
        }
    }
    
    auto def = this->super();
    while(def)
    {        
        QStringList super_functions = def->functionList();
        for (int funx_idx = 0; funx_idx < functions.size(); funx_idx++)
        {
            auto func = &functions[funx_idx];
            for (int i = 0; i < super_functions.size(); i++)
            {
                if (func->name == super_functions[i])
                {
                    const JZFunctionDefine *super_func = def->function(func->name);
                    if (!func->isVirtualFunction)
                    {
                        error = "父类存在重名函数" + func->delcare();
                        return false;
                    }

                    if (!env->isFunctionTypeMatch(func, super_func))
                    {
                        error = "函数签名不匹配" + func->delcare() + "," + super_func->delcare();
                        return false;
                    }
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

QStringList JZNodeObjectDefine::virtualFunctionList() const
{
    QSet<QString> set;
    auto def = this;
    while (def)
    {
        for (int i = 0; i < def->functions.size(); i++)
        {
            if(def->functions[i].isVirtualFunction)
                set << def->functions[i].name;
        }

        def = def->super();        
    }    
    return set.toList();
}

QStringList JZNodeObjectDefine::JZNodeObjectDefine::signalList() const
{
    QSet<QString> set;
    auto def = this;
    while(def)
    {
        for (int i = 0; i < def->signalDefines.size(); i++)
            set << def->signalDefines[i].name;

        def = def->super();
    }
    return set.toList();
}

const JZSignalDefine *JZNodeObjectDefine::signal(const QString &function) const
{
    for(int i = 0; i < signalDefines.size(); i++)
    {
        if(signalDefines[i].name == function)
            return &signalDefines[i];
    }

    auto def = super();
    if(def)
        return def->signal(function);
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

    return manager->meta(superName);
}

const JZNodeObjectDefine *JZNodeObjectDefine::cSuper() const
{
    auto def = this;
    while (def)
    {
        if (def->isCObject)
            return def;

        def = def->super();
    }
    return nullptr;
}

bool JZNodeObjectDefine::isInherits(int type) const
{
    return manager->isInherits(id,type);
}

bool JZNodeObjectDefine::isInherits(const QString &name) const
{
    return isInherits(manager->getClassId(name));
}

bool JZNodeObjectDefine::isAbstract() const
{
    if (isCObject)
        return cMeta.isAbstract;
    else
        return false;
}

bool JZNodeObjectDefine::isCopyable() const
{
    if(isCObject)
        return cMeta.isCopyable;
    else
    {
        for(auto &v: params)
        {
            int data_type = manager->env()->nameToType(v.type);
            if(JZNodeType::isObject(data_type))
            {                
                bool ret = manager->meta(data_type)->isCopyable();
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
    s << param.signalDefines;
    s << param.enums;

    s << param.isCObject;
    s << param.widgetDefine;
    s << param.widgetParams;
    s << param.widgetBind;
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
    s >> param.signalDefines;
    s >> param.enums;
    
    s >> param.isCObject;
    s >> param.widgetDefine;    
    s >> param.widgetParams;
    s >> param.widgetBind;
    return s;
}

//JZNodeCObjectDelcare
JZNodeCObjectDelcare::JZNodeCObjectDelcare()
{
    id = Type_none;
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

//JZNodeObject
JZNodeObject::JZNodeObject(const JZNodeObjectDefine *def)
{    
    m_define = def;
    m_cobj = nullptr;
    m_cobjOwner = false;
}

JZNodeObject::~JZNodeObject()
{        
    clearCObj();
}

const JZNodeObjectManager *JZNodeObject::manager() const
{
    return m_define->manager;
}

const JZCParamDefine *JZNodeObject::cparam(const QString &name) const
{
    const JZNodeObjectDefine *def = m_define;
    while (def)
    {
        auto it = def->cparams.find(name);
        if (it != def->cparams.end())
            return &it.value();

        def = def->super();
    }

    return nullptr;
}

void JZNodeObject::clearCObj()
{
    m_params.clear();
    if(m_cobjOwner)
    {        
        QPointer<QObject> ptr;
        if (isInherits(Type_object))
        {
            auto qobj = (QObject*)m_cobj;
            if(qobj->parent())
                return;

            ptr = QPointer<QObject>(qobj);
            qobj->disconnect(qobj,&QObject::destroyed,this,&JZNodeObject::onDestory);
        }
        else if(isInherits(Type_tableWidgetItem))
        {
            auto item = (QTableWidgetItem*)m_cobj;
            if (item->tableWidget())
                return;
        }
        else if (isInherits(Type_treeWidgetItem))
        {
            auto item = (QTreeWidgetItem*)m_cobj;
            if (item->treeWidget())
                return;
        }
        else if (isInherits(Type_listWidgetItem))
        {
            auto item = (QListWidgetItem*)m_cobj;
            if (item->listWidget())
                return;
        }

        auto def = m_define;
        while (!def->isCObject)
            def = def->super();

        def->cMeta.destory(m_cobj);
    }
    m_cobj = nullptr;
}

bool JZNodeObject::isInherits(int type) const
{
    return m_define->isInherits(type);
}

bool JZNodeObject::isInherits(const QString &name) const
{
    return m_define->isInherits(name);
}

bool JZNodeObject::isCopyable() const
{
    return m_define->isCopyable();
}

bool JZNodeObject::isCObject() const
{
    return m_define->isCObject;
}

bool JZNodeObject::isNull() const
{
    return m_define->isCObject && !m_cobj;
}

bool JZNodeObject::isValueType() const
{
    return m_define->isValueType();
}

const QString &JZNodeObject::className() const
{
    return m_define->className;
}

int JZNodeObject::type() const
{
    return m_define->id;
}

int JZNodeObject::baseType() const
{
    return m_define->baseId();
}

const JZNodeObjectDefine *JZNodeObject::meta() const
{
    return m_define;
}

void JZNodeObject::initParam(const QString &name, const QVariantPtr &ptr)
{
    m_params[name] = ptr;
}

bool JZNodeObject::hasParam(const QString &name) const
{
    return m_params.contains(name);
}

QStringList JZNodeObject::paramList() const
{
    return m_params.keys();
}

QVariant JZNodeObject::param(const QString &name) const
{    
    auto it = m_params.find(name);
    if (it != m_params.end())
    {
        if (!it->cparam)
            return *it->ptr.data();
        else
            return it->value();
    }
    Q_ASSERT(0);
    return QVariant();
}

void JZNodeObject::setParam(const QString &name, const QVariant &value)
{
    auto env = manager()->env();
    auto it = m_params.find(name);
    if (it != m_params.end())
    {
        Q_ASSERT(env->isSameType(JZNodeType::variantType(value), it->type));
        if (value == *it->ptr)
            return;
        
        if(!it->cparam)
        {
            *it->ptr = value;
        }
        else
        {
            it->setValue(value);
        }

        if (m_paramBind.contains(name))
        {
            auto& bind_list = m_paramBind[name];
            for (int i = 0; i < bind_list.size(); i++)
                bind_list[i]->dataToUi();
        }

        emit sigValueChanged(name);
    }
    else
    {
        Q_ASSERT(0);
    }
}

QVariantPtr* JZNodeObject::paramRef(const QString& name)
{
    auto it = m_params.find(name);
    if (it == m_params.end())
        return nullptr;

    return &it.value();
}

const JZFunctionDefine *JZNodeObject::function(const QString &name) const
{
    return m_define->function(name);
}

QStringList JZNodeObject::functionList() const
{
    return m_define->functionList();
}

const JZSignalDefine *JZNodeObject::signal(QString function) const
{
    return m_define->signal(function);
}

QStringList JZNodeObject::signalList() const
{
    return m_define->signalList();
}

void JZNodeObject::onDestory(QObject *obj)
{
    m_cobjOwner = false;
    m_cobj = nullptr;
}

void JZNodeObject::onRecvDestory(QObject *obj)
{   
    for(int i = m_connectList.size() - 1; i >= 0; i--)
    {
        if(m_connectList[i].recv == obj)
            m_connectList.removeAt(i);
    }
}

int JZNodeObject::signalConnectCount(JZNodeObject *recv) const
{
    int count = 0;
    for(int i = 0; i < m_connectList.size(); i++)
    {
        if(m_connectList[i].recv == recv)
            count++;
    }
    return count;
}

void JZNodeObject::signalConnect(JZFunctionPointer sig, JZFunctionPointer slot)
{
}

void JZNodeObject::signalDisconnect(JZFunctionPointer sig, JZFunctionPointer slot)
{
}

void JZNodeObject::signalConnect(JZFunctionPointer sig,JZNodeObject *recv,JZFunctionPointer slot)
{
    if(signalConnectCount(recv) == 1)
    {
        connect(recv,&QObject::destroyed,this,&JZNodeObject::onRecvDestory);
        connect(this,&JZNodeObject::sigTrigger,recv,&JZNodeObject::onSigTrigger);
    }
}

void JZNodeObject::signalDisconnect(JZFunctionPointer sig,JZNodeObject *recv,JZFunctionPointer slot)
{
    if(signalConnectCount(recv) == 0)
    {
        disconnect(recv,&QObject::destroyed,this,&JZNodeObject::onRecvDestory);
        disconnect(this,&JZNodeObject::sigTrigger,recv,&JZNodeObject::onSigTrigger);
    }
}

void JZNodeObject::signalEmit(JZFunctionPointer sig_name,const QVariantList &params)
{
    for(int i = 0; i < m_connectList.size(); i++)
    {
        if(m_connectList[i].signal == sig_name.function)
            emit sigTrigger(m_connectList[i].slot,params);
    }
}

void JZNodeObject::autoConnect()
{    
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
        auto jz_obj = toJZObject(param(param_name));
        if(!jz_obj)
        {
            qDebug() << "connect slot by name object not init";
            continue;
        }

        auto sig_func = jz_obj->signal(sig);
        if(!sig_func)
        {
            qDebug() << "connect slot by name no signal: " + sig;
            continue;
        }

        auto slot_func = function(func);
        if(!JZNodeType::sigSlotTypeMatch(sig_func,slot_func))
        {
            qDebug() << "connect slot by name slot function no match." << sig_func->fullName() << slot_func->delcare();
            continue;
        }

        JZFunctionPointer sig_func_ptr;
        sig_func_ptr.function = sig_func->fullName();

        JZFunctionPointer slot_func_ptr;
        slot_func_ptr.function = slot_func->fullName();
        JZObjectConnect(jz_obj,sig_func_ptr,this,slot_func_ptr);
    }
}

void JZNodeObject::autoBind()
{
    auto it = m_define->widgetBind.begin();
    while (it != m_define->widgetBind.end())
    {
        auto def = m_params.find(it->widget);
        if (def != m_params.end())
        {
            QWidget *w = manager()->objectCast<QWidget>(*def->ptr.data());
            
            QStringList path_list = it->path.split(".");
            if (path_list.size() == 1)
            {
                JZBindManager::instance()->bind(w, this, it->path, it->dir);
            }
            else
            {
                QString variable = path_list.back();
                path_list.pop_back();
                QString context_path = path_list.join(".");
                JZNodeObject *context_obj = toJZObject(param(context_path));

                JZBindManager::instance()->bind(w, context_obj, variable, it->dir);
            }
        }

        it++;
    }               
}

void JZNodeObject::autoInit()
{
    QString init_func = className() + "::__init__";
    auto func = manager()->env()->functionManager()->function(init_func);
    if (func)
    {
        JZNodeObjectPointer self(this, false);
        QVariantList in, out;
        in << QVariant::fromValue(self);
        JZScriptInvoke(init_func, in, out);
    }
}

void JZNodeObject::onSigTrigger(QString name,const QVariantList &params)
{
    auto func = function(name);
    QString full_name = func->fullName();

    QVariantList in,out;
    JZNodeObjectPointer ptr(this,false);
    in << QVariant::fromValue(ptr);
    for(int i = 0; i < func->paramIn.size() - 1; i++)
        in << params[i];
    JZScriptInvoke(full_name,in,out);
}

void JZNodeObject::setCObject(void *obj,bool owner)
{
    Q_ASSERT(obj);
    clearCObj();

    m_cobj = obj;
    if(m_define->isInherits(Type_object))
    {
        QObject *qobj = (QObject*)m_cobj;
        qobj->setProperty("JZObject",QVariant::fromValue<void*>(this));
        qobj->connect(qobj, &QObject::destroyed, this, &JZNodeObject::onDestory);
    }
    setCOwner(owner);
}

void *JZNodeObject::cobj() const
{
    return m_cobj;
}

void JZNodeObject::setCOwner(bool owner)
{
    if(m_cobjOwner == owner)
        return;

    m_cobjOwner = owner;
}

void JZNodeObject::addBind(QString param, JZBindObject* object)
{
    m_paramBind[param].push_back(object);
}

//JZNodeObjectData
JZNodeObjectData::JZNodeObjectData()
{
    isOwner = false;
    object = nullptr;
}

JZNodeObjectData::~JZNodeObjectData()
{
    if (isOwner && object)
    {
        auto inst = object->manager();
        inst->destory(object);
    }
}

//JZNodeObjectPointer
JZNodeObjectPointer::JZNodeObjectPointer()
{
    m_dataType = Type_none;
}

JZNodeObjectPointer::JZNodeObjectPointer(int data_type)
{
    m_data = QSharedPointer<JZNodeObjectData>(new JZNodeObjectData());
    m_dataType = data_type;
}

JZNodeObjectPointer::JZNodeObjectPointer(JZNodeObject *obj,bool isOwner)
{
    m_data = QSharedPointer<JZNodeObjectData>(new JZNodeObjectData());
    m_data->isOwner = isOwner;
    m_data->object = obj;
    if(obj)
        m_dataType = obj->type();
}

JZNodeObjectPointer::~JZNodeObjectPointer()
{
}

int JZNodeObjectPointer::type() const
{
    return m_dataType;
}

void JZNodeObjectPointer::setType(int type)
{
    m_dataType = type;
}

bool JZNodeObjectPointer::isNull() const
{
    if (!m_data || !m_data->object || m_data->object->isNull())
        return true;

    return false;
}

JZNodeObject *JZNodeObjectPointer::object() const
{
    return m_data->object;
}

void JZNodeObjectPointer::relaseObject()
{
    m_data.reset();
}

void JZNodeObjectPointer::releaseOwner()
{
    m_data->isOwner = false;
}

JZNodeObjectPointer JZNodeObjectPointer::toWeakPointer()
{
    JZNodeObjectPointer p = *this;
    p.m_dataType = JZNodeType::pointerType(this->m_dataType);
    return p;
}

bool JZNodeObjectPointer::operator==(const JZNodeObjectPointer &other) const
{
    JZNodeObject *o1 = object();
    JZNodeObject *o2 = other.object();    
    return o1->meta()->manager->equal(o1,o2);
}

bool JZNodeObjectPointer::operator!=(const JZNodeObjectPointer &other) const
{
    return !(this->operator==(other));
}

bool isJZObject(const QVariant &v)
{
    return (v.userType() == qMetaTypeId<JZNodeObjectPointer>());
}

JZNodeObject* toJZObject(const QVariant &v)
{
    if (v.userType() == qMetaTypeId<JZNodeObjectPointer>())
    {
        auto ptr = (JZNodeObjectPointer*)v.data();
        return ptr->object();
    }
    else
    {      
        Q_ASSERT(0);
        return nullptr;
    }
}

JZNodeObjectPointer toJZObjectHolder(const QVariant &v)
{
    if (v.userType() == qMetaTypeId<JZNodeObjectPointer>())
    {
        return v.value<JZNodeObjectPointer>();
    }
    else
    {      
        Q_ASSERT(0);
        return JZNodeObjectPointer();
    }
}

JZNodeObject* qobjectToJZObject(QObject *obj)
{
    auto ptr = obj->property("JZObject").value<void*>();
    if(!ptr)
        return nullptr;
    else
        return (JZNodeObject*)ptr;
}

//JZNodeObjectManager
JZNodeObjectManager::JZNodeObjectManager(JZScriptEnvironment *env)
{        
    m_objectId = Type_internalObject;
    m_enumId = Type_internalEnum;    
    m_userRegist = false;
    m_env = env;
}

JZNodeObjectManager::~JZNodeObjectManager()
{    
    m_metas.clear();    
}

JZScriptEnvironment *JZNodeObjectManager::env()
{
    return m_env;
}

const JZScriptEnvironment *JZNodeObjectManager::env() const
{
    return m_env;
}

void JZNodeObjectManager::init()
{          
    m_objectId = Type_internalObject;
    m_enumId = Type_internalEnum;
    m_userRegist = false;

    m_ctypeidMap.clear();
    m_enums.clear();
    m_metas.clear();
    m_qobjectId.clear();
    m_widgetFactory.clear();

    //init
    jzbind::ClassBind<JZVariantAny> cls_any(Type_any, "any");
    cls_any.def("type", false, &JZVariantAny::type);
    cls_any.regist();

    jzbind::ClassBind<JZFunctionPointer> cls_function(Type_function,"function");
    cls_function.regist();

    registQtClass(m_env);    
    initFunctions();           
}

void JZNodeObjectManager::setUserRegist(bool flag)
{
    m_userRegist = flag;
    if(m_userRegist)
        m_objectId = Type_userObject;
}

void JZNodeObjectManager::initFunctions()
{        
}

int JZNodeObjectManager::getId(const QString &type_name) const
{
    int type = getClassId(type_name);
    if (type != Type_none)
        return type;

    type = getEnumId(type_name);
    if (type != Type_none)
        return type;

    return Type_none;
}

int JZNodeObjectManager::getIdByCTypeid(const QString &type_name) const
{
    return m_ctypeidMap.value(type_name, Type_none);
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

int JZNodeObjectManager::nextObjectId()
{
    int id = m_objectId;
    while (meta(id))
    {
        id++;
    }

    m_objectId = id + 1;
    return id;
}

int JZNodeObjectManager::regist(const JZNodeObjectDefine &info)
{
    //可以先声明在注册
    Q_ASSERT(!info.className.isEmpty());
    //没有定义或者声明的id和之前一直
    Q_ASSERT(!meta(info.className));

    JZNodeObjectDefine *def = new JZNodeObjectDefine();
    *def = info;
    def->manager = this;
    if(info.id != Type_none)
    {
        def->id = info.id;        
    }
    else
    {        
        def->id = nextObjectId();
    }
    Q_ASSERT((!m_userRegist && def->id < Type_userObject) || (m_userRegist && def->id >= Type_userObject));

    m_metas.insert(def->id ,QSharedPointer<JZNodeObjectDefine>(def));
    return def->id;
}

void JZNodeObjectManager::replace(const JZNodeObjectDefine &define)
{
    Q_ASSERT(m_metas.contains(define.id));
        
    JZNodeObjectDefine *ptr = m_metas[define.id].data();
    *ptr = define;
    ptr->manager = this;
}

int JZNodeObjectManager::registCClass(const JZNodeObjectDefine &define,const QString &ctype_id)
{
    Q_ASSERT(!m_ctypeidMap.contains(ctype_id) || m_ctypeidMap[ctype_id] == define.id);

    int id = regist(define);
    m_ctypeidMap[ctype_id] = id;
    return id;
}

int JZNodeObjectManager::registEnum(const JZNodeEnumDefine &define)
{
    int id = Type_none;
    if (define.id() == Type_none)
        id = m_enumId++;
    else
        id = define.id();

    m_enums[id] = define;    
    m_enums[id].setId(id);
    return id;
}

int JZNodeObjectManager::registCEnum(const JZNodeEnumDefine &define, const QString &ctype_id)
{
    Q_ASSERT(!m_ctypeidMap.contains(ctype_id) || m_ctypeidMap[ctype_id] == define.id());

    int id = registEnum(define);
    m_ctypeidMap[ctype_id] = id;    
    return id;
}

void JZNodeObjectManager::setFlag(int flag, int flag_enum)
{
    m_enums[flag].setFlag(true, flag_enum);
}

void JZNodeObjectManager::unregist(int id)
{   
    if (!m_metas.contains(id))
        return;

    m_metas.remove(id);
    QString ctype_id = m_ctypeidMap.key(id);
    if(!ctype_id.isEmpty())
        m_ctypeidMap.remove(ctype_id);
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

JZEnum JZNodeObjectManager::createEnum(int enumType) const
{
    JZEnum e;
    e.type = enumType;
    e.value = enumMeta(enumType)->defaultValue();
    return e;
}

void JZNodeObjectManager::registWidgetFactory(int type, JZNodeObjectWidgetFactory define)
{
    m_widgetFactory[type] = define;
}

const JZNodeObjectWidgetFactory *JZNodeObjectManager::widgetFactory(int type) const
{
    auto it = m_widgetFactory.find(type);
    if (it == m_widgetFactory.end())
        return nullptr;

    return &it.value();
}

bool JZNodeObjectManager::hasType(int type_id) const
{
    return meta(type_id) || enumMeta(type_id);
}

const JZNodeObjectDefine *JZNodeObjectManager::meta(int id) const
{
    Q_ASSERT(id != Type_none && !JZNodeType::isPointer(id));
    return m_metas.value(id,nullptr).data();
}

const JZNodeObjectDefine *JZNodeObjectManager::meta(const QString &className) const
{
    if(className.isEmpty())
        return nullptr;

    int type = getClassId(className);
    if (type == Type_none)
        return nullptr;

    return meta(type);
}

const JZSignalDefine *JZNodeObjectManager::signal(const QString &name) const
{
    QStringList list = name.split("::");
    if (list.size() != 2)
        return nullptr;

    auto cls = meta(list[0]);
    if (!cls)
        return nullptr;

    return cls->signal(list[1]);
}

void JZNodeObjectManager::setQObjectType(int id, const QString & type_name)
{
    m_qobjectId[id] = type_name;
}

QString JZNodeObjectManager::getQObjectType(int id) const
{
    return m_qobjectId.value(id, QString());
}

int JZNodeObjectManager::getTypeByQObject(QString qmeta) const
{
    return m_qobjectId.key(qmeta, Type_none);
}

int JZNodeObjectManager::getClassId(const QString &class_name) const
{
    Q_ASSERT(!JZNodeType::isPointer(class_name));

    auto it = m_metas.begin();
    while(it != m_metas.end())
    {
        if(it.value()->className == class_name)
            return it.key();

        it++;
    }
    return Type_none;
}

bool JZNodeObjectManager::isInherits(const QString &class_name, const QString &super_name) const
{
    int class_type = getClassId(class_name);
    int super_type = getClassId(super_name);    
    return isInherits(class_type, super_type);
}

bool JZNodeObjectManager::isInherits(int class_name,int super_name) const
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

QStringList JZNodeObjectManager::getClassList() const
{
    QStringList list;

    auto it = m_metas.begin();
    while (it != m_metas.end())
    {
        if(it.key() >= Type_enum)
            list << (it.value()->className);
        it++;
    }
    return list;
}

QString JZNodeObjectManager::getClassName(int type_id) const
{
    auto it = m_metas.find(type_id);
    if(it == m_metas.end())
        return QString();

    return it.value()->className;
}

const JZNodeEnumDefine *JZNodeObjectManager::enumMeta(int type_id) const
{
    auto it = m_enums.find(type_id);
    if (it == m_enums.end())
        return nullptr;

    return &it.value();
}

const JZNodeEnumDefine *JZNodeObjectManager::enumMeta(const QString &enumName) const
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

QString JZNodeObjectManager::getEnumName(int type_id) const
{
    auto it = m_enums.find(type_id);
    if (it == m_enums.end())
        return QString();

    return it->name();
}

int JZNodeObjectManager::getEnumId(const QString &enumName) const
{
    auto it = m_enums.begin();
    while (it != m_enums.end())
    {
        if (it->name() == enumName)
            return it.key();
        it++;
    }
    return Type_none;
}

QStringList JZNodeObjectManager::getEnumList() const
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

void JZNodeObjectManager::copy(JZNodeObject *src,JZNodeObject *dst) const
{
    Q_ASSERT(dst->isCopyable());
    
    auto cBase = src->meta()->cSuper();
    if (cBase)
        cBase->cMeta.copy(src->cobj(), dst->cobj());

    auto it = dst->m_params.begin();
    while(it != dst->m_params.end())
    {       
        if (!it->isCParam())
        {
            QVariant* v1 = src->m_params[it.key()].ptr.data();
            QVariant* v2 = it->ptr.data();
            if (isJZObject(*v1))
            {
                JZNodeObject* src_ptr = toJZObject(*v1);
                JZNodeObject* dst_ptr = toJZObject(*v2);
                copy(src_ptr, dst_ptr);
            }
            else
                *v2 = *v1;
        }
        
        it++;
    }
}

void JZNodeObjectManager::create(const JZNodeObjectDefine *in_def,JZNodeObject *obj, CObjectInfo *cobj_info) const
{    
    Q_ASSERT(in_def);

    QList<const JZNodeObjectDefine*> def_list;
    const JZNodeObjectDefine* obj_def = in_def;
    
    while (obj_def)
    {
        def_list << obj_def;
        if (obj_def->isCObject)
        {
            if (cobj_info)
            {
                obj->m_cobj = cobj_info->cobj;
                obj->m_cobjOwner = cobj_info->isOwner;
            }
            else
            {
                if (!obj_def->cMeta.isAbstract)
                {
                    auto cobj = obj_def->cMeta.create();
                    obj->setCObject(cobj, true);
                }
                else
                {
                    obj->m_cobj = nullptr;
                    obj->m_cobjOwner = false;
                }
            }            
            break;
        }

        obj_def = obj_def->super();
    }


    QObject* qobj = nullptr;
    if (in_def->isInherits(Type_object))
        qobj = JZObjectCast<QObject>(obj);

    for (int i = def_list.size() - 1; i >= 0; i--)
    {
        obj_def = def_list[i];
        if (obj_def->widgetDefine.type != Widget_None)
        {            
            auto widget_factory = widgetFactory(obj_def->widgetDefine.type);
            widget_factory->creator(obj);
        }

        auto it = obj_def->params.begin();
        while(it != obj_def->params.end())
        {
            auto *param = &it.value();
            if (obj->m_params.contains(param->name)) //ui widget
            {
                it++;
                continue;
            }

            QVariantPtr ptr;
            ptr.type = m_env->nameToType(param->type);
            if (!obj->cparam(param->name))
            {
                *ptr.ptr = g_engine->createVariable(m_env->nameToType(param->type), param->value);        
                
                if (qobj && isInherits(ptr.type, Type_object))
                {
                    QObject* child = JZObjectCast<QObject>(toJZObject(*ptr.ptr));
                    child->setObjectName(param->name);
                    child->setParent(qobj);
                }
            }
            else
            {
                ptr.cobj = obj->m_cobj;
                ptr.cparam = obj->cparam(param->name);
            }
            obj->m_params[param->name] = ptr;

            it++;
        }
    }
}

JZNodeObject* JZNodeObjectManager::create(int type) const
{
    const JZNodeObjectDefine *def = meta(type);
    Q_ASSERT(def);

    JZNodeObject *obj = new JZNodeObject(def);    
    create(def,obj,nullptr);    
    return obj;
}

JZNodeObject* JZNodeObjectManager::create(const QString &name) const
{
    int id = getClassId(name);
    return create(id);
}

JZNodeObject* JZNodeObjectManager::createByCTypeid(const QString &type_id) const
{
    Q_ASSERT(m_ctypeidMap.contains(type_id));

    int className = m_ctypeidMap[type_id];
    return create(className);
}

JZNodeObject* JZNodeObjectManager::createReference(int type_id, void *ptr, bool owner) const
{
    CObjectInfo cobj_info;
    cobj_info.cobj = ptr;
    cobj_info.isOwner = owner;

    auto def = meta(type_id);
    JZNodeObject *obj = new JZNodeObject(def);
    create(def, obj, &cobj_info);
    return obj;
}

JZNodeObject* JZNodeObjectManager::createReference(const QString &type_name,void *ptr, bool owner) const
{
    int type_id = getId(type_name);
    return createReference(type_id,ptr,owner);
}

JZNodeObject* JZNodeObjectManager::createReferenceByCTypeid(const QString &ctype_id,void *ptr, bool owner) const
{
    if(!m_ctypeidMap.contains(ctype_id))
        return nullptr;
    
    return createReference(m_ctypeidMap[ctype_id],ptr,owner);
}

void JZNodeObjectManager::destory(JZNodeObject *object) const
{
    delete object;
}

JZNodeObjectPointer JZNodeObjectManager::createHolder(int type_id) const
{
    return JZNodeObjectPointer(create(type_id), true);
}

JZNodeObjectPointer JZNodeObjectManager::createHolder(const QString& type_name) const
{
    return JZNodeObjectPointer(create(type_name), true);
}

JZNodeObject* JZNodeObjectManager::clone(JZNodeObject *src) const
{
    Q_ASSERT(src->isCopyable());

    JZNodeObject* dst = create(src->type());
    copy(src,dst);
    return dst;
}

bool JZNodeObjectManager::equal(JZNodeObject* o1,JZNodeObject *o2) const
{
    if (o1 == o2)
        return true;
    if (!o1 || !o2)
        return false;

    if(o1->isValueType())
    {
        if(o1->isCObject())
            return o1->meta()->cMeta.equal(o1->cobj(),o2->cobj());
        else
        {
            auto cBase = o1->meta()->cSuper();
            if (cBase)
            {
                if (!cBase->cMeta.equal(o1->cobj(), o2->cobj()))
                    return false;
            }

            //c 的部分已经在上面比较过，不需要再做比较
            auto it = o1->m_params.begin();
            while(it != o1->m_params.end())
            {
                if (it->ptr.data() && (*it->ptr.data() != *o2->m_params[it.key()].ptr.data()))
                    return false;
                
                it++;
            }
            return true;
        }
    }
    else
    {
        return o1 == o2;
    }
}