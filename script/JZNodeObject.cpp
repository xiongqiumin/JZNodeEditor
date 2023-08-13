#include "JZNodeObject.h"
#include <QMetaObject>
#include "JZNodeBind.h"
#include <QApplication>
#include <QDebug>
#include "JZNodeUiLoader.h"
#include "JZNodeQtWrapper.h"

QString JObjectTypename(const QVariant &v)
{
    if(v.type() < QVariant::UserType)
        return v.typeName();
    else if(isJZObject(v))
    {
        JZNodeObject *ptr = toJZObject(v);
        return ptr->define->className;
    }
    else
        return "unknown";
}

int JZClassId(const QString &name)
{
    return JZNodeObjectManager::instance()->getClassId(name);
}

QString JZClassName(int id)
{
    return JZNodeObjectManager::instance()->getClassName(id);
}

void JZNodeDisp(const QVariant &v)
{
    QString str = JZNodeType::toString(v);
    JZScriptLog(str);    
}

QVariant CreateJZObject(QString name)
{
    int id = JZNodeObjectManager::instance()->getClassId(name);
    JZNodeObjectPtr ptr = JZNodeObjectManager::instance()->create(id);
    return QVariant::fromValue(ptr);
}

QString toString(const QVariant &v)
{
    return v.toString();
}

bool toBool(const QVariant &v)
{
    return v.toBool();
}

int toInt(const QVariant &v)
{
    return v.toInt();
}

double toDouble(const QVariant &v)
{
    return v.toDouble();
}

//CMeta
CMeta::CMeta()
{
    isCopyable = false;
    isAbstract = false;

    create = nullptr;
    copy = nullptr;
    destory = nullptr;
    addEventFilter = nullptr;
}


//CSingle
CSingle::CSingle()
{    
}

CSingle::~CSingle()
{

}

//SingleDefine
SingleDefine::SingleDefine()
{
    isCSingle = false;
    csingle = nullptr;
}

QDataStream &operator<<(QDataStream &s, const SingleDefine &param)
{
    Q_ASSERT(!param.isCSingle);
    s << param.name;
    s << param.eventType;
    s << param.paramOut;
    return s;
}

QDataStream &operator>>(QDataStream &s, SingleDefine &param)
{
    s >> param.name;
    s >> param.eventType;
    s >> param.paramOut;
    return s;
}

//JZNodeObjectDefine
JZNodeObjectDefine::JZNodeObjectDefine()
{
    id = -1;
    isCObject = false;
    isUiWidget = false;    
}

QString JZNodeObjectDefine::fullname() const
{
    QString name;
    if(!nameSpace.isEmpty())
        name = nameSpace + "::";
    name += className;
    return name;
}

void JZNodeObjectDefine::addParam(JZParamDefine def)
{
    params[def.name] = def;
}

void JZNodeObjectDefine::removeParam(QString name)
{
    params.remove(name);
}

QStringList JZNodeObjectDefine::paramList() const
{
    QStringList list;
    auto def = this;
    while(def)
    {
        list << def->params.keys();
        def = def->super();
    }
    return list;
}

JZParamDefine *JZNodeObjectDefine::param(QString name)
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
                    def = JZNodeObjectManager::instance()->meta(it->dataType);
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

void JZNodeObjectDefine::addFunction(FunctionDefine def)
{
    Q_ASSERT(!def.name.contains(".") && !def.name.contains("::"));
    functions.push_back(def);    
}

void JZNodeObjectDefine::removeFunction(QString function)
{
    int index = indexOfFunction(function);
    if(index != -1)
        functions.removeAt(index);
}

int JZNodeObjectDefine::indexOfFunction(QString function) const
{
    for (int i = 0; i < functions.size(); i++)
    {
        if (functions[i].name == function)
            return i;
    }
    return -1;
}

void JZNodeObjectDefine::addSingle(FunctionDefine def)
{

}

void JZNodeObjectDefine::removeSingle(QString function)
{

}

const FunctionDefine *JZNodeObjectDefine::function(QString name) const
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

const SingleDefine *JZNodeObjectDefine::single(QString function) const
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

QStringList JZNodeObjectDefine::eventList() const
{
    QSet<QString> set;
    auto def = this;
    while (def)
    {
        for (int i = 0; i < def->events.size(); i++)
            set << def->events[i].name;

        def = def->super();
    }
    return set.toList();
}

const EventDefine *JZNodeObjectDefine::event(QString function) const
{
    for (int i = 0; i < events.size(); i++)
    {
        if (events[i].name == function)
            return &events[i];
    }

    auto def = super();
    if (def)
        return def->event(function);
    else
        return nullptr;
}

JZNodeObjectDefine *JZNodeObjectDefine::super() const
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
            if(JZNodeType::isObject(v.dataType))
            {                
                bool ret = JZNodeObjectManager::instance()->meta(v.dataType)->isCopyable();
                if(!ret)
                    return false;
            }
        }
    }    
    return true;
}


QDataStream &operator<<(QDataStream &s, const JZNodeObjectDefine &param)
{
    Q_ASSERT(!param.isCObject);

    s << param.id;
    s << param.nameSpace;
    s << param.className;
    s << param.superName;

    s << param.params;
    s << param.functions;
    s << param.singles;
    s << param.events;

    s << param.isCObject;
    s << param.isUiWidget;
    s << param.widgteXml;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeObjectDefine &param)
{
    s >> param.id;
    s >> param.nameSpace;
    s >> param.className;
    s >> param.superName;

    s >> param.params;
    s >> param.functions;
    s >> param.singles;
    s >> param.events;
    
    s >> param.isCObject;
    s >> param.isUiWidget;
    s >> param.widgteXml;
    return s;
}

//JZObjectNull
QDataStream &operator<<(QDataStream &s, const JZObjectNull &param)
{
    return s;
}
QDataStream &operator >> (QDataStream &s, JZObjectNull &param)
{
    return s;
}

//JZNodeObject
JZNodeObject::JZNodeObject(JZNodeObjectDefine *def)
{    
    define = def;        
    cobj = nullptr;
    cowner = false;
}

JZNodeObject::~JZNodeObject()
{
    if(cowner && define->isCObject)
        define->cMeta.destory(cobj);
}

bool JZNodeObject::isInherits(int type) const
{
    return JZNodeObjectManager::instance()->isInherits(define->id,type);
}

bool JZNodeObject::isInherits(const QString &name) const
{
    return isInherits(JZNodeObjectManager::instance()->getClassId(name));
}

bool JZNodeObject::isCopyable() const
{
    return define->isCopyable();
}

bool JZNodeObject::isCObject() const
{
    return define->isCObject;
}

const QString &JZNodeObject::className() const
{
    return define->className;
}

int JZNodeObject::type() const
{
    return define->id;
}

const JZNodeObjectDefine *JZNodeObject::meta() const
{
    return define;
}

bool JZNodeObject::getParamRef(const QString &name,QVariant *&ref,QString &error)
{
    QStringList list = name.split(".");
    JZNodeObject *obj = this;
    for(int i = 0; i < list.size() - 1; i++)
    {
        auto it = obj->params.find(list[i]);
        if(it == obj->params.end())
        {
            error = "no such element";
            return false;
        }
        if(!isJZObject(it.value()))
        {
            error = "not a object";
            return false;
        }
        obj = toJZObject(it.value());
    }
    if(!obj->params.contains(list.back()))
    {
        error = "no such element";
        return false;
    }
    ref = &obj->params[list.back()];
    return true;
}

bool JZNodeObject::hasParam(QString name) const
{
    JZNodeObject *obj = const_cast<JZNodeObject *>(this);
    QVariant *ptr = nullptr;
    QString error;
    return obj->getParamRef(name,ptr,error);
}

QVariant JZNodeObject::param(QString name) const
{
    JZNodeObject *obj = const_cast<JZNodeObject *>(this);
    QVariant *ptr = nullptr;
    QString error;
    if(!obj->getParamRef(name,ptr,error))
        throw std::runtime_error(qPrintable(error));
    return *ptr;
}

void JZNodeObject::setParam(QString name,QVariant value)
{
    QVariant *ptr = nullptr;
    QString error;
    if(!getParamRef(name,ptr,error))
        throw std::runtime_error(qPrintable(error));
    *ptr = value;
}

QVariant *JZNodeObject::paramRef(QString name)
{
    QVariant *ptr = nullptr;
    QString error;
    getParamRef(name,ptr,error);
    return ptr;
}

const FunctionDefine *JZNodeObject::function(QString name)
{
    return define->function(name);
}

void JZNodeObject::updateWidgetParam()
{
    Q_ASSERT(isInherits("Widget"));

    QObject *obj = (QObject*)cobj;
    auto inst = JZNodeObjectManager::instance();
    auto def = define;
    while(def)
    {
        auto it = def->params.begin();
        while(it != def->params.end())
        {
            if(it.value().cref)
            {
                QWidget *w = obj->findChild<QWidget*>(it.key());
                JZNodeObject *jzobj = new JZNodeObject(inst->meta(it.value().dataType));
                jzobj->setCObject(w,false);
                params[it.key()] = QVariant::fromValue(JZNodeObjectPtr(jzobj));
            }
            it++;
        }
        def = JZNodeObjectManager::instance()->meta(def->superName);
    }
}

void JZNodeObject::connectSingles(int type)
{
    Q_ASSERT(isInherits("Object"));

    auto def = define;
    while(def)
    {
        for(int i = 0; i < def->singles.size(); i++)
        {
            auto &s = def->singles[i];
            if (s.isCSingle && s.eventType == type)
            {
                s.csingle->connect(this, s.eventType);
                return;
            }
        }
        def = JZNodeObjectManager::instance()->meta(def->superName);
    }
}

void JZNodeObject::setCObject(void *obj,bool owner)
{
    if(cobj && cowner)
        define->cMeta.destory(obj);

    cobj = obj;
    cowner = owner;    
}

QDebug operator<<(QDebug dbg, const JZNodeObjectPtr &obj)
{
    QString className = obj->className();    
    if(className == "list")
        dbg << *((QVariantList*)obj->cobj);
    else
        dbg << className;

    return dbg;
}

bool isJZObject(const QVariant &v)
{
    return (v.userType() == qMetaTypeId<JZNodeObjectPtr>()
            || v.userType() == qMetaTypeId<JZNodeObject*>()
            || v.userType() == qMetaTypeId<JZObjectNull>());
}

JZNodeObject* toJZObject(const QVariant &v)
{
    if(v.userType() == qMetaTypeId<JZNodeObjectPtr>())
    {
        JZNodeObjectPtr *ptr = (JZNodeObjectPtr*)v.data();
        return ptr->data();
    }
    else if(v.userType() == qMetaTypeId<JZNodeObject*>())
    {
        return v.value<JZNodeObject*>();
    }
    else if (v.userType() == qMetaTypeId<JZObjectNull>())
    {
        return nullptr;
    }
    else
    {        
        return nullptr;
    }    
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
    m_enumId = Type_enum;
}

JZNodeObjectManager::~JZNodeObjectManager()
{    
    m_metas.clear();    
}

void JZNodeObjectManager::init()
{  
    QMetaType::registerDebugStreamOperator<JZNodeObjectPtr>();           

    registQtClass();    
    initFunctions();    

    JZNodeFunctionManager::instance()->setUserRegist(true);
    m_objectId = Type_userObject;
}

void JZNodeObjectManager::initFunctions()
{
    JZNodeFunctionManager::instance()->registCFunction("createObject",true,jzbind::createFuncion(CreateJZObject));    
    JZNodeFunctionManager::instance()->registCFunction("typename",false,jzbind::createFuncion(JObjectTypename));
    JZNodeFunctionManager::instance()->registCFunction("print",true,jzbind::createFuncion(JZNodeDisp));

    JZNodeFunctionManager::instance()->registCFunction("toString", false,jzbind::createFuncion(toString));
    JZNodeFunctionManager::instance()->registCFunction("toBool", false,jzbind::createFuncion(toBool));
    JZNodeFunctionManager::instance()->registCFunction("toInt", false,jzbind::createFuncion(toInt));
    JZNodeFunctionManager::instance()->registCFunction("toDouble", false,jzbind::createFuncion(toDouble));
}

int JZNodeObjectManager::delcare(QString name, QString c_typeid, int id)
{    
    JZNodeObjectDefine def;
    def.className = name;
    def.id = id;
    return registCClass(def, c_typeid);    
}

int JZNodeObjectManager::regist(JZNodeObjectDefine info)
{
    Q_ASSERT(!info.className.isEmpty() && (info.superName.isEmpty() || getClassId(info.superName) != -1));

    JZNodeObjectDefine *def = new JZNodeObjectDefine();
    *def = info;
    if(info.id != -1)
    {
        Q_ASSERT(getClassName(info.id).isEmpty());
        def->id = info.id;
        m_objectId = qMax(m_objectId,def->id + 1);
    }
    else
        def->id = m_objectId++;

    m_metas.insert(def->id ,QSharedPointer<JZNodeObjectDefine>(def));
    return def->id;
}

void JZNodeObjectManager::replace(JZNodeObjectDefine define)
{
    if (m_metas.contains(define.id))
        unregist(define.id);
    
    regist(define);        
}

int JZNodeObjectManager::registCClass(JZNodeObjectDefine define,QString ctype_id)
{
    int id = regist(define);
    m_typeidMetas[ctype_id] = id;
    return id;
}

void JZNodeObjectManager::registEnum(QString enumName, QString ctype_id)
{
    int id = m_enumId++;
    m_typeidMetas[ctype_id] = id;
}

void JZNodeObjectManager::unregist(int name)
{
    m_metas.remove(name);
}

void JZNodeObjectManager::declareQObject(int id,QString name)
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

JZNodeObjectDefine *JZNodeObjectManager::meta(int id)
{
    return m_metas.value(id,nullptr).data();
}

JZNodeObjectDefine *JZNodeObjectManager::meta(QString className)
{
    if(className.isEmpty())
        return nullptr;

    return meta(getClassId(className));
}

int JZNodeObjectManager::getClassIdByTypeid(QString name)
{
    return m_typeidMetas.value(name,Type_none);
}

int JZNodeObjectManager::getClassIdByQObject(QString name)
{
    return m_qobjectId.value(name,Type_none);
}

int JZNodeObjectManager::getClassId(QString class_name)
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

bool JZNodeObjectManager::isInherits(QString class_name, QString super_name)
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

void JZNodeObjectManager::copy(JZNodeObject *dst,JZNodeObject *src)
{
    Q_ASSERT(dst->isCopyable());

    if(src->isCObject())    
    {
        src->define->cMeta.copy(dst->cobj,src->cobj);
        return;
    }

    auto it = dst->params.begin();
    while(it != dst->params.end() )
    {
        if(isJZObject(it.value()))
        {
            JZNodeObject *src_ptr = toJZObject(src->params[it.key()]);
            JZNodeObject *dst_ptr = toJZObject(it.value());
            copy(dst_ptr,src_ptr);
        }
        else
            dst->params[it.key()] = it.value();
        it++;
    }
}

void JZNodeObjectManager::create(JZNodeObjectDefine *def,JZNodeObject *obj)
{
    if (def->isCObject)
    {
        obj->cobj = def->cMeta.create();            
        obj->cowner = true;
        return;
    }        

    if (def->isUiWidget)
    {
        JZNodeUiLoader loader;
        QWidget *widget = loader.create(def->widgteXml);
        Q_ASSERT(widget);
        widget->setWindowTitle(def->className);
        obj->cobj = widget;
        if (!obj->cobj)
            throw std::runtime_error(qPrintable(loader.errorString()));
        obj->cowner = true;
        obj->updateWidgetParam();
    }
    else
    {
        if (!def->superName.isEmpty())
            create(def->super(), obj);
    }

    auto it = def->params.begin();
    while(it != def->params.end() )
    {
        if((int)it.value().dataType < Type_object)
        {
            obj->params[it.key()] = it.value().initialValue();
        }
        else
        {
            if(!it.value().cref)
                obj->params[it.key()] = QVariant::fromValue(JZObjectNull());
        }
        it++;
    }
}

JZNodeObjectPtr JZNodeObjectManager::create(int name)
{
    JZNodeObjectDefine *def = meta(name);
    Q_ASSERT(def);

    JZNodeObject *obj = new JZNodeObject(def);    
    create(def,obj);
    return JZNodeObjectPtr(obj);
}

JZNodeObjectPtr JZNodeObjectManager::createCClass(QString type_id)
{
    Q_ASSERT(m_typeidMetas.contains(type_id));

    int className = m_typeidMetas[type_id];
    return create(className);
}

JZNodeObjectPtr JZNodeObjectManager::createCClassRefrence(QString type_id,void *ptr)
{
    if(!m_typeidMetas.contains(type_id))
        return nullptr;

    auto def = meta(m_typeidMetas[type_id]);
    JZNodeObject *obj = new JZNodeObject(def);
    obj->setCObject(ptr,false);
    return JZNodeObjectPtr(obj);
}

JZNodeObjectPtr JZNodeObjectManager::clone(JZNodeObjectPtr other)
{
    Q_ASSERT(other->isCopyable());

    JZNodeObjectPtr ret = create(other->define->id);
    copy(ret.data(),other.data());    
    return ret;
}
