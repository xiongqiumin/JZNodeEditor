#include "JZScriptFile.h"
#include "JZNodeFactory.h"
#include "JZNodeFunction.h"
#include "JZUiFile.h"
#include "JZParamFile.h"
#include "JZProject.h"

//JZScriptFile
JZScriptFile::JZScriptFile(int type)
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

JZScriptFile::~JZScriptFile()
{

}

void JZScriptFile::loadFinish()
{
    auto it = m_nodes.begin();
    while (it != m_nodes.end())
    {
        it->data()->setFile(this);
        it++;
    }
}

void JZScriptFile::clear()
{
    m_nodeId = 0;    
    m_nodes.clear();
    m_connects.clear();
}

const FunctionDefine &JZScriptFile::function()
{
    return m_function;
}

void JZScriptFile::setFunction(FunctionDefine def)
{
    m_name = def.name;
    m_function = def;
}

int JZScriptFile::addNode(JZNodePtr node)
{
    Q_ASSERT(node->id() == -1);
    node->setId(m_nodeId++);
    node->setFile(this);
    m_nodes.insert(node->id(), node);
    return node->id();
}

void JZScriptFile::insertNode(JZNodePtr node)
{
    Q_ASSERT(node->id() != -1 && getNode(node->id()) == nullptr);
    node->setFile(this);
    m_nodes.insert(node->id(), node);
    m_nodesPos.insert(node->id(), QPointF());
}

void JZScriptFile::removeNode(int id)
{
    QList<int> lines = getConnectId(id);
    for (int i = 0; i < lines.size(); i++)
        removeConnect(lines[i]);

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
    return m_nodesPos.value(id,QPointF());
}

QList<int> JZScriptFile::nodeList()
{
    return m_nodes.keys();
}

bool JZScriptFile::hasConnect(JZNodeGemo from, JZNodeGemo to)
{
    for (int i = 0; i < m_connects.size(); i++)
    {
        if (m_connects[i].from == from && m_connects[i].to == to)
            return true;
    }
    return false;
}

int JZScriptFile::parentNode(int id)
{
    JZNode *node = getNode(id);
    if(node->flowIn() == -1)
        return id;
    auto in_lines = getConnectId(id, node->flowIn());
    if (in_lines.size() == 0)
        return -1;

    auto line = getConnect(in_lines[0]);

    JZNode *from = getNode(line->from.nodeId);
    auto pin = from->prop(line->from.propId);
    if (pin->isSubFlow())
        return line->from.nodeId;
    else
        return parentNode(line->from.nodeId);
}

bool JZScriptFile::canConnect(JZNodeGemo from, JZNodeGemo to,QString &error)
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

    auto out_lines = getConnectId(from.nodeId, from.propId);
    auto in_lines = getConnectId(to.nodeId, to.propId);
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
    if(pin_from->isFlow() && in_lines.size() > 0 && (parentNode(from.nodeId) != parentNode(to.nodeId)))
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
        QList<int> from_type = node_from->propType(from.propId);
        if (from_type.size() == 0)
        {
            error = "输出数据未设置";
            return false;
        }

        QList<int> in_type = node_to->propType(to.propId);
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

int JZScriptFile::addConnect(JZNodeGemo from, JZNodeGemo to)
{
    QString error;
    auto pin_from = getPin(from);
    auto pin_to = getPin(to);
    Q_ASSERT(pin_from && pin_to && pin_from->isOutput() && pin_to->isInput());
    Q_ASSERT(canConnect(from,to,error));

    JZNodeConnect connect;
    connect.id = m_nodeId++;
    connect.from = from;
    connect.to = to;
    m_connects.push_back(connect);

    JZNode *node_to = getNode(to.nodeId);
    node_to->pinLinked(to.propId);
    return connect.id;
}

void JZScriptFile::insertConnect(const JZNodeConnect &connect)
{
    Q_ASSERT(connect.id != -1 && getConnect(connect.id) == nullptr);
    m_connects.push_back(connect);

    JZNode *to = getNode(connect.to.nodeId);
    to->pinLinked(connect.to.propId);
}

void JZScriptFile::removeConnect(int id)
{
    for (int i = 0; i < m_connects.size(); i++)
    {
        auto &line = m_connects[i];
        if (line.id == id)
        {            
            JZNode *to = getNode(line.to.nodeId);
            int pin_id = line.to.propId;

            m_connects.removeAt(i);            
            to->pinUnlinked(pin_id);
            return;
        }
    }
}

void JZScriptFile::removeConnectByNode(int node_id, int prop_id)
{
    auto list = getConnectId(node_id, prop_id);
    for (int i = 0; i < list.size(); i++)
        removeConnect(list[i]);
}

QList<int> JZScriptFile::getConnectId(int id, int propId)
{
    QList<int> list;
    for (int i = 0; i < m_connects.size(); i++)
    {
        auto &c = m_connects[i];
        if ((propId == -1 && (c.from.nodeId == id || c.to.nodeId == id))
                || (c.from.nodeId == id && c.from.propId == propId)
                || (c.to.nodeId == id && c.to.propId == propId))
            list.push_back(c.id);
    }
    return list;
}

QList<int> JZScriptFile::getConnectOut(int id, int propId)
{
    QList<int> list;
    for (int i = 0; i < m_connects.size(); i++)
    {
        auto &c = m_connects[i];
        if ((propId == -1 && (c.from.nodeId == id || c.to.nodeId == id))
                || (c.from.nodeId == id && c.from.propId == propId))
            list.push_back(c.id);
    }
    return list;
}

QList<int> JZScriptFile::getConnectInput(int id, int propId)
{
    QList<int> list;
    for (int i = 0; i < m_connects.size(); i++)
    {
        auto &c = m_connects[i];
        if ((propId == -1 && (c.from.nodeId == id || c.to.nodeId == id))
                || (c.to.nodeId == id && c.to.propId == propId))
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

JZParamDefine *JZScriptFile::localVariableInfo(const QString &name)
{
    if (m_itemType == ProjectItem_scriptFunction)
    {
        for (int i = 0; i < m_function.paramIn.size(); i++)
        {
            if (m_function.paramIn[i].name == name)
                return &m_function.paramIn[i];
        }        
    }

    auto it = m_variables.find(name);
    if (it != m_variables.end())
        return &it.value();

    return nullptr;
}

void JZScriptFile::removeLocalVariable(QString name)
{
    m_variables.remove(name);
}

void JZScriptFile::replaceLocalVariableInfo(QString oldName, JZParamDefine def)
{
    Q_ASSERT(m_variables.contains(oldName));    
    m_variables.remove(oldName);
    m_variables[def.name] = def;
}

void JZScriptFile::addLocalVariable(QString name, int type, QVariant v)
{
    JZParamDefine info;
    info.name = name;
    info.dataType = type;
    info.value = v;
    addLocalVariable(info);
}

void JZScriptFile::addLocalVariable(JZParamDefine def)
{
    Q_ASSERT(!localVariableInfo(def.name) && def.dataType != Type_none);
    m_variables[def.name] = def;
}

QStringList JZScriptFile::localVariableList(bool hasFunc)
{
    QStringList list = m_variables.keys();
    if (hasFunc && m_itemType == ProjectItem_scriptFunction) 
    {        
        for (int i = 0; i < m_function.paramIn.size(); i++)
        {
            if (m_function.paramIn[i].name == "this")
                continue;

            list << m_function.paramIn[i].name;
        }            
    }
    return list;
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
    s << m_connects;    
    s << m_function;
    s << m_variables;
    s << m_nodesPos;
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
    s >> m_connects;    
    s >> m_function;    
    s >> m_variables;
    s >> m_nodesPos;
}