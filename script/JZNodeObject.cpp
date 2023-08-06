#include "JZNodeObject.h"
#include <QMetaObject>
#include "JZNodeBind.h"
#include <QWidget>
#include <QLineEdit>
#include <QRadioButton>
#include <QCheckBox>
#include <QPushButton>
#include <QApplication>
#include <QDebug>
#include <QTimer>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QShowEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QMessageBox>
#include "JZNodeUiLoader.h"

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

QStringList JZNodeObjectDefine::paramList()
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

int JZNodeObjectDefine::indexOfFunction(QString function)
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

const FunctionDefine *JZNodeObjectDefine::function(QString name)
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

QStringList JZNodeObjectDefine::JZNodeObjectDefine::singleList()
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

const SingleDefine *JZNodeObjectDefine::single(QString function)
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

QStringList JZNodeObjectDefine::eventList()
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

const EventDefine *JZNodeObjectDefine::event(QString function)
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

JZNodeObjectDefine *JZNodeObjectDefine::super()
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

bool JZNodeObject::isString() const
{
    return (define->className == "string");
}

const QString &JZNodeObject::className() const
{
    return define->className;
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
    if(className == "string")
        dbg << *((QString*)obj->cobj);
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

    initBase();
    initEvent();
    initCore();
    initObjects();
    initWidgets();
    initFunctions();    

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

void JZNodeObjectManager::initBase()
{
    jzbind::ClassBind<QPoint> cls_pt("Point");
    cls_pt.regist();

    jzbind::ClassBind<QPointF> cls_ptf("PointF");
    cls_ptf.regist();

    jzbind::ClassBind<QRect> cls_rect("Rect");
    cls_rect.def("create", false, [](int x, int y, int w,int h)->QRect {
        return QRect(x, y, w, h);
    });
    cls_rect.regist();

    jzbind::ClassBind<QRectF> cls_rectf("RectF");
    cls_rectf.regist();

    jzbind::ClassBind<QColor> cls_color("Color");
    cls_color.def("create", false, [](int r,int g,int b)->QColor{
            return QColor(r, g, b);
        });
    cls_color.regist();

    jzbind::ClassDelcare<QString>("string", Type_string);
}

void JZNodeObjectManager::initCore()
{    
    auto funcInst = JZNodeFunctionManager::instance();
    jzbind::ClassBind<QObject> cls_object("Object");
    cls_object.regist(Type_object);

    //stringlist
    jzbind::ClassBind<QStringList> cls_string_list("StringList");
    cls_string_list.def("join", false, [](const QStringList &list, const QString &sep)->QString {
        return list.join(sep);
    });
    cls_string_list.regist();

    //string 全部只读
    funcInst->registCFunction("string.append",false,jzbind::createFuncion([](const QString &a,const QString &b)->QString{
        return a + b;
    }));
    funcInst->registCFunction("string.left",false,jzbind::createFuncion([](const QString &a,int num)->QString{
        return a.left(num);
    }));
    funcInst->registCFunction("string.right",false,jzbind::createFuncion([](const QString &a,int num)->QString{
        return a.right(num);
    }));
    funcInst->registCFunction("string.size",false,jzbind::createFuncion([](const QString &a)->int{
        return a.size();
    }));
    funcInst->registCFunction("string.replace",false,jzbind::createFuncion([](const QString &in,const QString &before,const QString &after)->QString{
        QString ret = in;
        return ret.replace(before,after);
    }));  

    //list
    jzbind::ClassBind<JZNodeListIterator> cls_list_it("ListIterator");
    cls_list_it.def("next",true,&JZNodeListIterator::next);
    cls_list_it.def("atEnd",true,&JZNodeListIterator::atEnd);
    cls_list_it.def("key",true,&JZNodeListIterator::key);
    cls_list_it.def("value",true,&JZNodeListIterator::value);
    cls_list_it.regist();

    jzbind::ClassBind<QVariantList> cls_list("List");
    cls_list.def("iterator",true,[](QVariantList *list)->QVariant{
        JZNodeObjectPtr it_ptr = JZObjectCreate<JZNodeListIterator>();
        auto list_it = (JZNodeListIterator*)it_ptr->cobj;
        list_it->it = list->begin();
        list_it->list = list;
        return QVariant::fromValue(it_ptr);
    });
    cls_list.def("resize", true, [](QVariantList *list, int size) {
        while (list->size() > size) {
            list->pop_back();
        }
        while (list->size() < size) {
            list->push_back(QVariant());
        }
    });
    cls_list.def("set",true,[](QVariantList *list,int index,const QVariant &value)
    {
        if (index < 0 || index >= list->size())
        {
            QString error = QString::asprintf("index %d out of range %d",index,list->size());
            throw std::runtime_error(qPrintable(error));
        }
        (*list)[index] = value;
    });
    cls_list.def("get",false,[](QVariantList *list,int index)->QVariant{
        if (index < 0 || index >= list->size())
        {
            QString error = QString::asprintf("index %d out of range %d", index, list->size());
            throw std::runtime_error(qPrintable(error));
        }
        return list->at(index);
    });
    cls_list.def("push_front",true,&QVariantList::push_front);
    cls_list.def("pop_front",true,[](QVariantList *list)
    {
        if(list->size() == 0)
            throw std::runtime_error("list is empty");
        list->pop_front();
    });
    cls_list.def("push_back",true,&QVariantList::push_back);
    cls_list.def("pop_back",true,[](QVariantList *list){
        if(list->size() == 0)
            throw std::runtime_error("list is empty");
        list->pop_back();
    });
    cls_list.def("resize",true,[](QVariantList *list,int size){
        while(list->size() > size)
            list->pop_back();
        while(list->size() < size)
            list->push_back(QVariant());
    });
    cls_list.def("size",false,&QVariantList::size);
    cls_list.def("clear",true,&QVariantList::clear);
    cls_list.regist(Type_list);

    //map
    jzbind::ClassBind<JZNodeMapIterator> map_it("MapIterator");
    map_it.def("next",true,&JZNodeListIterator::next);
    map_it.def("atEnd",true,&JZNodeListIterator::atEnd);
    map_it.def("key",true,&JZNodeListIterator::key);
    map_it.def("value",true,&JZNodeListIterator::value);
    map_it.regist();

    jzbind::ClassBind<QVariantMap> cls_map("Map");
    cls_map.def("iterator",true,[](QVariantMap *map)->QVariant{
        JZNodeObjectPtr it_ptr = JZObjectCreate<JZNodeMapIterator>();
        auto map_it = (JZNodeMapIterator*)it_ptr->cobj;
        map_it->it = map->begin();
        map_it->map = map;
        return QVariant::fromValue(it_ptr);
    });
    cls_map.def("set",true,[](QVariantMap *map,const QString &key,const QVariant &value){ map->insert(key,value); });
    cls_map.def("get",false,[](QVariantMap *map,const QString &key)->QVariant{
        auto it = map->find(key);
        if (it == map->end())
        {
            QString error = "no such element, key = " + key;
            throw std::runtime_error(qPrintable(error));
        }

        return it.value();
    });
    cls_map.def("size",false,&QVariantMap::size);    
    cls_map.def("clear",true, &QVariantMap::clear);
    cls_map.regist(Type_map);
}


void JZNodeObjectManager::initEvent()
{
    jzbind::registEnum<Qt::KeyboardModifiers>(QString("KeypadModifier"));

    jzbind::ClassBind<QEvent> cls_event("Event");
    cls_event.regist();

    jzbind::ClassBind<QResizeEvent> cls_resize_event("ResizeEvent");
    cls_resize_event.regist();

    jzbind::ClassBind<QShowEvent> cls_show_event("ShowEvent");
    cls_show_event.regist();

    jzbind::ClassBind<QPaintEvent> cls_paint_event("PaintEvent");    
    cls_paint_event.regist();

    jzbind::ClassBind<QCloseEvent> cls_close_event("CloseEvent");
    cls_close_event.regist();

    jzbind::ClassBind<QKeyEvent> cls_key_event("KeyEvent");
    cls_key_event.def("key", false, &QKeyEvent::key);
    cls_key_event.def("modifiers", false, &QKeyEvent::modifiers);
    cls_key_event.regist();

    jzbind::ClassBind<QMouseEvent> cls_mouse_event("MouseEvent");
    cls_mouse_event.def("pos", false, &QMouseEvent::pos);
    cls_mouse_event.def("x", false, &QMouseEvent::x);
    cls_mouse_event.def("y", false, &QMouseEvent::y);
    cls_mouse_event.regist();
}

void JZNodeObjectManager::initObjects()
{
    jzbind::ClassBind<QTimer> cls_timer("Timer", "Object");
    cls_timer.def("start",true, QOverload<int>::of(&QTimer::start));
    cls_timer.def("stop",true, &QTimer::stop);
    cls_timer.def("isActive", false, &QTimer::isActive);
    cls_timer.defPrivateSingle("timeout", Event_timeout, &QTimer::timeout);
    cls_timer.regist(Type_timer);
}

void JZNodeObjectManager::initWidgets()
{
    //widget
    jzbind::ClassBind<QWidget> cls_widget("Widget","Object");
    cls_widget.def("setVisible",true,&QWidget::setVisible);
    cls_widget.def("show",true,&QWidget::show);
    cls_widget.def("hide",true,&QWidget::hide);
    cls_widget.def("close",true,&QWidget::close);
    cls_widget.def("rect", false, &QWidget::rect);
    cls_widget.def("update",true, QOverload<>::of(&QWidget::update));
    cls_widget.regist();

    //lineedit
    jzbind::ClassBind<QLineEdit> cls_lineEdit("LineEdit","Widget");
    cls_lineEdit.def("text",false,&QLineEdit::text);
    cls_lineEdit.def("setText",true,&QLineEdit::setText);;
    cls_lineEdit.regist();

    //abs_button
    jzbind::ClassBind<QAbstractButton> cls_abs_button("AbstractButton","Widget");
    cls_abs_button.def("text",false,&QAbstractButton::text);
    cls_abs_button.def("setText",true,&QAbstractButton::setText);
    cls_abs_button.defSingle("clicked",Event_buttonClicked,&QAbstractButton::clicked);
    cls_abs_button.regist();

    //button
    jzbind::ClassBind<QPushButton> cls_button("PushButton","AbstractButton");
    cls_button.regist();

    //button
    jzbind::ClassBind<QCheckBox> cls_check_box("RadioButton","AbstractButton");
    cls_check_box.regist();

    //painter
    jzbind::ClassBind<QPainter> cls_painter("Painter");
    cls_painter.def("create", true, [](QWidget *w)->QPainter*{ return new QPainter(w);}, false);
    cls_painter.def("drawRect",true, QOverload<const QRect&>::of(&QPainter::drawRect));
    cls_painter.def("fillRect",true, QOverload<const QRect&,const QColor&>::of(&QPainter::fillRect));
    cls_painter.regist();

    JZNodeFunctionManager::instance()->registCFunction("MessageBox.information", true, jzbind::createFuncion([](QWidget *w,QString text){
            QMessageBox::information(w, "", text);
        }));    
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
        obj->cobj = loader.create(def->widgteXml);
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
