#include "JZScriptFile.h"
#include "JZNodeFactory.h"
#include "JZNodeFunction.h"
#include "JZNodeFunctionManager.h"
#include "JZUiFile.h"
#include "JZParamFile.h"

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

void JZScriptFile::clear()
{
    m_nodeId = 0;
    m_bindClass.clear();
    m_nodes.clear();
    m_connects.clear();
}

void JZScriptFile::setBindClass(QString bindClass)
{
    m_bindClass = bindClass;        
}

QString JZScriptFile::bindClass()
{
    return m_bindClass;
}

const FunctionDefine &JZScriptFile::function()
{
    return m_function;
}

void JZScriptFile::setFunction(FunctionDefine def)
{
    m_function = def;
    m_function.script = itemPath();
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

bool JZScriptFile::canConnect(JZNodeGemo from, JZNodeGemo to,QString &error)
{    
    JZNode *node_from = getNode(from.nodeId);
    JZNode *node_to = getNode(to.nodeId);
    if(node_from == node_to)
        return false;

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
        if(pin_from->isFlow())
            error = "流程节点只能连接流程";
        else
            error = "数据节点只能连接数据";
        return false;
    }

    auto out_lines = getConnectId(from.nodeId, from.propId);
    auto in_lines = getConnectId(to.nodeId, to.propId);
    if((pin_from->isFlow() || pin_from->isSubFlow()) && out_lines.size() != 0)
    {
        error = "已有流程节点连接";
        return false;
    }
    if(pin_from->isSubFlow() && in_lines.size() != 0)
    {
        error = "子过程只能连接未连接的节点";
        return false;
    }
    if(!node_to->isFlowNode() && in_lines.size() > 0)  //输入点只能连接一个计算
    {
        error = "已有输入,只能连接一个输入";
        return false;
    }

    //检测数据类型
    if(!pin_from->isFlow())
    {
        QList<int> form_type = node_from->propType(from.propId);
        QList<int> in_type = node_to->propType(to.propId);
        bool ok = JZNodeType::canConvert(form_type,in_type);
        if(!ok)
        {
            QStringList inTypes;
            for(int i = 0; i < in_type.size(); i++)
                inTypes << JZNodeType::idToName(in_type[i]);

            error = "数据类型不匹配,需要" + inTypes.join(",");
            return false;
        }
    }

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
    s << m_function;
    s << m_bindClass;
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
    s >> m_function;
    s >> m_bindClass;
}

//JZScriptLibraryFile
JZScriptLibraryFile::JZScriptLibraryFile()
    :JZProjectItem(ProjectItem_library)
{
}
    
JZScriptLibraryFile::~JZScriptLibraryFile()
{

}

void JZScriptLibraryFile::addFunction(QString name,QStringList in,QStringList out)
{
    FunctionDefine define;
    define.name = name;
    for(int i = 0; i < in.size(); i++)
    {
        FunctionParam param;
        param.name = in[i];
        define.paramIn.push_back(param);
    }
    for(int i = 0; i < out.size(); i++)
    {
        FunctionParam param;
        param.name = out[i];
        define.paramOut.push_back(param);
    }

    JZScriptFile *file = new JZScriptFile(ProjectItem_scriptFunction);
    file->setName(name);
    file->setFunction(define);
    addItem(JZProjectItemPtr(file));
}

//JZScriptClassFile
JZScriptClassFile::JZScriptClassFile()
    :JZProjectItem(ProjectItem_class)
{
    m_classId = -1;
}

JZScriptClassFile::~JZScriptClassFile()
{

}

void JZScriptClassFile::saveToStream(QDataStream &s)
{
    JZProjectItem::saveToStream(s);
    s << m_className;
    s << m_super;
    s << m_classId;
}

void JZScriptClassFile::loadFromStream(QDataStream &s)
{
    JZProjectItem::loadFromStream(s);
    s >> m_className;
    s >> m_super;
    s >> m_classId;
}

void JZScriptClassFile::init(QString className,QString super)
{
    m_className = className;
    m_super = super;
    m_classId = JZNodeObjectManager::instance()->regist(objectDefine());
}

void JZScriptClassFile::unint()
{
    JZNodeObjectManager::instance()->unregist(m_classId);
}

void JZScriptClassFile::reinit()
{
    JZNodeObjectManager::instance()->replace(objectDefine());
}

bool JZScriptClassFile::addMemberVariable(QString name,int dataType,const QVariant &v)
{    
    getParamFile()->addVariable(name,dataType,v);
    JZNodeObjectManager::instance()->replace(objectDefine());
    return true;
}

void JZScriptClassFile::removeMemberVariable(QString name)
{
    getParamFile()->removeVariable(name);
    JZNodeObjectManager::instance()->replace(objectDefine());
}

bool JZScriptClassFile::addMemberFunction(FunctionDefine func)
{
    JZNodeObjectManager::instance()->replace(objectDefine());
    return true;
}

void JZScriptClassFile::removeMemberFunction(QString func)
{
    JZNodeObjectManager::instance()->replace(objectDefine());
}

JZParamFile *JZScriptClassFile::getParamFile()
{
    for(int i = 0; i < m_childs.size(); i++)
    {
        auto item = m_childs[i].data();
        if(item->itemType() == ProjectItem_param)
            return dynamic_cast<JZParamFile*>(item);
    }
    Q_ASSERT(0);
    return nullptr;
}

JZNodeObjectDefine JZScriptClassFile::objectDefine()
{    
    JZNodeObjectDefine define;
    define.className = m_className;
    define.superName = m_super;
    define.id = m_classId;
    for(int i = 0; i < m_childs.size(); i++)
    {
        auto item = m_childs[i].data();
        if(item->itemType() == ProjectItem_param)
        {
            auto param_item = dynamic_cast<JZParamFile*>(item);
            define.params = param_item->variables();
        }
        else if(item->itemType() == ProjectItem_scriptFunction)
        {
            auto function_item = dynamic_cast<JZScriptFile*>(item);
            define.addFunction(function_item->function());
        }
        else if(item->itemType() == ProjectItem_ui)
        {
            auto ui_item = dynamic_cast<JZUiFile*>(item);
            ui_item->getWidgetMembers(define.params);
        }
    }
    return define;
}
