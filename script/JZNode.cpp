#include <QComboBox>
#include <QPushButton>
#include "JZNode.h"
#include "JZNodeCompiler.h"
#include "JZNodeFunctionManager.h"

//JZNodeGemo
int JZNodeGemo::paramId(int nodeId, int pinId)
{
    Q_ASSERT(pinId >= 0 && pinId < 100 && nodeId >= 0);
    return nodeId * 100 + pinId;
}

JZNodeGemo JZNodeGemo::fromParamId(int param_id)
{
    int node_id = param_id / 100;
    int pin_id = param_id % 100;
    return JZNodeGemo(node_id, pin_id);
}

JZNodeGemo::JZNodeGemo()
{
    nodeId = -1;
    pinId = -1;
}

JZNodeGemo::JZNodeGemo(int node_id, int pin_id)
{
    Q_ASSERT(pin_id >= 0 && pin_id < 100 && node_id >= 0);

    nodeId = node_id;
    pinId = pin_id;
}

int JZNodeGemo::paramId() const
{
    return paramId(nodeId, pinId);
}

bool JZNodeGemo::isNull() const
{
    return (nodeId == -1);
}

bool JZNodeGemo::operator==(const JZNodeGemo &other) const
{
    return nodeId == other.nodeId && pinId == other.pinId;
}

// JZNodeConnect
JZNodeConnect::JZNodeConnect()
{
    id = -1;
}

void operator<<(QDataStream &s, const JZNodeConnect &param)
{
    s << param.id;
    s << param.from.nodeId;
    s << param.from.pinId;
    s << param.to.nodeId;
    s << param.to.pinId;
}

void operator>>(QDataStream &s, JZNodeConnect &param)
{
    s >> param.id;
    s >> param.from.nodeId;
    s >> param.from.pinId;
    s >> param.to.nodeId;
    s >> param.to.pinId;
}

//JZNodeGroup
JZNodeGroup::JZNodeGroup()
{
    id = -1;
}

void operator<<(QDataStream &s, const JZNodeGroup &param)
{
    s << param.id;
    s << param.memo;
}

void operator>>(QDataStream &s, JZNodeGroup &param)
{
    s >> param.id;
    s >> param.memo;
}

// JZNode
JZNode::JZNode()
{
    m_id = INVALID_ID;
    m_file = nullptr;
    m_flag = NodeProp_none;
    m_type = Node_none;
    m_group = -1;
}

JZNode::~JZNode()
{
}

QByteArray JZNode::toBuffer()
{
    Q_ASSERT(m_type != Node_none);

    QByteArray buffer;
    QDataStream s(&buffer, QIODevice::WriteOnly);
    saveToStream(s);
    return buffer;
}

bool JZNode::fromBuffer(const QByteArray &buffer)
{
    QDataStream s(buffer);    
    loadFromStream(s);
    return true;
}

int JZNode::type() const
{
    return m_type;
}

QPointF JZNode::pos() const
{
    return m_pos;        
}

void JZNode::setPos(QPointF pos)
{
    m_pos = pos;
}

void JZNode::setFlag(int flag)
{
    m_flag = flag;
}

int JZNode::flag() const
{
    return m_flag;
}

void JZNode::setGroup(int group)
{
    m_group = group;
}

int JZNode::group()
{
    return m_group;
}

const QString &JZNode::memo() const
{
    return m_memo;
}

void JZNode::setMemo(const QString &text)
{
    m_memo = text;
}

bool JZNode::isFlowNode() const
{
    return pinCount(Pin_flow) > 0;
}

bool JZNode::isParamNode() const
{
    return pinCount(Pin_flow) == 0;
}

int JZNode::addPin(const JZNodePin &pin)
{
    Q_ASSERT(pin.isInput() || pin.isOutput());
    Q_ASSERT(pin.isFlow() || pin.isParam() || pin.isSubFlow());

    auto list = pinList();
    
    int max_id = 0;
    for (int i = 0; i < list.size(); i++)
    {
        max_id = qMax(max_id, list[i] + 1);
    }

    JZNodePin new_prop = pin;
    new_prop.setId(max_id);
    m_pinList.push_back(new_prop);
    return max_id;
}

void JZNode::clearPin()
{
    m_pinList.clear();
}

int JZNode::addParamIn(QString name,int extFlag)
{
    JZNodePin pin;
    pin.setName(name);
    pin.setFlag(Pin_in | Pin_param | extFlag);
    return addPin(pin);        
}

int JZNode::addParamOut(QString name,int extFlag)
{
    JZNodePin pin;
    pin.setName(name);
    pin.setFlag(Pin_out | Pin_param | extFlag);
    return addPin(pin);
}

int JZNode::addFlowIn(int extFlag)
{
    JZNodePin pin;
    pin.setFlag(Pin_in | Pin_flow | extFlag);
    return addPin(pin);
}

int JZNode::addFlowOut(QString name,int extFlag)
{
    Q_ASSERT(flowOutList().size() == 0);

    JZNodePin pin;
    pin.setName(name);
    pin.setFlag(Pin_out | Pin_flow | extFlag);
    return addPin(pin);
}

int JZNode::addSubFlowOut(QString name,int extFlag)
{
    JZNodePin pin;
    pin.setName(name);
    pin.setFlag(Pin_out | Pin_subFlow | extFlag);
    return addPin(pin);
}

void JZNode::removePin(int id)
{
    int index = indexOfPin(id);
    if (index == -1)
        return;

    m_pinList.removeAt(index);
}

JZNodePin *JZNode::pin(int id)
{
    int index = indexOfPin(id);
    if (index >= 0)
        return &m_pinList[index];
    else
        return nullptr;
}

const JZNodePin *JZNode::pin(int id) const
{
    int index = indexOfPin(id);
    if (index >= 0)
        return &m_pinList[index];
    else
        return nullptr;
}

JZNodePin* JZNode::pinByIndex(int index)
{
    return &m_pinList[index];
}

const JZNodePin* JZNode::pinByIndex(int index) const
{
    return &m_pinList[index];
}

JZNodePin *JZNode::pin(QString name)
{
    int index = indexOfPinByName(name);
    if (index >= 0)
        return &m_pinList[index];
    else
        return nullptr;
}

bool JZNode::hasPin(int id) const
{
    return indexOfPin(id) >= 0;
}

int JZNode::indexOfPin(int id) const
{
    for (int i = 0; i < m_pinList.size(); i++)
    {
        if (m_pinList[i].id() == id)
            return i;
    }
    return -1;
}

int JZNode::indexOfPinByName(QString name) const
{
    for (int i = 0; i < m_pinList.size(); i++)
    {
        if (m_pinList[i].name() == name)
            return i;
    }
    return -1;
}

int JZNode::indexOfPinByType(int id, int flag) const
{
    int type_index = 0;
    for (int i = 0; i < m_pinList.size(); i++)
    {
        if (m_pinList[i].id() == id)
            return type_index;
        if (m_pinList[i].flag() & flag)
            type_index++;
    }
    return -1;
}

QList<int> JZNode::pinInList(int flag) const
{
    return pinListByType(Pin_in | flag);
}

QList<int> JZNode::pinOutList(int flag) const
{
    return pinListByType(Pin_out | flag);
}

QList<int> JZNode::pinListByType(int flag) const
{
    QList<int> ret;
    for (int i = 0; i < m_pinList.size(); i++)
    {
        if((m_pinList[i].flag() & flag) == flag)
            ret.push_back(m_pinList[i].id());
    }
    return ret;
}

int JZNode::pinCount() const
{
    return m_pinList.size();
}

int JZNode::pinCount(int flag) const
{
    int count = 0;
    for (int i = 0; i < m_pinList.size(); i++)
    {
        if ((m_pinList[i].flag() & flag) == flag)
            count++;
    }
    return count;
}

int JZNode::paramIn(int index) const
{
    auto list = pinInList(Pin_param);
    if(index < list.size())
        return list[index];
    else
        return -1;
}

JZNodeGemo JZNode::paramInGemo(int index) const
{
    return JZNodeGemo(m_id,paramIn(index));
}

int JZNode::paramInCount() const
{
    return pinInList(Pin_param).size();
}

QList<int> JZNode::paramInList() const
{
    return pinInList(Pin_param);
}

QString JZNode::paramInValue(int index) const
{
    auto list = paramInList();
    return pinValue(list[index]);
}

void JZNode::setParamInValue(int index, const QString &value)
{
    auto list = paramInList();    
    setPinValue(list[index], value);
}

int JZNode::paramOut(int index) const
{
    auto list = pinOutList(Pin_param);
    if(index < list.size())
        return list[index];
    else
        return -1;
}

JZNodeGemo JZNode::paramOutGemo(int index) const
{
    return JZNodeGemo(m_id,paramOut(index));
}

int JZNode::paramOutCount() const
{
    return pinOutList(Pin_param).size();
}

QList<int> JZNode::paramOutList() const
{
    return pinOutList(Pin_param);
}

QString JZNode::paramOutValue(int index) const
{
    auto list = paramOutList();
    return pinValue(list[index]);
}

void JZNode::setParamOutValue(int index, const QString &value)
{
    auto list = paramOutList();
    setPinValue(list[index], value);
}

int JZNode::flowIn() const
{
    auto list = pinInList(Pin_flow);
    if(list.size() != 0)
        return list[0];
    else
        return -1;
}

JZNodeGemo JZNode::flowInGemo() const
{
    return JZNodeGemo(m_id,flowIn());
}

int JZNode::flowInCount() const
{
    return pinInList(Pin_flow).size();
}

int JZNode::flowOut(int index) const
{
    auto list = pinOutList(Pin_flow);
    if(index < list.size())
        return list[index];
    else
        return -1;
}

JZNodeGemo JZNode::flowOutGemo(int index) const
{
    return JZNodeGemo(m_id,flowOut(index));
}

QList<int> JZNode::flowOutList() const
{
    return pinOutList(Pin_flow);
}

int JZNode::flowOutCount() const
{
    return pinOutList(Pin_flow).size();
}

int JZNode::subFlowOut(int index) const
{
    auto list = pinOutList(Pin_subFlow);
    if(index < list.size())
        return list[index];
    else
        return -1;
}

JZNodeGemo JZNode::subFlowOutGemo(int index) const
{
    return JZNodeGemo(m_id,subFlowOut(index));
}

QList<int> JZNode::subFlowList() const
{
    return pinOutList(Pin_subFlow);
}

int JZNode::subFlowCount() const
{
    return pinOutList(Pin_subFlow).size();
}

const QString &JZNode::pinValue(int id) const
{
    auto ptr = pin(id);
    Q_ASSERT(ptr);

    return ptr->value();
}

void JZNode::setPinValue(int id,const QString &value)
{        
    auto ptr = pin(id);
    Q_ASSERT(ptr);
    
    ptr->setValue(value);
    onPinChanged(id);
}

const QString &JZNode::pinName(int id) const
{
    auto ptr = pin(id);
    Q_ASSERT(ptr);

    return ptr->name();
}

void JZNode::setPinName(int id,const QString &name)
{
    auto ptr = pin(id);
    Q_ASSERT(ptr);

    ptr->setName(name);
    onPinChanged(id);
}

QList<int> JZNode::pinList() const
{
    QList<int> result;
    for(int i = 0; i < m_pinList.size(); i++)
        result.push_back(m_pinList[i].id());

    return result;
}

JZScriptItem *JZNode::file() const
{
    return m_file;
}

void JZNode::setFile(JZScriptItem *file)
{
    m_file = file;
    update();
}

QString JZNode::path()
{
    return m_file->itemPath() + "/" + QString::number(m_id);
}

const JZScriptEnvironment *JZNode::environment() const
{
    if(m_file && m_file->project())
        return m_file->project()->environment();
    
    return nullptr;
}

const QString& JZNode::name() const
{
    return m_name;
}

bool JZNode::canRemove()
{
    return !(m_flag & NodeProp_noRemove);
}

int JZNode::id() const
{
    return m_id;
}

void JZNode::setId(int id)
{
    m_id = id;
}

const QStringList &JZNode::pinType(int id) const
{
    return pin(id)->dataType();
}

void JZNode::setPinTypeArg(int id)
{
    pin(id)->setDataType({ JZNodeType::typeName(Type_arg)});
}

void JZNode::setPinTypeInt(int id)
{
    pin(id)->setDataType({JZNodeType::typeName(Type_int)});
}

void JZNode::setPinTypeNumber(int id)
{
    pin(id)->setDataType({ JZNodeType::typeName(Type_bool),JZNodeType::typeName(Type_int),JZNodeType::typeName(Type_int64),JZNodeType::typeName(Type_double)});
}

void JZNode::setPinTypeBool(int id)
{
    pin(id)->setDataType({ JZNodeType::typeName(Type_bool)});
}

void JZNode::setPinTypeString(int id)
{
    pin(id)->setDataType( { JZNodeType::typeName(Type_string)});
}

void JZNode::setPinType(int id,const QStringList &type)
{
    pin(id)->setDataType(type);
}

void JZNode::clearPinType(int id)
{
    QStringList type;
    pin(id)->setDataType(type);
}

bool JZNode::canLink(int node_id, int pin_id, QString &error)
{
    Q_UNUSED(error);
    return true;
}

void JZNode::onPinLinked(int pin_id)
{
}

void JZNode::onPinUnlinked(int pin_id)
{
}

void JZNode::onPinChanged(int pin_id)
{
    update();
}

void JZNode::update()
{
    if (m_file)
    {
        QString error;
        updateNode(error);
    }
}

bool JZNode::updateNode(QString &error)
{
    return true;
}

void JZNode::saveToStream(QDataStream &s) const
{
    s << m_type;
    s << m_id;
    s << m_name;
    s << m_pos;
    s << m_flag;
    s << m_group;
    s << m_memo;
    s << m_pinList;
}

void JZNode::loadFromStream(QDataStream &s)
{    
    int node_type;
    s >> node_type;
    Q_ASSERT(node_type == m_type);

    s >> m_id;
    s >> m_name;
    s >> m_pos;
    s >> m_flag;
    s >> m_group;
    s >> m_memo;
    s >> m_pinList;  
}

//JZNodeUnknown
JZNodeUnknown::JZNodeUnknown()
{
    m_type = Node_unknown;
}

JZNodeUnknown::~JZNodeUnknown()
{
}

bool JZNodeUnknown::compiler(JZNodeCompiler* compiler, QString& error)
{
    error = "Unknown Node";
    return false;
}
