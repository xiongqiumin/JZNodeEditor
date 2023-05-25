#include "JZNodeObject.h"
#include <QMetaObject>
#include "JZNodeBind.h"

//JZNodeObjectDefine
JZNodeObjectDefine::JZNodeObjectDefine()
{
    super = nullptr;
    isCObject = false;
}

FunctionDefine *JZNodeObjectDefine::function(QString name)
{
    if(super)
        return super->function(name);
    else
        return nullptr;
}

//JZNodeObject
JZNodeObject::JZNodeObject(JZNodeObjectDefine *def)
{
    define = def;
    if(define->isCObject)
        cobj = define->cMeta.create();
}

JZNodeObject::~JZNodeObject()
{
    if(define->isCObject)
        define->cMeta.destory(cobj);
}

bool JZNodeObject::isCObject() const
{
    return define->isCObject;
}

const QString &JZNodeObject::className() const
{
    return define->className;
}

QVariant JZNodeObject::param(QString name) const
{
    return params[name];
}

void JZNodeObject::setParam(QString name,QVariant value)
{
    params[name] = value;
}

FunctionDefine *JZNodeObject::function(QString name)
{
    return nullptr;
}

QDebug operator<<(QDebug dbg, const JZNodeObjectPtr &obj)
{
    QString className = obj->className();
    if(className == "string")
        dbg << *((QString*)obj->cobj);
    else
        dbg << className;

    return dbg;
}

//JZNodeObjectManager
JZNodeObjectManager *JZNodeObjectManager::instance()
{
    static JZNodeObjectManager inst;
    return &inst;
}

JZNodeObjectManager::JZNodeObjectManager()
{        
}

JZNodeObjectManager::~JZNodeObjectManager()
{
    for(auto ptr:m_metas)
        delete ptr;
    m_metas.clear();    
}

void JZNodeObjectManager::init()
{    
    jzbind::ClassBind<QString> cls_str("string");
    cls_str.def("left",&QString::left);
    cls_str.def("right",&QString::right);
    cls_str.def("size",&QString::size);
    cls_str.def("replace",[](const QString &in,const QString &before,const QString &after)->QString{
        QString ret = in;
        return ret.replace(before,after);
    });
    cls_str.regist();
}

void JZNodeObjectManager::regist(JZNodeObjectDefine define,QString super)
{
    Q_ASSERT(!define.className.isEmpty() && (super.isEmpty() || meta(super)));

    JZNodeObjectDefine *def = new JZNodeObjectDefine();
    *def = define;
    if(!super.isEmpty())
    {
        def->super = meta(super);
    }
    m_metas.insert(define.className,def);
}

void JZNodeObjectManager::registCClass(JZNodeObjectDefine define,QString type_id,QString super)
{
    regist(define,super);
    m_typeidMetas[type_id] = define.className;
}

void JZNodeObjectManager::unregist(QString name)
{
    m_metas.remove(name);
}

JZNodeObjectDefine *JZNodeObjectManager::meta(QString name)
{
    return m_metas.value(name,nullptr);
}

void JZNodeObjectManager::copy(JZNodeObject *src,JZNodeObject *dst)
{
    if(src->isCObject())    
    {
        src->define->cMeta.copy(src->cobj,dst->cobj);
        return;
    }

    auto it = dst->params.begin();
    while(it != dst->params.end() )
    {
        if((int)it.value().userType() == qMetaTypeId<JZNodeObjectPtr>())
        {
            JZNodeObjectPtr src_ptr = src->params[it.key()].value<JZNodeObjectPtr>();
            JZNodeObjectPtr dst_ptr = it.value().value<JZNodeObjectPtr>();
            copy(src_ptr.data(),src_ptr.data());
        }
        else
            src->params[it.key()] = it.value();
        it++;
    }
}

void JZNodeObjectManager::create(JZNodeObjectDefine *def,JZNodeObject *obj)
{
    if(def->super)
        create(def->super,obj);

    auto it = def->params.begin();
    while(it != def->params.end() )
    {
        if((int)it.value().userType() == qMetaTypeId<JZNodeObjectDelcare>())
        {
            JZNodeObjectDelcare delcare = it.value().value<JZNodeObjectDelcare>();
            JZNodeObjectDefine *sub_def = meta(delcare.className);
            obj->params[it.key()] = QVariant::fromValue(create(sub_def->className));
        }
        else
            obj->params[it.key()] = it.value();
        it++;
    }
}

JZNodeObjectPtr JZNodeObjectManager::create(QString name)
{
    JZNodeObjectDefine *def = meta(name);
    if(!def)
        return nullptr;

    JZNodeObject *obj = new JZNodeObject(def);
    if(!def->isCObject)
        create(def,obj);
    return JZNodeObjectPtr(obj);
}

JZNodeObjectPtr JZNodeObjectManager::createCClass(QString type_id)
{
    return create(m_typeidMetas[type_id]);
}

JZNodeObjectPtr JZNodeObjectManager::clone(JZNodeObjectPtr other)
{
    JZNodeObjectPtr ret = create(other->define->className);
    copy(ret.data(),other.data());    
    return ret;
}
