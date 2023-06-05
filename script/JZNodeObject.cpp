#include "JZNodeObject.h"
#include <QMetaObject>
#include "JZNodeBind.h"
#include <QWidget>
#include <QLineEdit>
#include <QRadioButton>
#include <QCheckBox>
#include <QPushButton>
#include <QApplication>

QString JNodeTypeName(const QVariant &v)
{
    if(v.type() < QVariant::UserType)
        return v.typeName();
    else if(v.userType() == qMetaTypeId<JZNodeObjectPtr>())
    {
        JZNodeObjectPtr ptr = v.value<JZNodeObjectPtr>();
        return ptr->define->className;
    }
    else
        return "unknown";
}

void JZNodeDisp(const QVariant &v)
{

}

//JZNodeListIterator
class JZNodeListIterator
{
public:
    void next(){ it++; }
    bool atEnd() { return (it == list->end()); }
    QVariant key() { return (it - list->begin()); }
    QVariant value(){ return *it; }

    QVariantList *list;
    QVariantList::iterator it;
};

//JZNodeMapIterator
class JZNodeMapIterator
{
public:
    void next(){ it++; }
    bool atEnd() { return (it == map->end()); }
    QVariant key() { return it.value(); }
    QVariant value(){ return it.key(); }

    QVariantMap *map;
    QVariantMap::iterator it;
};

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
    cowner = false;
}

JZNodeObject::~JZNodeObject()
{
    if(cowner && define->isCObject)
        define->cMeta.destory(cobj);
}

void JZNodeObject::createCObj()
{
    if(define->isCObject)
    {
        cobj = define->cMeta.create();
        cowner = true;
    }
}

bool JZNodeObject::isCObject() const
{
    return define->isCObject;
}

bool JZNodeObject::isString() const
{
    return (define->className == "string");
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
    if(className == "list")
        dbg << *((QVariantList*)obj->cobj);
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
    m_objectId = 0;
}

JZNodeObjectManager::~JZNodeObjectManager()
{
    for(auto ptr:m_metas)
        delete ptr;
    m_metas.clear();    
}

void JZNodeObjectManager::init()
{  
    JZNodeFunctionManager::instance()->registCFunction("typename",jzbind::createFuncion(JNodeTypeName));
    JZNodeFunctionManager::instance()->registCFunction("print",jzbind::createFuncion(JZNodeDisp));    

    //list
    jzbind::ClassBind<JZNodeListIterator> cls_list_it("listIterator");
    cls_list_it.def("next",&JZNodeListIterator::next);
    cls_list_it.def("atEnd",&JZNodeListIterator::atEnd);
    cls_list_it.def("key",&JZNodeListIterator::key);
    cls_list_it.def("value",&JZNodeListIterator::value);
    cls_list_it.regist();

    jzbind::ClassBind<QVariantList> cls_list("list");
    cls_list.def("iterator",[](QVariantList *list)->QVariant{
        JZNodeObjectPtr it_ptr = JZNodeObjectManager::instance()->create("listIterator");
        auto list_it = (JZNodeListIterator*)it_ptr->cobj;
        list_it->it = list->begin();
        list_it->list = list;
        return QVariant::fromValue(it_ptr);
    });
    cls_list.def("set",[](QVariantList *list,int index,const QVariant &value)
    {
        if(index < 0 || index >= list->size())
            throw std::runtime_error("index out of range");
        (*list)[index] = value;
    });
    cls_list.def("get",[](QVariantList *list,int index)->QVariant{
        if(index < 0 || index >= list->size())
            throw std::runtime_error("index out of range");
        return list->at(index);
    });
    cls_list.def("push_front",&QVariantList::push_front);
    cls_list.def("pop_front",[](QVariantList *list)
    {
        if(list->size() == 0)
            throw std::runtime_error("list is empty");
        list->pop_front();
    });
    cls_list.def("push_back",&QVariantList::push_back);
    cls_list.def("pop_back",[](QVariantList *list){
        if(list->size() == 0)
            throw std::runtime_error("list is empty");
        list->pop_back();
    });
    cls_list.def("resize",[](QVariantList *list,int size){
        while(list->size() > size)
            list->pop_back();
        while(list->size() < size)
            list->push_back(QVariant());
    });
    cls_list.def("size",&QVariantList::size);
    cls_list.regist();

    //map
    jzbind::ClassBind<JZNodeMapIterator> map_it("mapIterator");
    map_it.def("next",&JZNodeListIterator::next);
    map_it.def("atEnd",&JZNodeListIterator::atEnd);
    map_it.def("key",&JZNodeListIterator::key);
    map_it.def("value",&JZNodeListIterator::value);
    map_it.regist();

    jzbind::ClassBind<QVariantMap> cls_map("map");
    cls_map.def("iterator",[](QVariantMap *map)->QVariant{
        JZNodeObjectPtr it_ptr = JZNodeObjectManager::instance()->create("mapIterator");
        auto map_it = (JZNodeMapIterator*)it_ptr->cobj;
        map_it->it = map->begin();
        map_it->map = map;
        return QVariant::fromValue(it_ptr);
    });
    cls_map.def("set",[](QVariantMap *map,const QString &key,const QVariant &value){ map->insert(key,value); });
    cls_map.def("get",[](QVariantMap *map,const QString &key)->QVariant{
        auto it = map->find(key);
        if(it == map->end())
            throw std::runtime_error("no such element");

        return it.value();
    });
    cls_map.def("size",&QVariantMap::size);
    cls_map.regist();

    //string
    jzbind::ClassBind<QString> cls_str("string");
    cls_str.def("append",[](const QString &a,const QString &b)->QString{
        return a + b;
    });
    cls_str.def("left",&QString::left);
    cls_str.def("right",&QString::right);
    cls_str.def("size",&QString::size);
    cls_str.def("replace",[](const QString &in,const QString &before,const QString &after)->QString{
        QString ret = in;
        return ret.replace(before,after);
    });
    cls_str.regist();

    initWidgets();
}

void JZNodeObjectManager::initWidgets()
{/*
    //widget
    jzbind::ClassBind<QWidget> cls_widget("widget","");
    cls_widget.def("setVisible",&QWidget::setVisible);
    cls_widget.def("show",&QWidget::show);
    cls_widget.def("hide",&QWidget::hide);
    cls_widget.regist();

    //lineedit
    jzbind::ClassBind<QLineEdit> cls_lineEdit("lineEdit","widget");
    cls_lineEdit.def("text",&QLineEdit::text);
    cls_lineEdit.def("setText",&QLineEdit::setText);;
    cls_lineEdit.regist();

    //abs_button
    jzbind::ClassBind<QAbstractButton> cls_abs_button("abstractButton","widget");
    cls_abs_button.def("text",&QAbstractButton::text);
    cls_abs_button.def("setText",&QAbstractButton::setText);
    cls_abs_button.regist();

    //button
    jzbind::ClassBind<QWidget> cls_button("button","abstractButton");
    cls_button.regist();

    //button
    jzbind::ClassBind<QWidget> cls_radio_button("radioButton","abstractButton");
    cls_button.regist();
*/
}

void JZNodeObjectManager::regist(JZNodeObjectDefine define,QString super)
{
    Q_ASSERT(!define.className.isEmpty() && (super.isEmpty() || meta(super)));

    JZNodeObjectDefine *def = new JZNodeObjectDefine();
    *def = define;
    def->id = m_objectId++;
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

QString JZNodeObjectManager::getTypeid(QString name)
{
    return m_typeidMetas.key(name,QString());
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
    Q_ASSERT(def);

    JZNodeObject *obj = new JZNodeObject(def);
    if(!def->isCObject)
        create(def,obj);
    else
        obj->createCObj();
    return JZNodeObjectPtr(obj);
}

JZNodeObjectPtr JZNodeObjectManager::createCClass(QString type_id,bool init)
{
    if(!m_typeidMetas.contains(type_id))
        return nullptr;

    JZNodeObjectDefine *def = meta(m_typeidMetas[type_id]);
    Q_ASSERT(def);

    JZNodeObject *obj = new JZNodeObject(def);
    if(init)
        obj->createCObj();
    return JZNodeObjectPtr(obj);
}

JZNodeObjectPtr JZNodeObjectManager::createString(QString text)
{
    JZNodeObjectPtr ptr = create("string");
    *((QString*)ptr->cobj) = text;
    return ptr;
}

JZNodeObjectPtr JZNodeObjectManager::clone(JZNodeObjectPtr other)
{
    JZNodeObjectPtr ret = create(other->define->className);
    copy(ret.data(),other.data());    
    return ret;
}
