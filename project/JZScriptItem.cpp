﻿#include "JZScriptItem.h"
#include "JZNodeFactory.h"
#include "JZNodeEvent.h"
#include "JZNodeFunction.h"
#include "JZUiFile.h"
#include "JZParamItem.h"
#include "JZProject.h"

//JZScriptItem
JZScriptItem::JZScriptItem(int type)
    :JZProjectItem(type)
{
    clear();
    m_pri = 10;

    if(type == ProjectItem_scriptFunction)
    {
        JZNodeFunctionStart *node_start = new JZNodeFunctionStart();
        addNode(node_start);
    }
}

JZScriptItem::~JZScriptItem()
{
    clear();
}

bool JZScriptItem::isFunction() const
{
    return (itemType() == ProjectItem_scriptFunction);
}

void JZScriptItem::loadFinish()
{
    auto it = m_nodes.begin();
    while (it != m_nodes.end())
    {
        it.value()->setFile(this);
        it++;
    }

    if (m_itemType == ProjectItem_scriptFunction)
        m_function.name = m_name;
}

void JZScriptItem::clear()
{    
    qDeleteAll(m_nodes);
    m_nodes.clear();
    m_connects.clear();
    m_nodeId = 0;
}

int JZScriptItem::nextId()
{
    return m_nodeId;
}

const JZFunctionDefine &JZScriptItem::function()
{
    Q_ASSERT(m_itemType == ProjectItem_scriptFunction);
    m_function.name = m_name;
    return m_function;
}

void JZScriptItem::setFunction(JZFunctionDefine def)
{
    m_name = def.name;
    m_function = def;    
}

int JZScriptItem::addNode(JZNode *node)
{
    Q_ASSERT(node->id() == -1);
    node->setId(m_nodeId++);
    node->setFile(this);
    m_nodes.insert(node->id(), node);
    return node->id();
}

void JZScriptItem::insertNode(JZNode * node)
{
    Q_ASSERT(node->id() != -1 && getNode(node->id()) == nullptr);
    node->setFile(this);
    m_nodes.insert(node->id(), node);
    m_nodesPos.insert(node->id(), QPointF());
}

void JZScriptItem::removeNode(int id)
{    
    QList<int> lines = getConnectPin(id);
    for (int i = 0; i < lines.size(); i++)
        removeConnect(lines[i]);

    auto it = m_nodes.find(id);
    if (it != m_nodes.end())
    {
        Q_ASSERT(it.value()->canRemove());
        delete it.value();
        m_nodes.erase(it);
    }
    m_nodesPos.remove(id);
}

int JZScriptItem::addGroup(const JZNodeGroup &group)
{
    Q_ASSERT(group.id == -1);    

    int id = m_nodeId++;
    m_groups.push_back(group);
    m_groups.back().id = id;
    return id;
}

void JZScriptItem::insertGroup(const JZNodeGroup &group)
{
    m_groups.push_back(group);
    std::sort(m_groups.begin(), m_groups.end(), [](const JZNodeGroup &g1, const JZNodeGroup &g2)->bool{
        return g1.id < g2.id;
    });
}

void JZScriptItem::removeGroup(int id)
{
    for (int i = 0; i < m_groups.size(); i++)
    {
        if (m_groups[i].id == id)
        {
            m_groups.removeAt(i);
            return;
        }
    }
}

JZNodeGroup *JZScriptItem::getGroup(int id)
{
    for (int i = 0; i < m_groups.size(); i++)
    {
        if (m_groups[i].id == id)        
            return &m_groups[i];        
    }
    return nullptr;
}

QList<int> JZScriptItem::groupNodeList(int id)
{
    QList<int> list;
    auto it = m_nodes.begin();
    while (it != m_nodes.end())
    {
        if(it.value()->group() == id)
            list << it.value()->id();
        it++;
    }
    return list;
}

QList<JZNodeGroup> JZScriptItem::groupList()
{
    return m_groups;
}

JZNodePin *JZScriptItem::getPin(const JZNodeGemo &gemo)
{
    auto node = getNode(gemo.nodeId);
    if(!node)
        return nullptr;
    return node->pin(gemo.pinId);
}

JZNode *JZScriptItem::getNode(int id)
{
    auto it = m_nodes.find(id);
    if (it != m_nodes.end())
        return it.value();
    else
        return nullptr;
}

void JZScriptItem::setNodePos(int id,QPointF pos)
{
    m_nodesPos[id] = pos;        
}

QPointF JZScriptItem::getNodePos(int id)
{    
    return m_nodesPos.value(id,QPointF());
}

QList<int> JZScriptItem::nodeList()
{
    return m_nodes.keys();
}

bool JZScriptItem::hasConnect(JZNodeGemo from, JZNodeGemo to)
{
    for (int i = 0; i < m_connects.size(); i++)
    {
        if (m_connects[i].from == from && m_connects[i].to == to)
            return true;
    }
    return false;
}

int JZScriptItem::parentNode(int id)
{
    JZNode *node = getNode(id);
    if(node->flowIn() == -1)
        return id;
    auto in_lines = getConnectPin(id, node->flowIn());
    if (in_lines.size() == 0)
        return -1;

    auto line = getConnect(in_lines[0]);

    JZNode *from = getNode(line->from.nodeId);
    auto pin = from->pin(line->from.pinId);
    if (pin->isSubFlow())
        return line->from.nodeId;
    else
        return parentNode(line->from.nodeId);
}

bool JZScriptItem::checkConnectType(JZNodeGemo from, JZNodeGemo to,QString &error)
{
    auto env = project()->environment();
    JZNodePin *pin_from = getPin(from);
    JZNodePin *pin_to = getPin(to);

    //检测数据类型
    if(pin_from->isParam())
    {
        QList<int> from_type = env->nameToTypeList(pin_from->dataType());
        if (from_type.size() == 0)
        {
            error = pin_from->name() + "输出数据未设置类型";
            return false;
        }

        QList<int> in_type = env->nameToTypeList(pin_to->dataType());
        if (in_type.size() == 0) 
        {
            error = "输入数据未设置类型";
            return false;
        }

        bool ok = false;
        for (int i = 0; i < from_type.size(); i++)
        {
            for (int j = 0; j < in_type.size(); j++)
            {
                if (env->canConvert(from_type[i], in_type[j]))
                {
                    ok = true;
                    break;
                }
            }
            if (ok)
                break; 
        }        
        if(!ok)
        {
            QStringList toTypes = pin_from->dataType();
            QStringList formTypes = pin_from->dataType();            
            error = "数据类型不匹配,需要" + toTypes.join(",") + ", 输入为" + formTypes.join(",");
            return false;
        }
    }

    return true;
}

bool JZScriptItem::checkConnectNormal(JZNodeGemo from, JZNodeGemo to,QString &error)
{
    JZNode *node_from = getNode(from.nodeId);
    JZNode *node_to = getNode(to.nodeId);
    Q_ASSERT(node_from && node_to);
    if (node_from == node_to)
    {
        error = "输入输出不能是同一节点";
        return false;
    }
    
    JZNodePin *pin_from = node_from->pin(from.pinId);
    JZNodePin *pin_to = node_to->pin(to.pinId);
    Q_ASSERT(pin_from && pin_to);
    if(!(pin_from->isOutput() && pin_to->isInput())){
        error = "只能连接输入节点";
        return false;
    }
    if(hasConnect(from,to))
    {
        error = "连接已存在";
        return false;
    }
    if((pin_from->isFlow() || pin_from->isSubFlow()) != pin_to->isFlow())
    {
        if(pin_from->isParam())
            error = "数据节点只能连接数据";
        else
            error = "流程节点只能连接流程";
        return false;
    }

    auto out_lines = getConnectPin(from.nodeId, from.pinId);
    auto in_lines = getConnectPin(to.nodeId, to.pinId);
    if((pin_from->isFlow() || pin_from->isSubFlow()) && out_lines.size() != 0)
    {
        error = "已连接其他流程节点";
        return false;
    }
    if(pin_from->isSubFlow() && in_lines.size() != 0)
    {
        error = "子过程只能连接未连接的节点";
        return false;
    }
    if(pin_from->isFlow() && parentNode(to.nodeId) != -1 && (parentNode(from.nodeId) != parentNode(to.nodeId)))
    {        
        error = "两个节点属于不同的流程，无法连接";
        return false;
    }
    if(in_lines.size() > 0)  //输入点只能连接一个计算
    {
        bool cur_flow = node_from->isFlowNode();
        auto other_node = getNode(getConnect(in_lines[0])->from.nodeId);
        if (other_node->isFlowNode() != cur_flow)
        {
            error = "不能同时连接流程输入和数据输入";
            return false;
        }
        else if (!cur_flow)
        {
            error = "已有数据输入,只能连接一个输入";
            return false;
        }
    }
    if(!pin_from->isLiteral() && pin_to->isLiteral())
    {
        error = "输入需为常量";
        return false;
    }
    if (!node_to->canLink(from.nodeId, from.pinId, error))
        return false;

    return true;
}

bool JZScriptItem::canConnect(JZNodeGemo from, JZNodeGemo to,QString &error)
{        
    if(!checkConnectNormal(from,to,error))
        return false;

    //检测数据类型
    if(!checkConnectType(from,to,error))
        return false;
        
    return true;
}

int JZScriptItem::addConnect(JZNodeGemo from, JZNodeGemo to)
{
    int id = addConnectForce(from,to);

    QString error;
    Q_ASSERT_X(checkConnectType(from,to,error),"Error",qUtf8Printable(error));

    return id;
}

int JZScriptItem::addConnectForce(JZNodeGemo from, JZNodeGemo to)
{
    QString error;
    Q_ASSERT_X(checkConnectNormal(from,to,error),"Error",qUtf8Printable(error));

    JZNodeConnect connect;
    connect.id = m_nodeId++;
    connect.from = from;
    connect.to = to;
    m_connects.push_back(connect);

    JZNode *node_to = getNode(to.nodeId);
    node_to->onPinLinked(to.pinId);
    return connect.id;
}

void JZScriptItem::insertConnect(const JZNodeConnect &connect)
{
    Q_ASSERT(connect.id != -1 && getConnect(connect.id) == nullptr);
    m_connects.push_back(connect);
    std::sort(m_connects.begin(), m_connects.end(), [](const JZNodeConnect &l1, const JZNodeConnect &l2)->bool {
        return l1.id < l2.id;
    });

    JZNode *to = getNode(connect.to.nodeId);
    to->onPinLinked(connect.to.pinId);
}

void JZScriptItem::removeConnect(int id)
{
    for (int i = 0; i < m_connects.size(); i++)
    {
        auto &line = m_connects[i];
        if (line.id == id)
        {            
            JZNode *to = getNode(line.to.nodeId);
            int pin_id = line.to.pinId;

            m_connects.removeAt(i);            
            to->onPinUnlinked(pin_id);
            return;
        }
    }
}

void JZScriptItem::removeConnectByNode(int node_id, int prop_id)
{
    auto list = getConnectPin(node_id, prop_id);
    for (int i = 0; i < list.size(); i++)
        removeConnect(list[i]);
}

QList<int> JZScriptItem::getConnectPin(int id, int pinId)
{
    QList<int> list;
    for (int i = 0; i < m_connects.size(); i++)
    {
        auto &c = m_connects[i];
        if ((pinId == -1 && (c.from.nodeId == id || c.to.nodeId == id))
                || (c.from.nodeId == id && c.from.pinId == pinId)
                || (c.to.nodeId == id && c.to.pinId == pinId))
            list.push_back(c.id);
    }
    return list;
}

QList<int> JZScriptItem::getConnectOut(int id, int pinId)
{
    QList<int> list;
    for (int i = 0; i < m_connects.size(); i++)
    {
        auto &c = m_connects[i];
        if ((pinId == -1 && (c.from.nodeId == id || c.to.nodeId == id))
                || (c.from.nodeId == id && c.from.pinId == pinId))
            list.push_back(c.id);
    }
    return list;
}

QList<int> JZScriptItem::getConnectInput(int id, int pinId)
{
    QList<int> list;
    for (int i = 0; i < m_connects.size(); i++)
    {
        auto &c = m_connects[i];
        if ((pinId == -1 && (c.from.nodeId == id || c.to.nodeId == id))
                || (c.to.nodeId == id && c.to.pinId == pinId))
            list.push_back(c.id);
    }
    return list;
}

JZNodeConnect *JZScriptItem::getConnect(int id)
{
    for (int i = 0; i < m_connects.size(); i++)
    {
        if (m_connects[i].id == id)
            return &m_connects[i];
    }
    return nullptr;
}

QList<JZNodeConnect> JZScriptItem::connectList()
{
    return m_connects;
}

void JZScriptItem::addLocalVariable(const JZParamDefine &def)
{
    m_variables[def.name] = def;
}

void JZScriptItem::addLocalVariable(const QString &name,int dataType,const QString &value)
{
    QString type = project()->environment()->typeToName(dataType);
    addLocalVariable(JZParamDefine(name, type,value));
}

void JZScriptItem::removeLocalVariable(QString name)
{
    m_variables.remove(name);
}

void JZScriptItem::setLocalVariable(QString name, const JZParamDefine &def)
{
    m_variables.remove(name);
    m_variables[def.name] = def;
}

const JZParamDefine *JZScriptItem::localVariable(QString name)
{
    auto it = m_variables.find(name);
    if (it != m_variables.end())
        return &it.value();

    if (m_itemType == ProjectItem_scriptFunction)
    {
        for (int i = 0; i < m_function.paramIn.size(); i++)
        {
            if (m_function.paramIn[i].name == name)
                return &m_function.paramIn[i];
        }
    }

    return nullptr;
}

QStringList JZScriptItem::localVariableList(bool hasFunc)
{
    QStringList list = m_variables.keys();
    if (hasFunc && m_itemType == ProjectItem_scriptFunction) 
    {        
        for (int i = 0; i < m_function.paramIn.size(); i++)
        {
            if (i == 0 && m_function.isMemberFunction())
                continue;

            list << m_function.paramIn[i].name;
        }            
    }
    return list;
}

void JZScriptItem::saveEditorCache()
{
    m_editorCache.clear();
    QDataStream s(&m_editorCache,QIODevice::WriteOnly);

    s << m_nodeId;    
    QList<QByteArray> node_list;
    auto it = m_nodes.begin();
    while (it != m_nodes.end())
    {        
        node_list << it.value()->toBuffer();
        it++;
    }
    s << node_list;    
    s << m_connects;        
    s << m_variables;
    s << m_nodesPos;
    s << m_groups;
}

void JZScriptItem::loadEditorCache()
{
    QDataStream s(&m_editorCache,QIODevice::ReadOnly); 

    s >> m_nodeId;    
    QList<QByteArray> node_list;
    s >> node_list;
    for (int i = 0; i < node_list.size(); i++)
    {
        QByteArray node_buffer = node_list[i];
        QDataStream node_s(node_buffer);
        int node_type;
        node_s >> node_type;
                
        JZNode *node = JZNodeFactory::instance()->createNode(node_type);
        node->fromBuffer(node_buffer);
        m_nodes.insert(node->id(), node);
    }    
    s >> m_connects;    
    s >> m_variables;
    s >> m_nodesPos;
    s >> m_groups;
}

void JZScriptItem::saveToStream(QDataStream &s) const
{    
    s << m_name;
    s << m_function;
    s << m_editorCache;    
}

bool JZScriptItem::loadFromStream(QDataStream &s)
{    
    s >> m_name;
    s >> m_function;
    s >> m_editorCache;        
    loadEditorCache(); 
    return true;
}