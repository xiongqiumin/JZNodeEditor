#include <functional>
#include "JZContainer.h"
#include "JZNodeObject.h"
#include "JZNodeBind.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeObjectParser.h"

class ContainerFunction : public CFunction
{
public:
    virtual void call(const QVariantList &in, QVariantList &out) override
    {
        func(in, out);
    }

    std::function<void(const QVariantList &, QVariantList &)> func;
};

// JZList
QString JZList::type() const
{
    return "List<" + valueType + ">";
}

void listCheckSize(int index, int size)
{
    if (index < 0 || index >= size)
    {
        QString error = QString::asprintf("index %d out of range %d", index, size);
        throw std::runtime_error(qPrintable(error));
    }
}

void listSet(const QVariantList &in, QVariantList &out)
{
    auto obj = (JZList *)toJZObject(in[0])->cobj();
    listCheckSize(in[1].toInt(),obj->list.size());
    obj->list[in[1].toInt()] = in;
}
void listGet(const QVariantList &in, QVariantList &out)
{
    auto obj = (JZList *)toJZObject(in[0])->cobj();
    listCheckSize(in[1].toInt(),obj->list.size());
    out << obj->list[in[1].toInt()];
}
void listSize(const QVariantList &in, QVariantList &out)
{
    auto obj = (JZList *)toJZObject(in[0])->cobj();
    out << obj->list.size();
}
void listClear(const QVariantList &in, QVariantList &out)
{
    auto obj = (JZList *)toJZObject(in[0])->cobj();
    obj->list.clear();
}
void listResize(const QVariantList &in, QVariantList &out)
{
    auto obj = (JZList *)toJZObject(in[0])->cobj();
    int cur_size = obj->list.size();
    int size = in[1].toInt();
    int data_type = JZNodeType::nameToType(obj->valueType);

    if(cur_size < size)
    {
        for(int i = cur_size; i < size; i++)
            obj->list.push_back(JZNodeType::defaultValue(data_type));
    }
    else
    {
        for(int i = cur_size; i >= size; i--)
            obj->list.pop_back();
    }
}
void listPushBack(const QVariantList &in, QVariantList &out)
{
    auto obj = (JZList *)toJZObject(in[0])->cobj();
    obj->list.push_back(in[1]);
}
void listPopBack(const QVariantList &in, QVariantList &out)
{
    auto obj = (JZList *)toJZObject(in[0])->cobj();
    obj->list.pop_back();
}
void listPushFront(const QVariantList &in, QVariantList &out)
{
    auto obj = (JZList *)toJZObject(in[0])->cobj();
    obj->list.push_front(in[1]);
}
void listPopFront(const QVariantList &in, QVariantList &out)
{
    auto obj = (JZList *)toJZObject(in[0])->cobj();
    obj->list.pop_front();
}
void listIterator(const QVariantList &in, QVariantList &out)
{
}
void listRemoveAt(const QVariantList &in, QVariantList &out)
{
    auto obj = (JZList *)toJZObject(in[0])->cobj();
    obj->list.removeAt(in[1].toInt());
}
void listRemoveOne(const QVariantList &in, QVariantList &out)
{
    auto obj = (JZList *)toJZObject(in[0])->cobj();
    obj->list.removeOne(in[1]);
}
void listRemoveAll(const QVariantList &in, QVariantList &out)
{
    auto obj = (JZList *)toJZObject(in[0])->cobj();
    obj->list.removeAll(in[1]);
}
void listIndexOf(const QVariantList &in, QVariantList &out)
{
    auto obj = (JZList *)toJZObject(in[0])->cobj();
    out << obj->list.indexOf(in[1],in[2].toInt());
}
void listLastIndexOf(const QVariantList &in, QVariantList &out)
{
    auto obj = (JZList *)toJZObject(in[0])->cobj();
    out << obj->list.lastIndexOf(in[1],in[2].toInt());
}
void listMid(const QVariantList &in, QVariantList &out)
{
    auto obj = (JZList *)toJZObject(in[0])->cobj();

    auto ptr = JZObjectCreate<JZList>();
    auto ret = (JZList *)ptr->cobj();
    ret->list = obj->list.mid(in[1].toInt(),in[2].toInt());
    out << QVariant::fromValue(ptr);
}
void listAppend(const QVariantList &in, QVariantList &out)
{
    auto obj1 = (JZList *)toJZObject(in[0])->cobj();
    auto obj2 = (JZList *)toJZObject(in[0])->cobj();
    obj1->list.append(obj2->list);
}

// JZListIterator
JZListIterator::JZListIterator()
{
    index = 0;
    list = nullptr;
}

void listIteratorNext(const QVariantList &in, QVariantList &out)
{
    auto it = (JZListIterator *)toJZObject(in[0])->cobj();
    it->index++;
}

bool listIteratorAtEnd(const QVariantList &in, QVariantList &out)
{
    auto it = (JZListIterator *)toJZObject(in[0])->cobj();
    return (it->index == it->list->list.size());
}

int listIteratorKey(const QVariantList &in, QVariantList &out)
{
    auto it = (JZListIterator *)toJZObject(in[0])->cobj();
    return it->index;
}

const QVariant &listIteratorValue(const QVariantList &in, QVariantList &out)
{
    auto it = (JZListIterator *)toJZObject(in[0])->cobj();
    listCheckSize(it->index, it->list->list.size());
    return it->list->list[it->index];
}

void registFunction(const JZFunctionDefine &def, std::function<void(const QVariantList &, QVariantList &)> ptr)
{
    auto func_inst = JZNodeFunctionManager::instance();

    ContainerFunction *func = new ContainerFunction();
    func->func = ptr;
    func_inst->registCFunction(def, QSharedPointer<CFunction>(func));
}

void registList(QString type,int type_id)
{
    auto inst = JZNodeObjectManager::instance();

    QString list_name = "List<" + type + ">";
    QString it_name = "ListIterator<" + type + ">";
    JZFunctionDefine func_def;

    int it_id = inst->delcare(it_name);

    auto create_list = [type]()->void*{
        JZList *list = new JZList();
        list->valueType = type;
        return list;
    };

    auto from_string = [list_name](const QVariantList &in, QVariantList &out){
        JZNodeObjectParser parser;
        auto obj = parser.parseToType(list_name,in[0].toString());
        if(!obj)
            throw std::runtime_error(qUtf8Printable(parser.error()));
        out << QVariant::fromValue(obj);
    };

    // list
    JZNodeObjectDefine list;
    list.id = type_id;
    list.className = list_name;
    list.isCObject = true;
    list.cMeta.isCopyable = true;
    list.cMeta.create = create_list;
    list.cMeta.destory = jzbind::destoryClass<JZList>;
    list.cMeta.copy = jzbind::copyClass<JZList>;

    // __fromString__
    func_def = list.initStaticFunction("__fromString__");
    func_def.isCFunction = true;
    func_def.isFlowFunction = false;
    func_def.paramIn.push_back(JZParamDefine("value", Type_string));
    func_def.paramOut.push_back(JZParamDefine("object", list_name));
    list.addFunction(func_def);
    registFunction(func_def, from_string);

    // set
    func_def = list.initMemberFunction("set");
    func_def.isCFunction = true;
    func_def.isFlowFunction = true;
    func_def.paramIn.push_back(JZParamDefine("index", Type_int));
    func_def.paramIn.push_back(JZParamDefine("value", type));
    list.addFunction(func_def);
    registFunction(func_def, listSet);

    // get
    func_def = list.initMemberFunction("get");
    func_def.isCFunction = true;
    func_def.isFlowFunction = false;
    func_def.paramIn.push_back(JZParamDefine("index", Type_int));
    func_def.paramOut.push_back(JZParamDefine("value", type));
    list.addFunction(func_def);
    registFunction(func_def, listGet);

    // size
    func_def = list.initMemberFunction("size");
    func_def.isCFunction = true;
    func_def.isFlowFunction = false;
    list.addFunction(func_def);
    registFunction(func_def, listSize);

    // clear
    func_def = list.initMemberFunction("clear");
    func_def.isCFunction = true;
    func_def.isFlowFunction = true;
    list.addFunction(func_def);
    registFunction(func_def, listClear);

    // resize
    func_def = list.initMemberFunction("resize");
    func_def.isCFunction = true;
    func_def.isFlowFunction = true;
    func_def.paramIn.push_back(JZParamDefine("index", Type_int));
    list.addFunction(func_def);
    registFunction(func_def, listResize);

    // push_back
    func_def = list.initMemberFunction("push_back");
    func_def.isCFunction = true;
    func_def.isFlowFunction = true;
    func_def.paramIn.push_back(JZParamDefine("value", type));
    list.addFunction(func_def);
    registFunction(func_def, listPushBack);

    // pop_back
    func_def = list.initMemberFunction("pop_back");
    func_def.isCFunction = true;
    func_def.isFlowFunction = true;
    list.addFunction(func_def);
    registFunction(func_def, listPopBack);

    // push_front
    func_def = list.initMemberFunction("push_front");
    func_def.isCFunction = true;
    func_def.isFlowFunction = true;
    func_def.paramIn.push_back(JZParamDefine("value", type));
    list.addFunction(func_def);
    registFunction(func_def, listPushFront);

    // pop_front
    func_def = list.initMemberFunction("pop_front");
    func_def.isCFunction = true;
    func_def.isFlowFunction = true;
    list.addFunction(func_def);
    registFunction(func_def, listPopFront);

    // iterator
    func_def = list.initMemberFunction("iterator");
    func_def.isCFunction = true;
    func_def.isFlowFunction = false;
    func_def.paramOut.push_back(JZParamDefine("iterator", it_name));
    list.addFunction(func_def);
    registFunction(func_def, listIterator);

    // removeAt
    func_def = list.initMemberFunction("removeAt");
    func_def.isCFunction = true;
    func_def.isFlowFunction = true;
    func_def.paramIn.push_back(JZParamDefine("index", Type_int));
    list.addFunction(func_def);
    registFunction(func_def, listRemoveAt);

    // removeOne
    func_def = list.initMemberFunction("removeOne");
    func_def.isCFunction = true;
    func_def.isFlowFunction = true;
    func_def.paramIn.push_back(JZParamDefine("value", type));
    list.addFunction(func_def);
    registFunction(func_def, listRemoveOne);

    // removeAll
    func_def = list.initMemberFunction("removeAll");
    func_def.isCFunction = true;
    func_def.isFlowFunction = true;
    func_def.paramIn.push_back(JZParamDefine("value", type));
    list.addFunction(func_def);
    registFunction(func_def, listRemoveAll);

    // indexOf
    func_def = list.initMemberFunction("indexOf");
    func_def.isCFunction = true;
    func_def.isFlowFunction = false;
    func_def.paramIn.push_back(JZParamDefine("value", type));
    func_def.paramIn.push_back(JZParamDefine("start", Type_int, "0"));
    func_def.paramOut.push_back(JZParamDefine("index", Type_int));
    list.addFunction(func_def);
    registFunction(func_def, listIndexOf);

    // lastIndexOf
    func_def = list.initMemberFunction("lastIndexOf");
    func_def.isCFunction = true;
    func_def.isFlowFunction = false;
    func_def.paramIn.push_back(JZParamDefine("value", type));
    func_def.paramOut.push_back(JZParamDefine("start", Type_int, "-1"));
    list.addFunction(func_def);
    registFunction(func_def, listLastIndexOf);

    // mid
    func_def = list.initMemberFunction("mid");
    func_def.isCFunction = true;
    func_def.isFlowFunction = false;
    func_def.paramIn.push_back(JZParamDefine("start", Type_int));
    func_def.paramIn.push_back(JZParamDefine("length", Type_int));
    func_def.paramOut.push_back(JZParamDefine("list", list_name));
    list.addFunction(func_def);
    registFunction(func_def, listMid);

    // append
    func_def = list.initMemberFunction("append");
    func_def.isCFunction = true;
    func_def.isFlowFunction = true;
    func_def.paramIn.push_back(JZParamDefine("list", list_name));
    list.addFunction(func_def);
    registFunction(func_def, listAppend);

    inst->regist(list);

    JZNodeObjectDefine list_it;
    list_it.className = it_name;
    list_it.id = it_id;
    inst->regist(list_it);
}

//JZMap
bool JZMap::Key::operator<(const JZMap::Key &other) const
{
    Q_ASSERT(JZNodeType::variantType(v) == JZNodeType::variantType(other.v));
    int type = JZNodeType::variantType(v);
    if(type == Type_int)
        return v.toInt() < other.v.toInt();
    else if(type == Type_string)
        return v.toString() < other.v.toString();
    else if(type > Type_object)
        return toJZObject(v) < toJZObject(other.v);
    else
    {
        Q_ASSERT(0);
        return false;
    }
}

QString JZMap::type() const
{
    return "Map<" + keyType + "," + valueType + ">";
}

void mapSet(const QVariantList &in, QVariantList &out)
{
    auto obj = (JZMap *)toJZObject(in[0])->cobj();
    JZMap::Key key;
    key.v = in[0];
    obj->map.insert(key,in[1]);
}

void mapGet(const QVariantList &in, QVariantList &out)
{
    auto obj = (JZMap *)toJZObject(in[0])->cobj();
    JZMap::Key key;
    key.v = in[0];

    auto it = obj->map.find(key);
    if(it == obj->map.end())
        throw std::runtime_error("no element");

    out << obj->map[key];
}
void mapSize(const QVariantList &in, QVariantList &out)
{
    auto obj = (JZMap *)toJZObject(in[0])->cobj();
    out << obj->map.size();
}

void mapClear(const QVariantList &in, QVariantList &out)
{
    auto obj = (JZMap *)toJZObject(in[0])->cobj();
    obj->map.clear();
}

void mapContains(const QVariantList &in, QVariantList &out)
{
    auto obj = (JZMap *)toJZObject(in[0])->cobj();
    JZMap::Key key;
    key.v = in[0];
    out << obj->map.contains(key);
}

void registMap(QString key_type, QString value_type,int type_id)
{
    auto inst = JZNodeObjectManager::instance();

    QString map_name = "Map<" + key_type + "," + value_type + ">";
    QString it_name = "MapIterator<" + key_type + "," + value_type + ">";
    JZFunctionDefine func_def;

    int it_id = inst->delcare(it_name);

    auto create_map = [key_type,value_type]()->void*{
        JZMap *map = new JZMap();
        map->keyType = key_type;
        map->valueType = value_type;
        return map;
    };

    auto from_string = [map_name](const QVariantList &in, QVariantList &out){
        JZNodeObjectParser parser;
        auto obj = parser.parseToType(map_name,in[0].toString());
        if(!obj)
            throw std::runtime_error(qUtf8Printable(parser.error()));
        out << QVariant::fromValue(obj);
    };

    // list
    JZNodeObjectDefine map;
    map.id = type_id;
    map.className = map_name;
    map.isCObject = true;
    map.cMeta.isCopyable = true;
    map.cMeta.create = create_map;
    map.cMeta.destory = jzbind::destoryClass<JZMap>;
    map.cMeta.copy = jzbind::copyClass<JZMap>;

    // __fromString__
    func_def = map.initStaticFunction("__fromString__");
    func_def.isCFunction = true;
    func_def.isFlowFunction = false;
    func_def.paramIn.push_back(JZParamDefine("value", Type_string));
    func_def.paramOut.push_back(JZParamDefine("object", map_name));
    map.addFunction(func_def);
    registFunction(func_def, from_string);

    // set
    func_def = map.initMemberFunction("set");
    func_def.isCFunction = true;
    func_def.isFlowFunction = true;
    func_def.paramIn.push_back(JZParamDefine("index", key_type));
    func_def.paramIn.push_back(JZParamDefine("value", value_type));
    map.addFunction(func_def);
    registFunction(func_def, mapSet);

    // get
    func_def = map.initMemberFunction("get");
    func_def.isCFunction = true;
    func_def.isFlowFunction = false;
    func_def.paramIn.push_back(JZParamDefine("index", key_type));
    func_def.paramOut.push_back(JZParamDefine("value", value_type));
    map.addFunction(func_def);
    registFunction(func_def, mapGet);

    // size
    func_def = map.initMemberFunction("size");
    func_def.isCFunction = true;
    func_def.isFlowFunction = false;
    map.addFunction(func_def);
    registFunction(func_def, mapSize);

    // clear
    func_def = map.initMemberFunction("clear");
    func_def.isCFunction = true;
    func_def.isFlowFunction = true;
    map.addFunction(func_def);
    registFunction(func_def, mapClear);

    // iterator
    func_def = map.initMemberFunction("iterator");
    func_def.isCFunction = true;
    func_def.isFlowFunction = false;
    func_def.paramOut.push_back(JZParamDefine("iterator", it_name));
    map.addFunction(func_def);
    registFunction(func_def, listIterator);

    inst->regist(map);

    JZNodeObjectDefine map_it;
    map_it.className = it_name;
    map_it.id = it_id;
    inst->regist(map_it);
}