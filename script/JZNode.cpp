#include "JZNode.h"
#include "JZNodeCompiler.h"

// JZNodeGemo
JZNodeGemo::JZNodeGemo()
{
    nodeId = -1;
    propId = -1;
}

JZNodeGemo::JZNodeGemo(int id, int prop)
{
    nodeId = id;
    propId = prop;
}

bool JZNodeGemo::isNull() const
{
    return (nodeId == -1);
}

bool JZNodeGemo::operator==(const JZNodeGemo &other) const
{
    return nodeId == other.nodeId && propId == other.propId;
}

// JZNodeConnect
JZNodeConnect::JZNodeConnect()
{
    id = -1;
}

QDataStream &operator<<(QDataStream &s, const JZNodeConnect &param)
{
    s << param.id;
    s << param.from.nodeId;
    s << param.from.propId;
    s << param.to.nodeId;
    s << param.to.propId;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeConnect &param)
{
    s >> param.id;
    s >> param.from.nodeId;
    s >> param.from.propId;
    s >> param.to.nodeId;
    s >> param.to.propId;
    return s;
}

JZNodeConnect parseLine(const QByteArray &buffer)
{
    JZNodeConnect line;
    QDataStream s(buffer);
    s >> line;
    return line;
}

QByteArray formatLine(const JZNodeConnect &line)
{
    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << line;
    return data;
}

// JZNode
JZNode::JZNode()
{
    m_id = INVALID_ID;
    m_type = Node_none;
}

JZNode::~JZNode()
{
}

int JZNode::type()
{
    return m_type;
}

int JZNode::addFlow(const JZNodePin &prop)
{
    Q_ASSERT(prop.flag() & Prop_flow);
    return addPropInternal(prop);
}

int JZNode::addProp(const JZNodePin &prop)
{
    Q_ASSERT(prop.flag() & Prop_param);
    return addPropInternal(prop);
}

int JZNode::addPropInternal(const JZNodePin &prop)
{
    int max_id = 0;
    for (int i = 0; i < m_propList.size(); i++)
        max_id = qMax(max_id, m_propList[i].id() + 1);

    JZNodePin new_prop = prop;
    new_prop.setId(max_id);
    m_propList.push_back(new_prop);
    return max_id;
}

void JZNode::removeProp(int id)
{
    int index = indexOfProp(id);
    if (index == -1)
        return;

    m_propList.removeAt(id);
}

JZNodePin *JZNode::prop(int id)
{
    int index = indexOfProp(id);
    if (index >= 0)
        return &m_propList[index];
    else
        return nullptr;
}

int JZNode::indexOfProp(int id) const
{
    for (int i = 0; i < m_propList.size(); i++)
    {
        if (m_propList[i].id() == id)
            return i;
    }
    return -1;
}

int JZNode::indexOfPropByType(int id, int flag) const
{
    int type_index = 0;
    for (int i = 0; i < m_propList.size(); i++)
    {
        if (m_propList[i].id() == id)
            return type_index;
        if (m_propList[i].flag() & flag)
            type_index++;
    }
    return -1;
}

QVector<int> JZNode::propInList(int flag) const
{
    return propListByType(Prop_in | flag);
}

QVector<int> JZNode::propOutList(int flag) const
{
    return propListByType(Prop_out | flag);
}

QVector<int> JZNode::propListByType(int flag) const
{
    QVector<int> ret;
    for (int i = 0; i < m_propList.size(); i++)
    {
        if (m_propList[i].flag() & flag)
            ret.push_back(m_propList[i].id());
    }
    return ret;
}

int JZNode::propCount(int flag)
{
    int count = 0;
    for (int i = 0; i < m_propList.size(); i++)
    {
        if (m_propList[i].flag() & flag)
            count++;
    }
    return count;
}

const QList<JZNodePin> &JZNode::propList() const
{
    return m_propList;
}

QString JZNode::name()
{
    return m_name;
}

void JZNode::setName(QString name)
{
    m_name = name;
}

int JZNode::id()
{
    return m_id;
}

void JZNode::setId(int id)
{
    m_id = id;
}

void JZNode::expandNode()
{
    
}

void JZNode::saveToStream(QDataStream &s) const
{
    s << m_id;
    s << m_name;
    s << m_propList;
}

void JZNode::loadFromStream(QDataStream &s)
{
    s >> m_id;
    s >> m_name;
    s >> m_propList;
}

//JZNodeInput
JZNodeInput::JZNodeInput()
{
    JZNodePin in;
    in.setName("in");
    in.setFlag(Prop_in | Prop_param);    
    m_in = addProp(in);
}

bool JZNodeInput::compiler(JZNodeCompiler *compiler,QString &error)
{       
    int param = compiler->paramId(m_id,m_in);
    compiler->copyVariable(Reg_Call + m_paramIdx,param);
    return true;
}

//JZNodeOutput
JZNodeOutput::JZNodeOutput()
{
    JZNodePin out;
    out.setName("out");
    out.setFlag(Prop_out | Prop_param);    
    m_out = addProp(out);
}

bool JZNodeOutput::compiler(JZNodeCompiler *compiler,QString &error)
{           
    int param = compiler->paramId(m_id,m_out);
    compiler->copyVariable(param,Reg_Call + m_paramIdx);
    return true;
}

//JZNodeReturn
JZNodeReturn::JZNodeReturn()
{
    
}

bool JZNodeReturn::compiler(JZNodeCompiler *compiler,QString &error)
{   
    JZNodeIR ir(OP_return);    
    return true;
}

//JZNodeExit
JZNodeExit::JZNodeExit()
{

}

bool JZNodeExit::compiler(JZNodeCompiler *compiler,QString &error)
{
    JZNodeIR ir(OP_exit);    
    return true;
}

// JZNodeParallel
JZNodeParallel::JZNodeParallel()
{
}

// JZNodeParallelEnd
JZNodeParallelEnd::JZNodeParallelEnd()
{

}

//JZNodeIf
JZNodeIf::JZNodeIf()
{

}

// JZNodeFunction
JZNodeFunction::JZNodeFunction()
{
    m_type = Node_function;
}

bool JZNodeFunction::compiler(JZNodeCompiler *compiler,QString &error)
{
    QVector<int> in_list = propInList(Prop_param);
    QVector<int> out_list = propInList(Prop_param);
    for(int i = 0; i < in_list.size(); i++)
    {
        int id = compiler->paramId(m_id,in_list[i]);
        compiler->copyVariable(Reg_Call+i,id);
    }

    JZNodeIR call(OP_call);
    call.params << functionName;
    compiler->addStatement(call);    

    for(int i = 0; i < out_list.size(); i++)
    {
        int id = compiler->paramId(m_id,out_list[i]);
        compiler->copyVariable(id,Reg_Call+i);
    }

    return true;
}

void JZNodeFunction::saveToStream(QDataStream &s) const
{
    JZNode::saveToStream(s);
    s << functionName;    
}

void JZNodeFunction::loadFromStream(QDataStream &s)
{
    JZNode::loadFromStream(s);
    s >> functionName;    
}