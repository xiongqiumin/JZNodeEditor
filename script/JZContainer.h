#ifndef JZCONTAINER_H_
#define JZCONTAINER_H_

#include <QMap>
#include <QVariant>
#include <QList>
#include <QVector>
#include <functional>
#include "JZNodeObject.h"
#include "JZScriptEnvironment.h"

QString listType(QString value);
QString listIteratorType(QString list_type);
QString mapType(QString key, QString value);
QString mapIteratorType(QString map_type);
void checkEmpty(int size);
void checkSize(int index, int size);
void checkContains(bool flag);

template<class T>
void registList(JZScriptEnvironment *env,int type = Type_none)
{
    Q_ASSERT(env == jzbind::bindEnvironment());

    QString name = "QList<" + env->ctypeidToName(typeid(T).name()) + ">";
    jzbind::ClassBind<QList<T>> cls_list(type, name);
    cls_list.setValueType(true);
    cls_list.def("__fromString__", false, [](const QString& text)->QList<T> {
        return QList<T>();
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
    cls_list.def("indexOf", false, [](QList<T>* l, const T& t, int from) { return l->indexOf(t, from); });
    cls_list.def("lastIndexOf", true, [](QList<T>* l, const T& t, int from) { return l->lastIndexOf(t, from); });
    cls_list.def("removeAt", true, [](QList<T>* l,int index) {
        checkSize(index, l->size()); 
        return l->removeAt(index); 
    });
    cls_list.def("removeOne", true, [](QList<T>* l, const T& t) { l->removeOne(t); });
    cls_list.def("removeAll", true, [](QList<T>* l, const T& t) { l->removeAll(t); });

    cls_list.def("contains", false, [](QList<T>* l, const T& t)->bool { return l->contains(t);  });
    cls_list.def("mid", false, [](QList<T>* l, int pos, int len) { return l->mid(pos, len);  });
    cls_list.def("append", true, [](QList<T>* l, QList<T>* other) { l->append(*other);  });

    cls_list.regist();    
}

template<class Key,class Value>
void registMap(JZScriptEnvironment* env, int type = Type_none)
{
    Q_ASSERT(env == jzbind::bindEnvironment());

    QString key_name = env->ctypeidToName(typeid(Key).name());
    QString value_name = env->ctypeidToName(typeid(Value).name());    
    
    QString map_type = mapType(key_name, value_name);
    jzbind::ClassBind<QMap<Key,Value>::iterator> cls_map_it(Type_none, mapIteratorType(map_type));
    
    jzbind::ClassBind<QMap<Key,Value>> cls_map(type, map_type);
    cls_map.setValueType(true);
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