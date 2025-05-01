#ifndef JZCONTAINER_H_
#define JZCONTAINER_H_

#include <QMap>
#include <QVariant>
#include <QList>
#include <QVector>
#include <functional>
#include "JZNodeObjectParser.h"
#include "JZNodeObject.h"
#include "JZScriptEnvironment.h"
#include "JZNodeObjectParser.h"
#include "JZNodeBind.h"

void checkEmpty(int size);
void checkSize(int index, int size);
void checkContains(bool flag);

extern JZNodeObject* JZObjectFromString(int type, const QString& text);

template<typename T, typename = void>
struct has_equal_operator : std::false_type {};

template<typename T>
struct has_equal_operator<T, std::void_t<decltype(std::declval<T>() == std::declval<T>())>> : std::true_type {};

template<class T>
void registListEqual(jzbind::ClassBind<QList<T>> &cls_list, std::true_type)
{
    cls_list.def("indexOf", false, [](QList<T>* l, const T& t, int from) { return l->indexOf(t, from); });
    cls_list.def("lastIndexOf", true, [](QList<T>* l, const T& t, int from) { return l->lastIndexOf(t, from); });
    cls_list.def("removeOne", true, [](QList<T>* l, const T& t) { l->removeOne(t); });
    cls_list.def("removeAll", true, [](QList<T>* l, const T& t) { l->removeAll(t); });
    cls_list.def("contains", false, [](QList<T>* l, const T& t)->bool { return l->contains(t);  });
}

template<class T>
void registListEqual(jzbind::ClassBind<QList<T>> &cls_list, std::false_type)
{
}

template<class T>
void registList(JZScriptEnvironment *env,int type = Type_none)
{
    Q_ASSERT(env == jzbind::bindEnvironment());
    Q_ASSERT(env->ctypeidToType(typeid(T).name()) != Type_none);

    QString list_type = "QList<" + env->ctypeidToName(typeid(T).name()) + ">";

    jzbind::ClassBind<QList<T>> cls_list(type, list_type);
    int list_type_id = cls_list.id();

    cls_list.setValueType(true);
    cls_list.def("__fromString__", false, [list_type_id](const QString& text)->QList<T> {
        JZNodeObject* obj = JZObjectFromString(list_type_id, text);
        QList<T> *ret = JZObjectCast<QList<T>>(obj);
        return *ret;
    });
    cls_list.def("__toString__", false, [](QList<T>* l)->QString {
        return QString();
    });
    cls_list.def("set", true, [](QList<T>* l,int index,const T & t) {
        checkSize(index, l->size()); 
        (*l)[index] = t; 
    });
    cls_list.def("get", false, [](QList<T>* l,int index) -> T{
        checkSize(index, l->size());
        return (*l)[index];
    });
    cls_list.def("size", false, [](QList<T>* l) ->int { return l->size(); });
    cls_list.def("clear", true, [](QList<T>* l) { l->clear(); });

    cls_list.def("insert", true, [](QList<T>*l,int index, const T & t) {
        checkSize(index, l->size() + 1); 
        l->insert(index, t);
    });
    cls_list.def("push_back", true, [](QList<T>* l, const T& t) {
        l->push_back(t);
    });
    cls_list.def("pop_back", true, [](QList<T>* l){
        checkEmpty(l->size());
        l->pop_back();
    });
    cls_list.def("push_front", true, [](QList<T>* l, const T& t) {
        l->push_back(t);
    });
    cls_list.def("pop_front", true, [](QList<T>* l){
        checkEmpty(l->size());
        l->pop_front();
    });    
    cls_list.def("removeAt", true, [](QList<T>* l,int index) {
        checkSize(index, l->size()); 
        return l->removeAt(index); 
    });

    registListEqual(cls_list, has_equal_operator<T>());    

    cls_list.def("mid", false, [](QList<T>* l, int pos, int len) { return l->mid(pos, len);  });
    cls_list.def("append", true, [](QList<T>* l, QList<T>* other) { l->append(*other);  });
    cls_list.def("resize", true, [](QList<T>* l, int size) { 
        
        if (l->size() > size)
        {
            while (l->size() > size)
                l->pop_back();
        }
        else
        {
            for(int i = l->size(); i < size; i++)
                l->append(T());
        }
    });

    cls_list.regist();    
}

template<class Key,class Value>
void registMap(JZScriptEnvironment* env, int type = Type_none)
{
    Q_ASSERT(env == jzbind::bindEnvironment());

    QString key_name = env->ctypeidToName(typeid(Key).name());
    QString value_name = env->ctypeidToName(typeid(Value).name());    
    QString map_type = JZNodeType::mapType(key_name, value_name);

    jzbind::ClassBind<QMap<Key,Value>::iterator> cls_map_it(Type_none, JZNodeType::mapIteratorType(map_type));

    jzbind::ClassBind<QMap<Key,Value>> cls_map(type, map_type);
    int map_type_id = cls_map.id();

    cls_map.setValueType(true);
    cls_map.def("__fromString__", false, [map_type_id](const QString& text)->QMap<Key, Value> {
        JZNodeObject* obj = JZObjectFromString(map_type_id, text);
        QMap<Key, Value> *ret = JZObjectCast<QMap<Key, Value>>(obj);
        return *ret;
    });
    cls_map.def("__toString__", false, [](QMap<Key, Value>* l)->QString {
        JZNodeObjectFormat format;
        JZNodeObjectPointer holder = runtimeEnvironment()->objectManager()->objectReferencePointer(l, false);
        return format.format(holder.object());
    });
    cls_map.def("set", false, [](QMap<Key, Value>* map, Key key, const Value& t) {
        checkContains(map->contains(key));
        (*map)[key] = t;
    });
    cls_map.def("get", false, [](QMap<Key, Value>* map, Key key)->Value {
        checkContains(map->contains(key));
        return (*map)[key];
    });
    cls_map.def("insert", false, [](QMap<Key, Value>* map, Key key, const Value& t) {
        checkContains(!map->contains(key));
        map->insert(key, t);
    });
    cls_map.def("size", false, [](QMap<Key, Value>* map) ->int { return map->size(); });
    cls_map.def("clear", false, [](QMap<Key, Value>* map) { map->clear(); });

    cls_map.def("begin", false, [](QMap<Key, Value>* map)->QMap<Key, Value>::iterator { 
        return map->begin(); 
    });
    cls_map.def("end", false, [](QMap<Key, Value>* map)->QMap<Key, Value>::iterator { 
        return map->end(); 
    });
    cls_map.regist();

    //cls_map_it
    cls_map_it.def("key", true, &QMap<Key, Value>::iterator::key);
    cls_map_it.def("value", true, &QMap<Key, Value>::iterator::value);
    cls_map_it.def("next", true, [](QMap<Key, Value>::iterator *it) {  it++; });
    cls_map_it.regist();
}

void listForeach(JZNodeObject *obj, std::function<bool(int,QVariant)> vistor);
void mapForeach(JZNodeObject *obj,std::function<bool(QVariant,QVariant)> vistor);


#endif