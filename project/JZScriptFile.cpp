#include "JZScriptFile.h"
#include "JZNodeFactory.h"

//JZScriptFile
JZScriptFile::JZScriptFile(int type,bool dir)
    :JZProjectItem(type,dir)
{
    clear();
}

JZScriptFile::~JZScriptFile()
{

}

void JZScriptFile::clear()
{
    m_nodeId = 0;
    m_nodes.clear();
    m_connects.clear();
}

int JZScriptFile::addNode(JZNodePtr node)
{
    Q_ASSERT(node->id() == -1);
    node->setId(m_nodeId++);        
    m_nodes.insert(node->id(), node);
    return node->id();
}

void JZScriptFile::insertNode(JZNodePtr node)
{
    Q_ASSERT(node->id() != -1 && getNode(node->id()) == nullptr);
    m_nodes.insert(node->id(), node);
    m_nodesPos.insert(node->id(), QPointF());
}

void JZScriptFile::removeNode(int id)
{
    m_nodes.remove(id);
    m_nodesPos.remove(id);
}

JZNodePin *JZScriptFile::getPin(const JZNodeGemo &gemo)
{
    auto node = getNode(gemo.nodeId);
    if(!node)
        return nullptr;
    return node->prop(gemo.propId);
}

JZNode *JZScriptFile::getNode(int id)
{
    auto it = m_nodes.find(id);
    if (it != m_nodes.end())
        return it->data();
    else
        return nullptr;
}

void JZScriptFile::setNodePos(int id,QPointF pos)
{
    m_nodesPos[id] = pos;        
}

QPointF JZScriptFile::getNodePos(int id)
{
    auto it = m_nodesPos.find(id);
    if(it != m_nodesPos.end())
        return it.value();
    return QPointF();
}

QList<int> JZScriptFile::nodeList()
{
    return m_nodes.keys();
}

int JZScriptFile::addConnect(JZNodeGemo from, JZNodeGemo to)
{
    auto pin_from = getPin(from);
    auto pin_to = getPin(to);
    Q_ASSERT(pin_from && pin_to && pin_from->isOutput() && pin_to->isInput());
    Q_ASSERT(((pin_from->isFlow() || pin_from->isSubFlow()) && pin_to->isFlow()) 
        || (pin_from->isParam() && pin_to->isParam()));

    JZNodeConnect connect;
    connect.id = m_nodeId++;
    connect.from = from;
    connect.to = to;
    m_connects.push_back(connect);
    return connect.id;
}

bool JZScriptFile::canConnect(JZNodeGemo from, JZNodeGemo to)
{
    JZNodePin *pin_from = getPin(from);
    JZNodePin *pin_to = getPin(to);
    if(!JZNodeType::canConvert(pin_from->dataType(),pin_to->dataType()))
        return false;

    return true;
}

void JZScriptFile::insertConnect(const JZNodeConnect &connect)
{
    Q_ASSERT(connect.id != -1 && getConnect(connect.id) == nullptr);
    m_connects.push_back(connect);
}

void JZScriptFile::removeConnect(int id)
{
    for (int i = 0; i < m_connects.size(); i++)
    {
        if (m_connects[i].id == id)
        {
            m_connects.removeAt(i);
            return;
        }
    }
}

void JZScriptFile::removeConnectByNode(int node_id, int prop_id, int type)
{
    auto list = getConnectId(node_id, prop_id, type);
    for (int i = 0; i < list.size(); i++)
        removeConnect(list[i]);
}

QList<int> JZScriptFile::getConnectId(int id, int propId, int flag)
{
    QList<int> list;
    for (int i = 0; i < m_connects.size(); i++)
    {
        auto &c = m_connects[i];
        if ((propId == -1 && (c.from.nodeId == id || c.to.nodeId == id)) || (c.from.nodeId == id && c.from.propId == propId && (flag & Prop_out)) || (c.to.nodeId == id && c.to.propId == propId && (flag & Prop_in)))
            list.push_back(c.id);
    }
    return list;
}

JZNodeConnect *JZScriptFile::getConnect(int id)
{
    for (int i = 0; i < m_connects.size(); i++)
    {
        if (m_connects[i].id == id)
            return &m_connects[i];
    }
    return nullptr;
}

QList<JZNodeConnect> JZScriptFile::connectList()
{
    return m_connects;
}

void JZScriptFile::saveToStream(QDataStream &s)
{
    JZProjectItem::saveToStream(s);

    s << m_nodeId;
    int size = m_nodes.size();
    s << size;
    auto it = m_nodes.begin();
    while (it != m_nodes.end())
    {
        s << it->data()->type();
        it->data()->saveToStream(s);
        it++;
    }
    s << m_nodesPos;
    s << m_connects;    
}

void JZScriptFile::loadFromStream(QDataStream &s)
{
    JZProjectItem::loadFromStream(s);

    s >> m_nodeId;
    int size = 0;
    s >> size;
    for (int i = 0; i < size; i++)
    {
        int type;
        s >> type;
        JZNode *node = JZNodeFactory::instance()->createNode(type);
        node->loadFromStream(s);
        m_nodes.insert(node->id(), JZNodePtr(node));        
    }
    s >> m_nodesPos;
    s >> m_connects;
}
