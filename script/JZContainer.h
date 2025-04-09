#ifndef JZCONTAINER_H_
#define JZCONTAINER_H_

#include <QMap>
#include <QVariant>
#include <QList>
#include <QVector>
#include "JZNodeObject.h"
#include "JZScriptEnvironment.h"

void checkEmpty(int size);
void checkSize(int index, int size);
void checkContains(bool flag);

template<class T>
void registList(JZScriptEnvironment *env,int type = Type_none)
{
    Q_ASSERT(env == jzbind::bindEnvironment());

    QString name = "QList<" + env->ctypeidToName(typeid(T).name()) + ">";
    jzbind::ClassBind<QList<T>> cls_list(type, name);
    cls_list.def("set", false, [](QList<T>* l,int index,const T & t) {
        checkSize(index, l->size()); 
        (*l)[index] = t; 
    });
    cls_list.def("get", false, [](QList<T>* l,int index) -> T{
        checkSize(index, l->size());
        return (*l)[index];
    });
    cls_list.def("size", false, [](QList<T>* l) ->int { return l->size(); });
    cls_list.def("clear", false, [](QList<T>* l) { l->clear(); });

    cls_list.def("insert", false, [](QList<T>*l,int index, const T & t) {
        checkSize(index, l->size() + 1); 
        l->insert(index, t);
    });
    cls_list.def("push_back", false, [](QList<T>* l, const T& t) {
        l->push_back(t);
    });
    cls_list.def("pop_back", false, [](QList<T>* l){
        checkEmpty(l->size());
        l->pop_back();
    });
    cls_list.def("push_front", false, [](QList<T>* l, const T& t) {
        l->push_back(t);
    });
    cls_list.def("pop_front", false, [](QList<T>* l){
        checkEmpty(l->size());
        l->pop_front();
    });
    cls_list.def("indexOf", false, [](QList<T>* l, const T& t) { return l->indexOf(t); });
    cls_list.def("lastIndexOf", false, [](QList<T>* l, const T& t) { return l->lastIndexOf(t); });
    cls_list.def("removeAt", false, [](QList<T>* l,int index) { 
        checkSize(index, l->size()); 
        return l->removeAt(index); 
    });
    cls_list.def("removeOne", false, [](QList<T>* l, const T& t) { l->removeOne(t); });
    cls_list.def("removeAll", false, [](QList<T>* l, const T& t) { l->removeAll(t); });

    cls_list.def("contains", false, [](QList<T>* l, const T& t)->bool { return l->contains(t);  });
    cls_list.def("mid", false, [](QList<T>* l, int pos, int len) { return l->mid(pos, len);  });
    cls_list.def("append", false, [](QList<T>* l, QList<T>* other) { l->append(*other);  });

    cls_list.regist();
}

template<class Key,class Value>
void registMap(JZScriptEnvironment* env, int type = Type_none)
{
    Q_ASSERT(env == jzbind::bindEnvironment());

    QString key_name = env->ctypeidToName(typeid(Key).name());
    QString value_name = env->ctypeidToName(typeid(Value).name());

    QString name = "QMap<" + key_name + "," + value_name + ">";
    jzbind::ClassBind<QMap<Key,Value>> cls_map(type, name);
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
    cls_map.regist();
}

#endif