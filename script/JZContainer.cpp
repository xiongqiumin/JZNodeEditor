#include <functional>
#include "JZContainer.h"
#include "JZNodeObject.h"
#include "JZNodeBind.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeObjectParser.h"
#include "JZNodeEngine.h"

class ContainerFunction : public BuiltInFunction
{
public:
    virtual void call(JZNodeEngine *engine) override
    {
        func(engine);
    }

    std::function<void(JZNodeEngine*)> func;
};

void registFunction(JZFunctionDefine &def, std::function<void(JZNodeEngine*)> ptr)
{
    auto func_inst = JZNodeFunctionManager::instance();
    def.isCFunction = true;

    ContainerFunction *func = new ContainerFunction();
    func->func = ptr;
    func_inst->registBuiltInFunction(def, QSharedPointer<BuiltInFunction>(func));
}

// JZList
QString JZList::type() const
{
    return "QList<" + valueType + ">";
}

inline void listCheckSize(int index, int size)
{
    if (index < 0 || index >= size)
    {
        QString error = QString::asprintf("index %d out of range %d", index, size);
        throw std::runtime_error(qPrintable(error));
    }
}

void listSet(JZNodeEngine *engine)
{
    auto obj = (JZList *)toJZObject(engine->getReg(Reg_CallIn))->cobj();
    int index = engine->getReg(Reg_CallIn + 1).toInt();
    listCheckSize(index,obj->list.size());
    obj->list[index] = engine->getReg(Reg_CallIn + 2);
}
void listGet(JZNodeEngine *engine)
{
    auto obj = (JZList *)toJZObject(engine->getReg(Reg_CallIn))->cobj();
    int index = engine->getReg(Reg_CallIn + 1).toInt();
    listCheckSize(index,obj->list.size());
    engine->setReg(Reg_CallOut,obj->list[index]);
}
void listSize(JZNodeEngine *engine)
{
    auto obj = (JZList *)toJZObject(engine->getReg(Reg_CallIn))->cobj();
    engine->setReg(Reg_CallOut,obj->list.size());
}
void listClear(JZNodeEngine *engine)
{
    auto obj = (JZList *)toJZObject(engine->getReg(Reg_CallIn))->cobj();
    obj->list.clear();
}
void listResize(JZNodeEngine *engine)
{
    auto obj = (JZList *)toJZObject(engine->getReg(Reg_CallIn))->cobj();
    int cur_size = obj->list.size();
    int size = engine->getReg(Reg_CallIn + 1).toInt();
    int data_type = JZNodeType::nameToType(obj->valueType);

    if(cur_size < size)
    {
        for(int i = cur_size; i < size; i++)
            obj->list.push_back(engine->createVariable(data_type));
    }
    else
    {
        for(int i = cur_size; i >= size; i--)
            obj->list.pop_back();
    }
}
void listPushBack(JZNodeEngine *engine)
{
    auto obj = (JZList *)toJZObject(engine->getReg(Reg_CallIn))->cobj();
    obj->list.push_back(engine->getReg(Reg_CallIn + 1));
}
void listPopBack(JZNodeEngine *engine)
{
    auto obj = (JZList *)toJZObject(engine->getReg(Reg_CallIn))->cobj();
    obj->list.pop_back();
}
void listPushFront(JZNodeEngine *engine)
{
    auto obj = (JZList *)toJZObject(engine->getReg(Reg_CallIn))->cobj();
    obj->list.push_front(engine->getReg(Reg_CallIn + 1));
}
void listPopFront(JZNodeEngine *engine)
{
    auto obj = (JZList *)toJZObject(engine->getReg(Reg_CallIn))->cobj();
    obj->list.pop_front();
}
void listRemoveAt(JZNodeEngine *engine)
{
    auto obj = (JZList *)toJZObject(engine->getReg(Reg_CallIn))->cobj();
    obj->list.removeAt(engine->getReg(Reg_CallIn + 1).toInt());
}
void listRemoveOne(JZNodeEngine *engine)
{
    auto obj = (JZList *)toJZObject(engine->getReg(Reg_CallIn))->cobj();
    obj->list.removeOne(engine->getReg(Reg_CallIn + 1));
}
void listRemoveAll(JZNodeEngine *engine)
{
    auto obj = (JZList *)toJZObject(engine->getReg(Reg_CallIn))->cobj();
    obj->list.removeAll(engine->getReg(Reg_CallIn + 1));
}
void listIndexOf(JZNodeEngine *engine)
{
    auto obj = (JZList *)toJZObject(engine->getReg(Reg_CallIn))->cobj();
    int from = engine->getReg(Reg_CallIn + 2).toInt();
    int index = obj->list.indexOf(engine->getReg(Reg_CallIn + 1), from);
    engine->setReg(Reg_CallOut,index);
}
void listLastIndexOf(JZNodeEngine *engine)
{
    auto obj = (JZList *)toJZObject(engine->getReg(Reg_CallIn))->cobj();
    int from = engine->getReg(Reg_CallIn + 2).toInt();
    int index = obj->list.lastIndexOf(engine->getReg(Reg_CallIn + 1), from);
    engine->setReg(Reg_CallOut,index);
}
void listContains(JZNodeEngine *engine)
{
    auto obj = (JZList *)toJZObject(engine->getReg(Reg_CallIn))->cobj();
    bool has = obj->list.contains(engine->getReg(Reg_CallIn + 1));
    engine->setReg(Reg_CallOut,has);
}
void listMid(JZNodeEngine *engine)
{
    auto obj = (JZList *)toJZObject(engine->getReg(Reg_CallIn))->cobj();
    auto ptr = JZNodeObjectManager::instance()->create(obj->type());
    auto ret = (JZList *)ptr->cobj();
    int index = engine->getReg(Reg_CallIn + 1).toInt();
    int size = engine->getReg(Reg_CallIn + 2).toInt();
    ret->list = obj->list.mid(index,size);
    engine->setReg(Reg_CallOut,QVariant::fromValue(JZNodeObjectPtr(ptr,true)));
}
void listInsert(JZNodeEngine *engine)
{
    auto obj = (JZList *)toJZObject(engine->getReg(Reg_CallIn))->cobj();
    int index = engine->getReg(Reg_CallIn+1).toInt();
    obj->list.insert(index,engine->getReg(Reg_CallIn + 2));
}
void listAppend(JZNodeEngine *engine)
{
    auto obj1 = (JZList *)toJZObject(engine->getReg(Reg_CallIn))->cobj();
    auto obj2 = (JZList *)toJZObject(engine->getReg(Reg_CallIn+1))->cobj();
    obj1->list.append(obj2->list);
}

void registList(QString type,int type_id)
{
    auto inst = JZNodeObjectManager::instance();
    Q_ASSERT(JZNodeType::isVaildType(type));

    QString list_name = "QList<" + type + ">";
    if(inst->meta(list_name) && inst->meta(list_name)->functions.size() > 0)
        return;

    JZFunctionDefine func_def;
    auto create_list = [type]()->void*{
        JZList *list = new JZList();
        list->valueType = type;
        return list;
    };

    auto from_string = [list_name](JZNodeEngine *engine){
        JZNodeObjectParser parser;
        QString text = engine->getReg(Reg_CallIn).toString();
        auto obj = parser.parseToType(list_name,text);
        if(!obj)
            throw std::runtime_error(qUtf8Printable(parser.error()));
        engine->setReg(Reg_CallOut,QVariant::fromValue(JZNodeObjectPtr(obj,true)));
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
    func_def.isFlowFunction = false;
    func_def.paramIn.push_back(JZParamDefine("value", Type_string));
    func_def.paramOut.push_back(JZParamDefine("object", list_name));
    list.addFunction(func_def);
    registFunction(func_def, from_string);

    // set
    func_def = list.initMemberFunction("set");
    func_def.isFlowFunction = true;
    func_def.paramIn.push_back(JZParamDefine("index", Type_int));
    func_def.paramIn.push_back(JZParamDefine("value", type));
    list.addFunction(func_def);
    registFunction(func_def, listSet);

    // get
    func_def = list.initMemberFunction("get");
    func_def.isFlowFunction = false;
    func_def.paramIn.push_back(JZParamDefine("index", Type_int));
    func_def.paramOut.push_back(JZParamDefine("value", type));
    list.addFunction(func_def);
    registFunction(func_def, listGet);

    // size
    func_def = list.initMemberFunction("size");
    func_def.isFlowFunction = false;
    func_def.paramOut.push_back(JZParamDefine("size", Type_int));
    list.addFunction(func_def);
    registFunction(func_def, listSize);

    // clear
    func_def = list.initMemberFunction("clear");
    func_def.isFlowFunction = true;
    list.addFunction(func_def);
    registFunction(func_def, listClear);

    // resize
    func_def = list.initMemberFunction("resize");
    func_def.isFlowFunction = true;
    func_def.paramIn.push_back(JZParamDefine("index", Type_int));
    list.addFunction(func_def);
    registFunction(func_def, listResize);

    // push_back
    func_def = list.initMemberFunction("push_back");
    func_def.isFlowFunction = true;
    func_def.paramIn.push_back(JZParamDefine("value", type));
    list.addFunction(func_def);
    registFunction(func_def, listPushBack);

    // pop_back
    func_def = list.initMemberFunction("pop_back");
    func_def.isFlowFunction = true;
    list.addFunction(func_def);
    registFunction(func_def, listPopBack);

    // push_front
    func_def = list.initMemberFunction("push_front");
    func_def.isFlowFunction = true;
    func_def.paramIn.push_back(JZParamDefine("value", type));
    list.addFunction(func_def);
    registFunction(func_def, listPushFront);

    // pop_front
    func_def = list.initMemberFunction("pop_front");
    func_def.isFlowFunction = true;
    list.addFunction(func_def);
    registFunction(func_def, listPopFront);

    // removeAt
    func_def = list.initMemberFunction("removeAt");
    func_def.isFlowFunction = true;
    func_def.paramIn.push_back(JZParamDefine("index", Type_int));
    list.addFunction(func_def);
    registFunction(func_def, listRemoveAt);

    // removeOne
    func_def = list.initMemberFunction("removeOne");
    func_def.isFlowFunction = true;
    func_def.paramIn.push_back(JZParamDefine("value", type));
    list.addFunction(func_def);
    registFunction(func_def, listRemoveOne);

    // removeAll
    func_def = list.initMemberFunction("removeAll");
    func_def.isFlowFunction = true;
    func_def.paramIn.push_back(JZParamDefine("value", type));
    list.addFunction(func_def);
    registFunction(func_def, listRemoveAll);

    // contains
    func_def = list.initMemberFunction("contains");
    func_def.isFlowFunction = false;
    func_def.paramIn.push_back(JZParamDefine("value", type));
    func_def.paramOut.push_back(JZParamDefine("has", Type_bool));
    list.addFunction(func_def);
    registFunction(func_def, listContains);

    // indexOf
    func_def = list.initMemberFunction("indexOf");
    func_def.isFlowFunction = false;
    func_def.paramIn.push_back(JZParamDefine("value", type));
    func_def.paramIn.push_back(JZParamDefine("start", Type_int, "0"));
    func_def.paramOut.push_back(JZParamDefine("index", Type_int));
    list.addFunction(func_def);
    registFunction(func_def, listIndexOf);

    // lastIndexOf
    func_def = list.initMemberFunction("lastIndexOf");
    func_def.isFlowFunction = false;
    func_def.paramIn.push_back(JZParamDefine("value", type));
    func_def.paramIn.push_back(JZParamDefine("start", Type_int, "-1"));
    func_def.paramOut.push_back(JZParamDefine("index", Type_int));
    list.addFunction(func_def);
    registFunction(func_def, listLastIndexOf);

    // mid
    func_def = list.initMemberFunction("mid");
    func_def.isFlowFunction = false;
    func_def.paramIn.push_back(JZParamDefine("start", Type_int));
    func_def.paramIn.push_back(JZParamDefine("length", Type_int));
    func_def.paramOut.push_back(JZParamDefine("list", list_name));
    list.addFunction(func_def);
    registFunction(func_def, listMid);

    // insert
    func_def = list.initMemberFunction("insert");
    func_def.isFlowFunction = true;
    func_def.paramIn.push_back(JZParamDefine("index", Type_int));
    func_def.paramIn.push_back(JZParamDefine("value", type));
    list.addFunction(func_def);
    registFunction(func_def, listInsert);

    // append
    func_def = list.initMemberFunction("append");
    func_def.isFlowFunction = true;
    func_def.paramIn.push_back(JZParamDefine("list", list_name));
    list.addFunction(func_def);
    registFunction(func_def, listAppend);

    inst->regist(list);
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
    else if(type >= Type_class)
        return toJZObject(v) < toJZObject(other.v);
    else
    {
        Q_ASSERT(0);
        return false;
    }
}

QString JZMap::type() const
{
    return "QMap<" + keyType + "," + valueType + ">";
}

void mapSet(JZNodeEngine *engine)
{
    auto obj = (JZMap *)toJZObject(engine->getReg(Reg_CallIn))->cobj();
    JZMap::Key key;
    key.v = engine->getReg(Reg_CallIn+1);
    obj->map.insert(key,engine->getReg(Reg_CallIn+2));
}

void mapGet(JZNodeEngine *engine)
{
    auto obj = (JZMap *)toJZObject(engine->getReg(Reg_CallIn))->cobj();
    JZMap::Key key;
    key.v = engine->getReg(Reg_CallIn+1);

    auto it = obj->map.find(key);
    if(it == obj->map.end())
        throw std::runtime_error("no element");

    engine->setReg(Reg_CallOut,obj->map[key]);
}
void mapSize(JZNodeEngine *engine)
{
    auto obj = (JZMap *)toJZObject(engine->getReg(Reg_CallIn))->cobj();
    engine->setReg(Reg_CallOut,obj->map.size());
}

void mapClear(JZNodeEngine *engine)
{
    auto obj = (JZMap *)toJZObject(engine->getReg(Reg_CallIn))->cobj();
    obj->map.clear();
}

void mapContains(JZNodeEngine *engine)
{
    auto obj = (JZMap *)toJZObject(engine->getReg(Reg_CallIn))->cobj();
    JZMap::Key key;
    key.v = engine->getReg(Reg_CallIn+1);
    engine->setReg(Reg_CallOut,obj->map.contains(key));
}

void registMap(QString key_type, QString value_type,int type_id)
{
    auto inst = JZNodeObjectManager::instance();
    Q_ASSERT(JZNodeType::isVaildType(key_type) && JZNodeType::isVaildType(value_type));

    QString map_name = "QMap<" + key_type + "," + value_type + ">";
    QString it_name = "QMapIterator<" + key_type + "," + value_type + ">";
    if(inst->meta(map_name) && inst->meta(map_name)->functions.size() > 0)
        return;

    JZFunctionDefine func_def;
    int it_id = inst->delcare(it_name);

    auto create_map = [key_type,value_type]()->void*{
        JZMap *map = new JZMap();
        map->keyType = key_type;
        map->valueType = value_type;
        return map;
    };

    auto from_string = [map_name](JZNodeEngine *engine){
        JZNodeObjectParser parser;
        QString text = engine->getReg(Reg_CallIn).toString();
        auto obj = parser.parseToType(map_name,text);
        if(!obj)
            throw std::runtime_error(qUtf8Printable(parser.error()));
        engine->setReg(Reg_CallOut,QVariant::fromValue(JZNodeObjectPtr(obj,true)));
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
    func_def.isFlowFunction = false;
    func_def.paramIn.push_back(JZParamDefine("value", Type_string));
    func_def.paramOut.push_back(JZParamDefine("object", map_name));
    map.addFunction(func_def);
    registFunction(func_def, from_string);

    // set
    func_def = map.initMemberFunction("set");
    func_def.isFlowFunction = true;
    func_def.paramIn.push_back(JZParamDefine("index", key_type));
    func_def.paramIn.push_back(JZParamDefine("value", value_type));
    map.addFunction(func_def);
    registFunction(func_def, mapSet);

    // get
    func_def = map.initMemberFunction("get");
    func_def.isFlowFunction = false;
    func_def.paramIn.push_back(JZParamDefine("index", key_type));
    func_def.paramOut.push_back(JZParamDefine("value", value_type));
    map.addFunction(func_def);
    registFunction(func_def, mapGet);

    // size
    func_def = map.initMemberFunction("size");
    func_def.isFlowFunction = false;
    func_def.paramOut.push_back(JZParamDefine("size", Type_int));
    map.addFunction(func_def);
    registFunction(func_def, mapSize);

    // clear
    func_def = map.initMemberFunction("clear");
    func_def.isFlowFunction = true;
    map.addFunction(func_def);
    registFunction(func_def, mapClear);

    inst->regist(map);

    JZNodeObjectDefine map_it;
    map_it.className = it_name;
    map_it.isCObject = true;
    map_it.id = it_id;
    inst->replace(map_it);
}

void registSet(QString value_type,int type_id)
{
    auto inst = JZNodeObjectManager::instance();
    QString map_name = "Set<" + value_type + ">";
    QString it_name = "SetIterator<" + value_type + ">";
    if(inst->meta(map_name) && inst->meta(map_name)->functions.size() > 0)
        return;
}

TemplateInfo parseTemplate(QString type)
{
    TemplateInfo info;
    int start_idx = type.indexOf("<");
    int end_idx = type.lastIndexOf(">");
    if(start_idx == -1 || end_idx == -1)
    {
        info.error = type + " format error";
        return info;
    }

    info.name = type.left(start_idx); 
    start_idx++;

    QString arg = type.mid(start_idx,end_idx - start_idx);
    info.args = arg.split(",");
    return info;
}   

bool checkContainer(QString type,QString &error)
{
    QString list_pre = "QList<";
    QString map_pre = "QMap<";
    QString set_pre = "QSet<";

    TemplateInfo info = parseTemplate(type);
    if(!info.error.isEmpty())
    {
        error = info.error;
        return false;
    }

    if((info.name == "QList" && info.args.size() == 1)
        || (info.name == "QMap" && info.args.size() == 2)
        || (info.name == "QSet" && info.args.size() == 1))
    {
        for(int i = 0; i < info.args.size(); i++)
        {
            if(!JZNodeType::isVaildType(info.args[i]))
            {
                error = "no such type " + info.args[i];
                return false;
            }
        }
        return true;
    }   

    error = type + " format error";
    return false;
}

void registContainer(QString type,int type_id)
{
    QString list_pre = "QList<";
    QString map_pre = "QMap<";
    QString set_pre = "QSet<";

    int end_idx = type.lastIndexOf(">");
    if(type.startsWith(list_pre))
    {
        QString value_type = type.mid(list_pre.size(),end_idx - list_pre.size());
        registList(value_type,type_id);
    }
    else if(type.startsWith(map_pre))
    {
        QString type_str = type.mid(map_pre.size(),end_idx - map_pre.size());
        QStringList type_list = type_str.split(",");
        QString key_type = type_list[0];
        QString value_type = type_list[1];
        registMap(key_type,value_type,type_id);
    }
    else if(type.startsWith(set_pre))
    {
        QString type_str = type.mid(set_pre.size(),end_idx - set_pre.size());
        registSet(type_str,type_id);
    }   
}

void unregistContainer(QString type)
{
    auto type_id = JZNodeObjectManager::instance()->getId(type);
    if(type_id != Type_none)
        JZNodeObjectManager::instance()->unregist(type_id);
}