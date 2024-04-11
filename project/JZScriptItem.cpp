#include "JZScriptItem.h"
#include "JZNodeFactory.h"
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
        addNode(JZNodePtr(node_start));
    }
}

JZScriptItem::~JZScriptItem()
{

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
        it->data()->setFile(this);
        it++;
    }

    if (m_itemType == ProjectItem_scriptFunction)
        m_function.name = m_name;
}

void JZScriptItem::clear()
{
    m_nodeId = 0;    
    m_nodes.clear();
    m_connects.clear();
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

int JZScriptItem::addNode(JZNodePtr node)
{
    Q_ASSERT(node->id() == -1);
    node->setId(m_nodeId++);
    node->setFile(this);
    m_nodes.insert(node->id(), node);
    return node->id();
}

void JZScriptItem::insertNode(JZNodePtr node)
{
    Q_ASSERT(node->id() != -1 && getNode(node->id()) == nullptr);
    node->setFile(this);
    m_nodes.insert(node->id(), node);
    m_nodesPos.insert(node->id(), QPointF());
}

void JZScriptItem::removeNode(int id)
{
    QList<int> lines = getConnectId(id);
    for (int i = 0; i < lines.size(); i++)
        removeConnect(lines[i]);

    m_nodes.remove(id);
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
        if(it->data()->group() == id)
            list << it->data()->id();
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
        return it->data();
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
    auto in_lines = getConnectId(id, node->flowIn());
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

bool JZScriptItem::canConnect(JZNodeGemo from, JZNodeGemo to,QString &error)
{        
    JZNode *node_from = getNode(from.nodeId);
    JZNode *node_to = getNode(to.nodeId);
    if (node_from == node_to)
    {
        error = "输入输出不能是同一节点";
        return false;
    }
    
    JZNodePin *pin_from = getPin(from);
    JZNodePin *pin_to = getPin(to);
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

    auto out_lines = getConnectId(from.nodeId, from.pinId);
    auto in_lines = getConnectId(to.nodeId, to.pinId);
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
    if(pin_to->isLiteral() /*&& node_from->type() != Node_literal*/)
    {
        error = "常量,无法连接数据";
        return false;
    }
    //检测数据类型
    if(pin_from->isParam())
    {
        QList<int> from_type = node_from->pinType(from.pinId);
        if (from_type.size() == 0)
        {
            error = "输出数据未设置";
            return false;
        }

        QList<int> in_type = node_to->pinType(to.pinId);
        if (in_type.size() == 0) 
        {
            error = "输入数据未设置";
            return false;
        }

        bool ok = false;
        for (int i = 0; i < from_type.size(); i++)
        {
            for (int j = 0; j < in_type.size(); j++)
            {
                if (JZNodeType::canConvert(from_type[i], in_type[j]))
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
            QStringList inTypes;
            for(int i = 0; i < in_type.size(); i++)
                inTypes << JZNodeType::typeToName(in_type[i]);

            error = "数据类型不匹配,需要" + inTypes.join(",");
            return false;
        }
    }

    return true;
}

int JZScriptItem::addConnect(JZNodeGemo from, JZNodeGemo to)
{
    QString error;
    auto pin_from = getPin(from);
    auto pin_to = getPin(to);
    Q_ASSERT(pin_from && pin_to);
    Q_ASSERT(canConnect(from,to,error));

    JZNodeConnect connect;
    connect.id = m_nodeId++;
    connect.from = from;
    connect.to = to;
    m_connects.push_back(connect);

    JZNode *node_to = getNode(to.nodeId);
    node_to->pinLinked(to.pinId);
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
    to->pinLinked(connect.to.pinId);
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
            to->pinUnlinked(pin_id);
            return;
        }
    }
}

void JZScriptItem::removeConnectByNode(int node_id, int prop_id)
{
    auto list = getConnectId(node_id, prop_id);
    for (int i = 0; i < list.size(); i++)
        removeConnect(list[i]);
}

QList<int> JZScriptItem::getConnectId(int id, int pinId)
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

QByteArray JZScriptItem::toBuffer()
{
    QByteArray buffer;
    QDataStream s(&buffer,QIODevice::WriteOnly);
    
    s << m_name;
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
    s << m_function;
    s << m_variables;
    s << m_nodesPos;
    s << m_groups;

    return buffer;
}

bool JZScriptItem::fromBuffer(const QByteArray &buffer)
{
    int node_type;
    QDataStream s(buffer);    

    s >> m_name;
    s >> m_nodeId;
    
    QList<QByteArray> node_list;
    s >> node_list;
    for (int i = 0; i < node_list.size(); i++)
    {
        QByteArray buffer = node_list[i];
        QDataStream node_s(buffer);
        int node_type;
        node_s >> node_type;
                
        JZNode *node = JZNodeFactory::instance()->createNode(node_type);
        node->fromBuffer(buffer);
        m_nodes.insert(node->id(), JZNodePtr(node));
    }    
    s >> m_connects;
    s >> m_function;
    s >> m_variables;
    s >> m_nodesPos;
    s >> m_groups;
    return true;
}